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

#define UNLOCK() std::unique_lock<std::mutex> lck(m_SimMtx); \
	m_SimTrigger = true;									 \
	lck.unlock();											 \
	m_SimCondition.notify_all();


std::mutex mtx;
std::condition_variable cv;
bool isActive;

std::function<void(rx::subscriber<bool>)> create_subscriber_function(unsigned seconds_delay)
{
	return [seconds_delay](rx::subscriber<bool> s)
			{
				
				std::cout << "true_observable[" << seconds_delay << "] working...\n";
				auto t0 = std::chrono::high_resolution_clock::now();
				std::this_thread::sleep_for(std::chrono::seconds(seconds_delay));

				auto t1 = std::chrono::high_resolution_clock::now();
				
				if(isActive) {
					std::cout << "true_observable[" << seconds_delay << "] took " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()<< "[ms]\n";
					s.on_next(true);

					s.on_completed();
				}
			};
}

rx::observable<bool> create_timed_observable_false(rx::observable<bool> false_observable)
{
	return rx::observable<>::timer(seconds(2)) |
		rxo::flat_map([false_observable](int /*s*/) { return false_observable; }) |
		rxo::subscribe_on(rx::observe_on_new_thread());
}

int main()
{
	std::mutex m_SimMtx;
	std::condition_variable m_SimCondition;
	bool m_SimTrigger = false;
	isActive = true;
	

	auto true_observable_short =
		rx::observable<>::create<bool>(
			create_subscriber_function(1)
		) | rxo::subscribe_on(rx::observe_on_new_thread());


	auto true_observable_long =
		rx::observable<>::create<bool>(
			create_subscriber_function(4)
		) | rxo::subscribe_on(rx::observe_on_new_thread());


	auto false_observable =
		rx::observable<>::just(false) | rxo::tap([](bool /*val*/) { std::cout << "false_observable working...\n"; });

	std::unique_lock<std::mutex> lck(m_SimMtx, std::defer_lock);

	//==============================================================================================================================

	std::cout << "\n\n======TESTING sequqence======\n";
	auto t0 = std::chrono::high_resolution_clock::now();
	std::atomic<bool> fuse(true);
	// normal sequence
	auto subs = true_observable_short.
		merge_error_delay(create_timed_observable_false(false_observable)).
		take(1).
		subscribe(
			[&](bool i) {
				if(fuse.exchange(true))
				{
					std::cout << i << std::endl;
					UNLOCK()
				}

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
	isActive = false;
	fuse.store(true);
	subs.unsubscribe();
	lck.unlock();
	auto t1 = std::chrono::high_resolution_clock::now();
	std::cout << "short 2s delayed seq took: " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()<< "[ms]\n\n";


	std::cout << "\n\n======TESTING overtake======\n";
	t0 = std::chrono::high_resolution_clock::now();
	// overtake
	isActive = true;
	subs = true_observable_long.
		merge_error_delay(create_timed_observable_false(false_observable)).
		take(1).
		subscribe(
			[&](bool i) {
				if(fuse.exchange(false))
				{
					std::cout << i << std::endl;
					UNLOCK()
				}

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
	isActive = false;
	fuse.store(true);
	subs.unsubscribe();
	lck.unlock();
	t1 = std::chrono::high_resolution_clock::now();
	std::cout << "long 2s delayed seq took: " << std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count()<< "[ms]\n";

	//================================================================================================================================

	rx::observable<bool> test0, test1, teste;

	teste = rx::observable<>::create<bool>([](rx::subscriber<bool> s) {

			s.on_error(std::make_exception_ptr<std::logic_error>(std::logic_error("fail")));
			
			s.on_next(true);

			s.on_completed();
			
		} ) | rxo::subscribe_on(rx::observe_on_new_thread());
		
	test0 = rx::observable<>::just(true); // | rxo::subscribe_on(rx::observe_on_new_thread());
	test1 = rx::observable<>::just(false); // | rxo::subscribe_on(rx::observe_on_new_thread());

	std::cout << "\n\n======TESTING error======\n";
	subs = teste.merge_error_delay(create_timed_observable_false(test1), create_timed_observable_false(test0)).take(1).subscribe(
		[&](bool v) {
			if(fuse.exchange(false)) {
				std::cout << v << std::endl;
				UNLOCK()
			}
		},
		[&](std::exception_ptr ep) {
			
			try{
				std::rethrow_exception(ep);
			}
			catch(std::logic_error e) {
				std::cout << "logic error: " << e.what() << std::endl;
			}
			UNLOCK()
		} );

	lck.lock();
	m_SimCondition.wait(
		lck,
		[&]
		{
			return m_SimTrigger;
		}
	);
	m_SimTrigger = false;
	isActive = false;
	fuse.store(true);
	subs.unsubscribe();
	lck.unlock();

	
    return 0;
}
