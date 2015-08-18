#pragma once

namespace stat_log
{
   template <typename Repr>
   struct SimpleCounter
   {
      using SharedType = Repr;
      void write(Repr* shared_ptr, Repr value)
      {
         (*shared_ptr) += value;
      }

      void doCommand(Repr* shared_ptr, StatCmd cmd, boost::any& arg)
      {
         //TODO: handle all commands
         std::cout << std::dec << " In doCommand, value = "
            << (unsigned long int)*shared_ptr << " ";
      }
   };

   template <typename Repr>
   struct is_serialization_deferred<SimpleCounter<Repr>>
   {
      static constexpr bool value = false;
   };
}
