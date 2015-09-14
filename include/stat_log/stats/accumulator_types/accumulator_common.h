#pragma once
#include <boost/accumulators/framework/extractor.hpp>
#include <sstream>


namespace stat_log
{
namespace accumulator
{
//ACCUMULATOR TRAITS:
//For each statistic type the user defines, they must also provide a mapping
//so the library knowns how to serialize the stat into shared memory.
//For example, suppose the _mean_ stat is to be used. In this case, the user
//must provide something like (where AccumulatorSetType is their definition
//of accumulator_set<..>):
//
//   template <>
//   struct traits<AccumulatorSetType, boost::accumulator::tag::mean>
//   {
//       using shared_type = typename AccumulatorSetType::sample_type;
//       static void serialize(AccumulatorSetType& acc, char* ptr)
//       {
//          *reinterpret_cast<shared_type*>(ptr) = boost::accumulator::mean(acc);
//       }
//
//       static constexpr size_t size()
//       {
//          return sizeof(shared_type);
//       }
//
//       static const char* const stat_name = "mean";
//       static dumpStat(void* ptr)
//       {
//          ss << *reinterpret_cast<shared_type*>(ptr);
//       }
//   };
template <typename AccumStatType, typename Tag>
struct traits;

namespace detail
{
template <typename AccumSet, typename AccumStatTag,
          typename SharedType, template<typename...> class Derived>
struct traits_common
{
   using shared_type = SharedType;
   static void serialize(AccumSet& acc, char* ptr)
   {
      *reinterpret_cast<shared_type*>(ptr) =
         boost::accumulators::extractor<AccumStatTag>{}(acc);
   }

   static constexpr size_t size()
   {
      return sizeof(shared_type);
   }

   static void getTitle(void* ptr, std::stringstream& ss)
   {
      ss << Derived<AccumSet, AccumStatTag>::stat_name;
   }

   static void dumpStat(void* ptr, std::stringstream& ss)
   {
      ss << *reinterpret_cast<shared_type*>(ptr);
   }
};
}
}

