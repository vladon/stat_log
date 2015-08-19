#pragma once
#include <boost/any.hpp>
#include <type_traits>

namespace stat_log
{
   template <typename Repr>
   struct SimpleCounter
   {
      using SharedType = Repr;
      static_assert(std::is_integral<SharedType>::value,
            "SimpleCounter's underlying type MUST be integral!");
      static void write(void* shared_ptr, Repr value)
      {
         auto ptr = reinterpret_cast<Repr*>(shared_ptr);
         (*ptr) += value;
      }

      static void doCommand(void* shared_ptr, StatCmd cmd, boost::any& arg)
      {
         auto ptr = reinterpret_cast<Repr*>(shared_ptr);
         //TODO: handle all commands
         std::cout << std::dec << (unsigned long int)*ptr;
      }
   };

   template <typename Repr>
   struct is_serialization_deferred<SimpleCounter<Repr>>
   {
      static constexpr bool value = false;
   };
}
