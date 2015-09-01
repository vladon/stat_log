#pragma once
#include <boost/accumulators/statistics/count.hpp>

namespace stat_log
{
   namespace accumulator
   {
      template <typename AccumSet>
      struct traits<AccumSet, boost::accumulators::tag::count>
      {
         using SharedType = std::size_t;
         static void serialize(AccumSet& acc, char* ptr)
         {
            *reinterpret_cast<SharedType*>(ptr) =
               boost::accumulators::count(acc);
         }

         static constexpr size_t size()
         {
            return sizeof(SharedType);
         }

         static constexpr const char* stat_name = "count";

         static void dumpStat(void* ptr)
         {
            std::cout << *reinterpret_cast<SharedType*>(ptr);
         }
      };
   }
}

