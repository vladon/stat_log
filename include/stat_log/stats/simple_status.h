//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

#pragma once
#include <boost/any.hpp>
#include <type_traits>
#include <stat_log/stats/stats_common.h>

namespace stat_log
{
   struct AssignPolicy
   {
      template <typename Repr>
      static void write(void* shared_ptr, Repr value)
      {
         auto ptr = reinterpret_cast<Repr*>(shared_ptr);
         (*ptr) = value;
      }
   };

   template <typename Repr>
   using SimpleStatus = detail::SimpleStat<Repr, AssignPolicy>;
}
