
#include "rxcpp/rx.hpp"

#include <type_traits>
#include <thread>
#include <iostream>
#include <memory>
// create alias' to simplify code
// these are owned by the user so that
// conflicts can be managed by the user.
namespace rx=rxcpp;
namespace rxsub=rxcpp::subjects;
namespace rxu=rxcpp::util;


// At this time, RxCpp will fail to compile if the contents
// of the std namespace are merged into the global namespace
// DO NOT USE: 'using namespace std;'

class OnlinePaesant
{
private:
	std::shared_ptr<std::atomic<bool>> m_canceltrigger;
public:

	OnlinePaesant() :
		m_canceltrigger(std::make_shared<std::atomic<bool>>(false)) {}

	rxcpp::observable<int> produceInt()
		{
			return rxcpp::observable<>::create<int>(
				[this](rxcpp::subscriber<int> s)
				{
					m_canceltrigger->store(false);
					s.add(rx::make_subscription([=]() { m_canceltrigger->store(true); }));
					
					int n = 10;
					while(n-- > 0 && !m_canceltrigger->load())
					{
						std::cout << "online working...\n";
						std::this_thread::sleep_for(std::chrono::milliseconds(150));
					}

					if(!m_canceltrigger->exchange(true))
					{
						s.on_next(1);
					}
					s.on_completed();
				});
		}
};

class OfflinePaesant
{
private:
	std::shared_ptr<std::atomic<bool>> m_canceltrigger;
public:

	OfflinePaesant() :
		m_canceltrigger(std::make_shared<std::atomic<bool>>(false)) {}

	rxcpp::observable<int> produceIntToo()
		{
			return rxcpp::observable<>::create<int>(
				[this](rxcpp::subscriber<int> s)
				{
					m_canceltrigger->store(false);
					
					s.add(rx::make_subscription([=]() { m_canceltrigger->store(true); }));
					
					int n = 2;
					while(n-- > 0 && !m_canceltrigger->load())
					{
						std::cout << "offline working...\n";
						std::this_thread::sleep_for(std::chrono::milliseconds(150));
					}

					if(!m_canceltrigger->exchange(true))
					{
						s.on_next(2);
					}
					s.on_completed();
				});	
		}
};


class Aristocracy
{
private:

	OnlinePaesant m_paesant1;
	OfflinePaesant m_paesant2;
	rxcpp::composite_subscription m_sub;
	rxcpp::observe_on_one_worker::coordinator_type m_coord;

	
public:

	Aristocracy() :
		m_coord(rxcpp::observe_on_new_thread().create_coordinator()) {}

	void carrotWithStick(std::function<void(int)> callback)
		{
			m_sub.unsubscribe();

			auto obs = m_paesant1.produceInt().merge(m_paesant2.produceIntToo()).subscribe_on(rxcpp::observe_on_new_thread()).
				take(1);

			m_sub = obs.subscribe(callback);
		}

	void carrot(std::function<void(int)> callback)
		{
			m_sub.unsubscribe();
			
			auto obs = m_paesant1.produceInt().merge(m_paesant2.produceIntToo()).subscribe_on(rxcpp::serialize_same_worker(m_coord.get_worker())).
				take(1);

			m_sub = obs.subscribe(callback);
		}

	void stick()
		{
			m_sub.unsubscribe();
		}
	
};

struct Keeper
{
	std::mutex mtx;
	std::condition_variable cv;
	bool pass;

	Keeper() : pass(false) {}
};


// do not get offended by the stupidity of the name convention within this test case :)
// also, it is known and intentional, that only 1st observable will produce something
int main()
{

	Aristocracy ar;

	Keeper kp;
	
	auto callback = [&kp](int i) {

		std::cout << "good job pals, especially " << (i == 1 ? "1st" : "2nd" ) << ", taxes held\n";
		
		std::unique_lock<std::mutex> lck(kp.mtx);
		kp.pass = true;
		lck.unlock();
		kp.cv.notify_one();
	};

	

	//TEST CASE ~ no joining
	std::cout << "TEST CASE: no joining\n";
	auto t0 = std::chrono::high_resolution_clock::now();
	ar.carrot(callback);
	auto t1 = std::chrono::high_resolution_clock::now();
	auto dur = std::chrono::duration<double>(t1-t0);
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() << "[ms]\n";
	
	ar.carrot(callback);
	t0 = std::chrono::high_resolution_clock::now();
	dur = std::chrono::duration<double>(t0-t1);
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() << "[ms]\n";


	ar.carrot(callback);
	t1 = std::chrono::high_resolution_clock::now();
	dur = std::chrono::duration<double>(t1-t0);
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() << "[ms]\n";

	ar.stick();
	t0 = std::chrono::high_resolution_clock::now();
	dur = std::chrono::duration<double>(t0-t1);
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() << "[ms]\n";

	ar.carrot(callback);
	t1 = std::chrono::high_resolution_clock::now();
	dur = std::chrono::duration<double>(t1-t0);
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() << "[ms]\n";

	std::unique_lock<std::mutex> lck(kp.mtx);
	kp.cv.wait(
			lck,
			[&]
			{
				return kp.pass;
			}
			);

	kp.pass = false;
	lck.unlock();


	//TEST CASE ~ with joining
	std::cout << "TEST CASE: with joining\n";
	t0 = std::chrono::high_resolution_clock::now();
	ar.carrotWithStick(callback);
	t1 = std::chrono::high_resolution_clock::now();
	dur = std::chrono::duration<double>(t1-t0);
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() << "[ms]\n";
	
	ar.carrotWithStick(callback);
	t0 = std::chrono::high_resolution_clock::now();
	dur = std::chrono::duration<double>(t0-t1);
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() << "[ms]\n";
	

	ar.carrotWithStick(callback);
	t1 = std::chrono::high_resolution_clock::now();
	dur = std::chrono::duration<double>(t1-t0);
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() << "[ms]\n";
	

	ar.stick();
	t0 = std::chrono::high_resolution_clock::now();
	dur = std::chrono::duration<double>(t0-t1);
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() << "[ms]\n";

	ar.carrotWithStick(callback);
	t1 = std::chrono::high_resolution_clock::now();
	dur = std::chrono::duration<double>(t1-t0);
	std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(dur).count() << "[ms]\n";


	lck.lock();
	kp.cv.wait(
			lck,
			[&]
			{
				return kp.pass;
			}
			);

	kp.pass = false;

	
    return 0;
}
