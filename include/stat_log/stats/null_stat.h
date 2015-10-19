//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

#pragma once

#include <stat_log/util/command.h>
#include <boost/any.hpp>

namespace stat_log
{
   //Null stat that will effectively remove the
   // writeStat() calls (after the optimizer runs).
   struct NullStat
   {
      //TODO: we really want to have shared_type == void,
      // but this doesn't compile in because I am using the
      // sizeof operator in other files.
      using shared_type = int;
      template <typename ...Args>
      static void write(void* shared_ptr, Args... args)
      {
      }

      static void doStatCommand(
            void* shared_ptr,
            StatCmd cmd,
            boost::any& arg,
            StatCmdOutput& stat_output)
      {
         if(cmd == StatCmd::PRINT_STAT_TYPE)
         {
            stat_output.entryTitle = "NULL stat";
         }
      }
   };

}
