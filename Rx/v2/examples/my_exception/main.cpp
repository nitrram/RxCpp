#include "rxcpp/rx.hpp"

#include <exception>
// create alias' to simplify code
// these are owned by the user so that
// conflicts can be managed by the user.
namespace rx=rxcpp;
namespace rxo=rx::operators;
namespace rxsub=rxcpp::subjects;
namespace rxu=rxcpp::util;

// At this time, RxCpp will fail to compile if the contents
// of the std namespace are merged into the global namespace
// DO NOT USE: 'using namespace std;'


int main()
{

  // idiomatic (prefer operators)
  {
    auto published_observable =
      rx::observable<>::range(1, 3) |
      rxo::flat_map([](int i) {

	  return rx::observable<>::create<int>([=](rx::subscriber<int> s) {

	       std::exception_ptr ee =
	       std::make_exception_ptr(std::invalid_argument("blah"));

	       if(!(i%2)) {
		 s.on_error(ee);
	       } else {
		 s.on_next(i);
	       }
	    });

	 
	}) |
      rxo::map([](int i)
	       {
		 return i;
	       });


    auto subs = published_observable.subscribe(
					       [](int i) { std::cout << i << std::endl; },
					       [](std::exception_ptr eptr) {
						 try
						   {
						     std::rethrow_exception(eptr);
						   }
						 catch(std::exception ex)
						   {
						     std::cout << ex.what() << std::endl;
						   }
					       }
					       );

  }

  return 0;
}
