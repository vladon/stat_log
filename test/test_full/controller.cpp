//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

#include "common.h"

using namespace stat_log;

int main(int argc, char** argv)
{
   initializeStatistics<false == IsOperational>();
   handleCommandLineArgs(argc, argv);
   return 0;
}
