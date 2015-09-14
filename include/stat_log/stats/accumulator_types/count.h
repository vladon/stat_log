#pragma once
#include <boost/accumulators/statistics/count.hpp>
#include <iostream>

namespace stat_log
{
   namespace accumulator
   {
      template <typename AccumSet>
      struct traits<AccumSet, boost::accumulators::tag::count>
      {
         using shared_type = std::size_t;
         static void serialize(AccumSet& acc, char* ptr)
         {
            *reinterpret_cast<shared_type*>(ptr) =
               boost::accumulators::count(acc);
         }

         static constexpr size_t size()
         {
            return sizeof(shared_type);
         }

         static constexpr const char* stat_name = "count";

         static void dumpStat(void* ptr)
         {
            std::cout << *reinterpret_cast<shared_type*>(ptr);
         }
      };
   }
}

