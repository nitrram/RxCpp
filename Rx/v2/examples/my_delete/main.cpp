#include "rxcpp/rx.hpp"

#include <iostream>
#include <mutex>
#include <condition_variable>

// create alias' to simplify code
// these are owned by the user so that
// conflicts can be managed by the user.
namespace rx=rxcpp;
namespace rxsub=rx::subjects;
namespace rxu=rx::util;
namespace rxo=rx::operators;

// At this time, RxCpp will fail to compile if the contents
// of the std namespace are merged into the global namespace
// DO NOT USE: 'using namespace std;'

std::mutex mtx;
std::condition_variable cv;

class Callback
{
public:
	Callback() {
		std::cout << "Callback created " << this << std::endl;
	}
	Callback(const Callback &) = delete;
	Callback(Callback&&) = delete;

	void onCalled(int d) {
		std::cout << d << std::endl;
	}
};

class CWrapper {

public:

	CWrapper() : cb(new Callback()) {}

	~CWrapper() { delete cb; cb = nullptr; }

	Callback *cb;
};


template<typename T>
class safe_observable {
public:

	safe_observable(rx::observable<T> t) {
		obs = t | rxo::finally([this]() { std::lock_guard<std::recursive_mutex> lck(mtx); });
	}

	~safe_observable() {
		std::cout << "safe_observable destructed\n";
	}

	rx::composite_subscription subscribe()
	{
		return obs.subscribe();
	}

	rx::composite_subscription subscribe(std::function<void(T)> on_next)
	{
		return obs.subscribe(
			[this,on_next](T result) {
				std::lock_guard<std::recursive_mutex> lck(mtx);
				on_next(result);
			});
	}

	rx::composite_subscription subscribe(
		std::function<void(T)> on_next,
		std::function<void(std::exception_ptr)> on_error
	)
	{
		return obs.subscribe(
			[this, on_next](T result) {
				std::lock_guard<std::recursive_mutex> lck(mtx);
				on_next(result);
			},
			[this, on_error](std::exception_ptr ex) {
				std::lock_guard<std::recursive_mutex> lck(mtx);
				on_error(ex);
			});
	}

	rx::composite_subscription subscribe(
		std::function<void(T)> on_next,
		std::function<void(std::exception_ptr)> on_error,
		std::function<void()> on_completed
	)
	{
		return obs.subscribe(
			[this, on_next](T result) {
				std::lock_guard<std::recursive_mutex> lck(mtx);
				on_next(result);
			},
			[this, on_error](std::exception_ptr ex) {
				std::lock_guard<std::recursive_mutex> lck(mtx);
				on_error(ex);
			},
			[this, on_completed]() {
				std::lock_guard<std::recursive_mutex> lck(mtx);
				on_completed();
			}
			);
	}

private:
	rx::observable<T> obs;
	std::recursive_mutex mtx;
};


int main()
{

	std::mutex simMtx;
	std::condition_variable simCondition;
	bool simTrigger = false;

	using tt = std::chrono::high_resolution_clock;
	using ms = std::chrono::milliseconds;
	using fsec = std::chrono::duration<float>;

	CWrapper *cw = new CWrapper();

	auto observable = rx::observable<>::create<int>(
		[](rx::subscriber<int> s) {

			//s.add(rx::make_subscription([&]() { std::cout << "internal unsubscribe " << std::this_thread::get_id() << std::endl; }));
			auto t0 = tt::now();
			std::this_thread::sleep_for(ms(2000));
			auto t1 = tt::now();

			fsec diff = t1 - t0;

			s.on_next(std::chrono::duration_cast<ms>(diff).count());

			// predchozi on next subscription zpusobi unsubscribci, takze sem uz se nedostanu
			std::this_thread::sleep_for(ms(200));

			s.on_next(88231);

			s.on_completed();
		}
	) |	rxo::subscribe_on(rx::observe_on_new_thread());

	rx::composite_subscription sub;
	{
		//tento objekt zije jen v lokalnim scope
		safe_observable<int> ssub(observable);

		sub = ssub.subscribe([&](int d) {

				// uvolni cekajici vlakno, aby mohlo smazat captured promennou pod rukama
				std::unique_lock<std::mutex> lck(simMtx);
				simTrigger = true;
				lck.unlock();
				simCondition.notify_all();

				// pockej, aby mazaci vlakno melo cas smazat objekt
				std::this_thread::sleep_for(ms(500));

				std::cout << "segfault?\n";
				cw->cb->onCalled(d);
			}
			);
	}

	//#############case 1
	//sub.unsubscribe(); // callback se nikdy neprovola(nestihne to)

	// simuluj mazani na jinem vlakne
	auto deleter = rx::observable<>::create<int>(
		[&](rx::subscriber<int> s) {

			//pockej nez se zacne provadet lambda v subscribci
			{
				std::unique_lock<std::mutex> lck(simMtx);
				simCondition.wait(
					lck,
					[&]
					{
						return simTrigger;
					}
				);
			}

			//###############case 2
			sub.unsubscribe(); // pokud callback probiha, je zamkly, toto je blokujici
			delete cw;
			cw = nullptr;

			s.on_completed();
		}) | rxo::finally([&](){
			// uvolni cekajici vlakno
			std::unique_lock<std::mutex> lck(simMtx);
			simTrigger = true;
			lck.unlock();
			simCondition.notify_all();
			}) | rxo::subscribe_on(rx::observe_on_new_thread());

	auto dd = deleter.subscribe();

	std::this_thread::sleep_for(ms(6000));

	std::cout <<  "Test passed OK!\n";// << cw->cb << "]\n";

	dd.unsubscribe(); // deleter uz neni treba, tak ho interruptni (kvuli ####case 1)

	return 0;
}
