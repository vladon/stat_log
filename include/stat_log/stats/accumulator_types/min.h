#pragma once
#include <boost/accumulators/statistics/min.hpp>
#include <iostream>

namespace stat_log
{
   namespace accumulator
   {
      template <typename AccumSet>
      struct traits<AccumSet, boost::accumulators::tag::min>
      {
         using shared_type = typename AccumSet::sample_type;
         static void serialize(AccumSet& acc, char* ptr)
         {
            *reinterpret_cast<shared_type*>(ptr) =
               boost::accumulators::min(acc);
         }

         static constexpr size_t size()
         {
            return sizeof(shared_type);
         }

         static constexpr const char* stat_name = "min";

         static void dumpStat(void* ptr)
         {
            std::cout << *reinterpret_cast<shared_type*>(ptr);
         }
      };
   }
}

