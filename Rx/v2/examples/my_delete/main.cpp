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

	safe_observable(rx::observable<T> t) : in_progress(false) {
		//obs = t | rxo::finally([]() { if(in_progress) });
	}


	// void subscribe();

	// void subscribe(std::function<void(T)>);

	// void subscribe(std::function<void(T)>, std::function<void(std::exception_ptr)>);

	// void subscribe(std::function<void(T)>, std::function<void(std::exception_ptr)>, std::function<void()>) {}

private:
	rx::observable<T> obs;
	std::recursive_mutex mtx;
	bool in_progress;
};


int main()
{

	std::mutex simMtx;
	std::condition_variable simCondition;
	bool simTrigger = false;

	using tt = std::chrono::high_resolution_clock;
	using ms = std::chrono::milliseconds;
	using fsec = std::chrono::duration<float>;

	std::mutex mutex;
	CWrapper *cw = new CWrapper();

	auto observable = rx::observable<>::create<int>(
		[](rx::subscriber<int> s) {

			//s.add(rx::make_subscription([&]() { cv.notify_all(); }));
			auto t0 = tt::now();
			std::this_thread::sleep_for(ms(2000));
			auto t1 = tt::now();

			fsec diff = t1 - t0;

			s.on_next(std::chrono::duration_cast<ms>(diff).count());

			s.on_completed();
		}
	) | rxo::finally([&](){
			std::lock_guard<std::mutex> crit(mutex);
			std::cout << "finally\n"; }) |
		rxo::subscribe_on(rx::observe_on_new_thread());

	auto sub = observable.subscribe([&](int d) {

			std::lock_guard<std::mutex> crit(mutex);
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
