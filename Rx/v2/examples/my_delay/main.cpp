#include "rxcpp/rx.hpp"

#include <mutex>
#include <condition_variable>
#include <memory>

// create alias' to simplify code
// these are owned by the user so that
// conflicts can be managed by the user.
namespace rx=rxcpp;
namespace rxo=rx::operators;
namespace rxsub=rxcpp::subjects;
namespace rxu=rxcpp::util;

using namespace std::chrono;

// At this time, RxCpp will fail to compile if the contents
// of the std namespace are merged into the global namespace
// DO NOT USE: 'using namespace std;'

#define UNLOCK() std::unique_lock<std::mutex> lck(m_SimMtx);	\
	m_SimTrigger = true;										\
	lck.unlock();												\
	m_SimCondition.notify_all();


std::mutex mtx;
std::condition_variable cv;



std::function<void(rx::subscriber<bool>)> create_subscriber_function(unsigned seconds_delay, bool output = true, bool is_error = false)
{
	return [seconds_delay,output, is_error](rx::subscriber<bool> s)
	{
		s.add(rx::make_subscription([](){ std::cout << "online finish execution\n"; }));

		std::cout << "true_observable[" << seconds_delay << "] working...\n";
		auto t0 = std::chrono::high_resolution_clock::now();
		std::this_thread::sleep_for(std::chrono::seconds(seconds_delay));

		auto t1 = std::chrono::high_resolution_clock::now();
		std::cout << "true_observable[" << seconds_delay << "] took " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()<< "[ms]\n";

		if(is_error) {

			s.on_error(std::make_exception_ptr<std::logic_error>(std::logic_error("failed")));
		}

		s.on_next(output);

		s.on_completed();
	};
}

struct slocker {

public:
	slocker() : sactiv(true), strig(false) {}

	std::atomic<bool> sactiv;
	std::atomic<bool> strig;
};


rx::observable<bool> cto2(rx::observable<bool> online, rx::observable<bool> offline)
{
	// trigger, active
	using tlock = std::shared_ptr<slocker>;

	return rx::observable<>::just(tlock(new slocker())) |
		rxo::flat_map(
			[offline, online](tlock t)
			{
				using namespace std::chrono;

				auto timedOffline = rx::observable<>::create<int>(
					[=](rx::subscriber<int> s)
					{
						using namespace std::chrono;

						auto t0 = high_resolution_clock::now();
						time_point<high_resolution_clock> t1;
						while(!t->strig.load()) {
							std::this_thread::sleep_for(milliseconds(10));
							t1 = high_resolution_clock::now();
							if(duration_cast<milliseconds>(t1 - t0).count() > 2000) {
								t->strig.store(true);
							}
						}

						if(t->sactiv.exchange(false)) {
							s.on_next(1);
							s.on_completed();
						}
					} ) |
					rxo::flat_map([offline](int){ return offline; }) | rxo::subscribe_on(rx::observe_on_new_thread());


				auto threadedOnline = online |
					rxo::tap([=](bool){ std::cout << "tap inactive\n";
										t->sactiv.store(false); }) | // kdyz prijde vysledek
					rxo::finally([=](){ std::cout << "online finished\n";
										t->strig.store(true); }) | // po dokonceni
					rxo::subscribe_on(rx::observe_on_new_thread());

				return threadedOnline.subscribe_on(rx::observe_on_new_thread()).merge_error_delay(timedOffline).take(1);
			} );

}

int main()
{
	std::mutex m_SimMtx;
	std::condition_variable m_SimCondition;
	bool m_SimTrigger = false;


	auto true_observable_short =
		rx::observable<>::create<bool>(
			create_subscriber_function(1)
		);


	auto true_observable_long =
		rx::observable<>::create<bool>(
			create_subscriber_function(4)
		);


	auto false_observable =
		rx::observable<>::just(false) | rxo::tap([](bool /*val*/) { std::cout << "false_observable working...\n"; });

	std::unique_lock<std::mutex> lck(m_SimMtx, std::defer_lock);

	//==============================================================================================================================

	std::cout << "\n\n======TESTING sequqence======\n";
	auto t0 = std::chrono::high_resolution_clock::now();
	// normal sequence
	auto subs = cto2(true_observable_short, false_observable).subscribe(
		[&](bool i) {
			std::cout << i << std::endl;
			UNLOCK()
		}
	);

	lck.lock();
	m_SimCondition.wait(
		lck,
		[&]
		{
			return m_SimTrigger;
		}
	);
	m_SimTrigger = false;
	subs.unsubscribe();
	lck.unlock();
	auto t1 = std::chrono::high_resolution_clock::now();
	std::cout << "short 2s delayed seq took: " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()<< "[ms]\n\n";


	std::cout << "\n\n======TESTING overtake======\n";
	t0 = std::chrono::high_resolution_clock::now();
	// overtake
	subs = cto2(true_observable_long, false_observable).subscribe(
		[&](bool i)
		{
			std::cout << i << std::endl;
			UNLOCK()
		}
	);

	lck.lock();
	m_SimCondition.wait(
		lck,
		[&]
		{
			return m_SimTrigger;
		}
	);
	m_SimTrigger = false;

	subs.unsubscribe();
	lck.unlock();
	t1 = std::chrono::high_resolution_clock::now();
	std::cout << "long 2s delayed seq took: " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()<< "[ms]\n";

	//================================================================================================================================

	rx::observable<bool> test0, test1, teste;

	teste = rx::observable<>::create<bool>(
		create_subscriber_function(1, true, true)
	);

	test0 = rx::observable<>::just(true);
	test1 = rx::observable<>::just(false);

	std::cout << "\n\n========TESTING error=======\n";


	std::cout << "======[ ..... 1 ...... ]======\n";
	t0 = std::chrono::high_resolution_clock::now();
	cto2(teste, test0).subscribe(
		[&](bool i) { std::cout << i << std::endl; UNLOCK() },
		[&](std::exception_ptr) { }
	);

	lck.lock();
	m_SimCondition.wait(
		lck,
		[&]
		{
			return m_SimTrigger;
		}
	);
	m_SimTrigger = false;
	lck.unlock();

	t1 = std::chrono::high_resolution_clock::now();
	std::cout << "error 2s delayed seq took: " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()<< "[ms]\n";

	std::cout << "\n======[ ..... 0 ...... ]======\n";
	t0 = std::chrono::high_resolution_clock::now();
	cto2(teste, test1).subscribe(
		[&](bool i) { std::cout << i << std::endl; UNLOCK() },
		[&](std::exception_ptr) { }
	);

	lck.lock();
	m_SimCondition.wait(
		lck,
		[&]
		{
			return m_SimTrigger;
		}
	);
	m_SimTrigger = false;
	lck.unlock();

	t1 = std::chrono::high_resolution_clock::now();
	std::cout << "error 2s delayed seq took: " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()<< "[ms]\n";


	return 0;
}
