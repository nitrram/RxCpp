#include "rxcpp/rx.hpp"

#include <mutex>
#include <condition_variable>

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

std::mutex mtx;
std::condition_variable cv;

int main()
{

    // using tt = std::chrono::high_resolution_clock;
    // using ms = std::chrono::milliseconds;
    // using fsec = std::chrono::duration<float>;
	std::mutex m_SimMtx;
	std::condition_variable m_SimCondition;
	bool m_SimTrigger = false;
	
	std::atomic<bool> timeout(false);

//	auto emitter = rx::observable<>::

	auto true_observable =
		rx::observable<>::create<bool>(
			[&](rx::subscriber<bool> s)
			{
				std::cout << "true_observable working...\n";
				std::this_thread::sleep_for(std::chrono::seconds(1));
				if(!timeout.exchange(true)) {
					s.on_next(true);
				}

				s.on_completed();
			}
		);

	auto false_observable =
		rx::observable<>::just(false) | rxo::tap([](bool /*val*/) { std::cout << "false_observable working...\n"; });


	// 0 then 1
	auto subs = false_observable.delay(std::chrono::seconds(2),rx::observe_on_new_thread()).concat(true_observable).
		take(1).subscribe_on(rx::observe_on_new_thread()).subscribe(
			[&](bool i) {
				std::cout << i << std::endl;
				timeout.exchange(true);


				std::unique_lock<std::mutex> lck(m_SimMtx);
				m_SimTrigger = true;
				lck.unlock();
				m_SimCondition.notify_all();

			}
		);

	
	std::unique_lock<std::mutex> lck(m_SimMtx);
	m_SimCondition.wait(
		lck,
		[&]
		{
			return m_SimTrigger;
		}
	);
	lck.unlock();


    return 0;
}
