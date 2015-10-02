//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

#include "common.h"
#include <thread>
#include <chrono>

int main(int argc, char** argv)
{
   initializeStatistics<true == IsOperational>();
   genStats_MAC();
   genStats_SIS();
   genStats_HW_INTF();

   while(true)
   {
      std::this_thread::sleep_for(std::chrono::seconds{2});
   }
   return 0;
}
