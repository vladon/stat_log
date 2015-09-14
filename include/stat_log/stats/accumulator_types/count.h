#pragma once
#include <boost/accumulators/statistics/count.hpp>
#include <stat_log/stats/accumulator_types/accumulator_common.h>

namespace stat_log
{
   namespace accumulator
   {
      template <typename AccumSet>
      struct traits<AccumSet, boost::accumulators::tag::count>
         : detail::traits_common<AccumSet, boost::accumulators::tag::count,
            std::size_t, traits>
      {
         static constexpr const char* stat_name = "count";
      };
   }
}

