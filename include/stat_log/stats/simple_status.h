#pragma once
#include <boost/any.hpp>
#include <type_traits>
#include <stat_log/parsers/parser_common.h>

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
