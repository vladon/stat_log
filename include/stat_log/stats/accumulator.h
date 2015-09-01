#pragma once
#include <iostream>
#include <stat_log/parsers/parser_common.h>
#include <stat_log/stats/stats_common.h>

namespace stat_log
{

template <typename AccumSet>
struct Accumulator {};

namespace accumulator
{

   //OPERATIONAL ACCUMULATOR TRAITS:
   //For each statistic type the user defines, they must also provide an
   //operational mapping so the library knowns how to serialize the stat into
   //shared memory.
   //For example, suppose the _mean_ stat is to be used. In this case, the user
   //must provide something like (where AccumulatorSetType is their definition
   //of accumulator_set<..>):
   //
   //   template <>
   //   struct op_traits<AccumulatorSetType, boost::accumulator::tag::mean>
   //   {
   //       using SharedType = double;
   //       static void serialize(AccumulatorSetType& acc, void* ptr)
   //       {
   //          *reinterpret_cast<SharedType*>(ptr) = boost::accumulator::mean(acc);
   //       }
   //
   //       static size_t size()
   //       {
   //          return sizeof(SharedType);
   //       }
   //   };
   template <typename AccumStatType, typename Repr>
   struct op_traits;

   //CONTROL ACCUMULATOR TRAITS:
   //TODO: documentation.
   template <typename AccumStatType>
   struct control_traits;
}

namespace detail
{
   template <typename AccumSet>
   struct AccumOp
   {
      using sample_type = typename AccumSet::sample_type;
      //features_type is a MPL list of features.
      //TODO: we need to transform this into something
      // we can use to serialize each stat -- the op_traits
      // will be used to do this transformation.
      using features_type = typename AccumSet::features_type;
      //Called via the deferred processing thread
      void serialize(void* ptr)
      {
         std::cout << "SERIALIZE!\n";
         //TODO: need to do fusion::for_each here
         // to visit each of the features' serialize method.
      }

      // using SharedType = fusion_algo_to_transform_stat_list_by_the_op_traits;
      //TODO
      using SharedType = double;
      void write(void* /*shared_ptr*/, sample_type sample)
      {
         acc(sample);
      }

   private:
      AccumSet acc;
   };

   template <typename AccumSet>
   struct AccumControl
   {
      //TODO: implement this ...
      //Just for testing
      using SharedType = double;
      static void doStatCommand(
            void* shared_ptr,
            StatCmd cmd,
            boost::any& arg,
            const std::vector<std::string>& enumNames,
            const std::vector<std::string>& dimensionNames,
            int dimension_idx)
      {
         auto ptr = reinterpret_cast<SharedType*>(shared_ptr);
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

   template <typename AccumSet>
   struct stat_type_to_impl<Accumulator<AccumSet>, true>
   {
      using type = AccumOp<AccumSet>;
   };

   template <typename AccumSet>
   struct stat_type_to_impl<AccumSet, false>
   {
      using type = AccumControl<AccumSet>;
   };
}

template <typename AccumSet>
struct is_serialization_deferred<detail::AccumOp<AccumSet>>
{
   static constexpr bool value = true;
};


}

