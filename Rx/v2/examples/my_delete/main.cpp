#include "rxcpp/rx.hpp"

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

int main()
{

    using tt = std::chrono::high_resolution_clock;
    using ms = std::chrono::milliseconds;
    using fsec = std::chrono::duration<float>;

    std::recursive_mutex mutex;
    CWrapper *cw = new CWrapper();

    auto observable = rx::observable<>::create<int>(
        [](rx::subscriber<int> s) {

            //s.add(rx::make_subscription([&]() { cv.notify_all(); }));
            //std::mutex mtx;
            //std::unique_lock<std::mutex> lck(mtx);

            auto t0 = tt::now();
            //cv.wait_for(lck,std::chrono::seconds(5));
            std::this_thread::sleep_for(ms(5000));
            auto t1 = tt::now();

            fsec diff = t1 - t0;

            s.on_next(std::chrono::duration_cast<ms>(diff).count());

            s.on_completed();
        }
    ) | rxo::subscribe_on(rx::observe_on_new_thread());


    auto sub = observable.subscribe([&cw](int d) {
            std::cout << "callback about to be triggered with " << d << "[" << cw->cb << "]" << std::endl;
            std::this_thread::sleep_for(ms(500));
            cw->cb->onCalled(d); });

    delete cw;
    cw = nullptr;


    std::cout <<  "callback gone [0]\n";// << cw->cb << "]\n";

    //sub.unsubscribe();

    std::this_thread::sleep_for(ms(6000));

    return 0;
}
