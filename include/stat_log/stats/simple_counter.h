#pragma once
#include <boost/any.hpp>
#include <type_traits>
#include <stat_log/stats/stats_common.h>

namespace stat_log
{
   struct IncrementPolicy
   {
      template <typename Repr>
      static void write(void* shared_ptr, Repr value)
      {
         auto ptr = reinterpret_cast<Repr*>(shared_ptr);
         (*ptr) += value;
      }
   };

   template <typename Repr>
   using SimpleCounter = detail::SimpleStat<Repr, IncrementPolicy>;
}
