#pragma once
#include <boost/accumulators/statistics/min.hpp>
#include <stat_log/stats/accumulator_types/accumulator_common.h>

namespace stat_log
{
   namespace accumulator
   {
      template <typename AccumSet>
      struct traits<AccumSet, boost::accumulators::tag::min>
         : detail::traits_common<AccumSet, boost::accumulators::tag::min,
            typename AccumSet::sample_type, traits>
      {
         static constexpr const char* stat_name = "min";
      };
   }
}

