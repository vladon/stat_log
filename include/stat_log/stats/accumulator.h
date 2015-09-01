#pragma once
#include <iostream>
#include <stat_log/parsers/parser_common.h>
#include <stat_log/stats/stats_common.h>

namespace stat_log
{

template <typename Sample, typename Features, typename Weight = void>
struct Accumulator {};

namespace detail
{
   template <typename Sample, typename Features, typename Weight>
   struct AccumOp
   {
      //Called via the deferred processing thread
      void serialize(void* ptr)
      {
         std::cout << "SERIALIZE!\n";
      }

      //Just for testing
      using SharedType = Sample;
      static void write(void* shared_ptr, Sample value)
      {
         auto ptr = reinterpret_cast<Sample*>(shared_ptr);
         (*ptr) = value;
      }

   private:
      // boost::accumulators::accumulator_set<Sample, Features, Weight> acc;
   };

   template <typename Sample, typename Features, typename Weight>
   struct AccumControl
   {
      //Just for testing
      using SharedType = Sample;
      static void doStatCommand(
            void* shared_ptr,
            StatCmd cmd,
            boost::any& arg,
            const std::vector<std::string>& enumNames,
            const std::vector<std::string>& dimensionNames,
            int dimension_idx)
      {
         auto ptr = reinterpret_cast<Sample*>(shared_ptr);
         //TODO: handle all commands
         auto& val = *ptr;
         if(cmd == StatCmd::DUMP_STAT)
         {
            if(val >= 0 && static_cast<size_t>(val) < enumNames.size())
               std::cout << enumNames[val];
            else
               std::cout << std::dec << (unsigned long int)val;
         }
         else if(cmd == StatCmd::PRINT_STAT_TYPE)
         {
            std::cout << "Accumulator";
         }
         else if(cmd == StatCmd::CLEAR_STAT)
         {
            val = 0;
         }
      }
   };

   template <typename... Args>
   struct stat_type_to_impl<Accumulator<Args...>, true>
   {
      using type = AccumOp<Args...>;
   };

   template <typename... Args>
   struct stat_type_to_impl<Accumulator<Args...>, false>
   {
      using type = AccumControl<Args...>;
   };
}

template <typename... Args>
struct is_serialization_deferred<detail::AccumOp<Args...>>
{
   static constexpr bool value = true;
};


}

