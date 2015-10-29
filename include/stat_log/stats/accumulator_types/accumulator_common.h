//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

#pragma once
#include <boost/accumulators/framework/extractor.hpp>
#include <boost/accumulators/statistics/count.hpp>
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
//       static void dumpStat(void* ptr, bool is_valid, std::stringstream& ss)
//       {
//          ss << *reinterpret_cast<shared_type*>(ptr);
//          if(is_valid)
//             ss << *reinterpret_cast<shared_type*>(ptr);
//          else
//             ss << shared_type{};
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
      using namespace boost::accumulators;
      auto& shm_value = *reinterpret_cast<shared_type*>(ptr);
      if(extractor<tag::count>{}(acc) > 0)
         shm_value = boost::accumulators::extractor<AccumStatTag>{}(acc);
      else
         shm_value = static_cast<shared_type>(0);

   }

   static constexpr size_t size()
   {
      return sizeof(shared_type);
   }

   static void getTitle(void* ptr, std::stringstream& ss)
   {
      ss << Derived<AccumSet, AccumStatTag>::stat_name;
   }

   static void dumpStat(void* ptr, bool is_valid, std::stringstream& ss)
   {
      if(is_valid)
         ss << *reinterpret_cast<shared_type*>(ptr);
      else
         ss << shared_type{}; //Valid initialization to "0"
   }
};

} //detail
} //accumulator
} //stat_log

