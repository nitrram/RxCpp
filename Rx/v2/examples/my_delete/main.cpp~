#include "rxcpp/rx.hpp"

#include <mutex>
#include <condition_variable>

// create alias' to simplify code
// these are owned by the user so that
// conflicts can be managed by the user.
namespace rx=rxcpp;
namespace rxsub=rxcpp::subjects;
namespace rxu=rxcpp::util;

// At this time, RxCpp will fail to compile if the contents
// of the std namespace are merged into the global namespace
// DO NOT USE: 'using namespace std;'

std::mutex mtx;
std::condition_variable cv;

int main()
{

    using tt = std::chrono::high_resolution_clock;
    using ms = std::chrono::milliseconds;
    using fsec = std::chrono::duration<float>;



    // idiomatic (prefer operators)
    {
        auto published_observable =
            rx::observable<>::create<int>(
                [](rx::subscriber<int> s)
                {
                    s.add(rx::make_subscription([&]() { cv.notify_all(); }));
                    std::mutex mtx;
                    std::unique_lock<std::mutex> lck(mtx);

                    auto t0 = tt::now();

                    cv.wait_for(lck,std::chrono::seconds(5));

                    auto t1 = tt::now();

                    fsec diff = t1 - t0;

                    std::cout << std::chrono::duration_cast<ms>(diff).count() << "[ms]\n";


                    s.on_next(24);

                    s.on_completed();
                }
            )
            .subscribe_on(rx::observe_on_new_thread());


        auto subs = published_observable.subscribe(
            [](int i) { std::cout << i << std::endl; }
        );


        std::this_thread::sleep_for(std::chrono::seconds(3));


        subs.unsubscribe();



        std::cout << "waiting yet 6 more seconds\n";

        std::this_thread::sleep_for(std::chrono::seconds(6));
        /*
        published_observable.
            ref_count().
            take_until(rx::observable<>::timer(std::chrono::seconds(1))).
            finally([](){
                std::cout << "unsubscribed" << std::endl << std::endl;
            }).
            subscribe([](int i){
                std::cout << i << std::endl;
            });
        */
    }

    return 0;
}
