//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

#pragma once
#include <boost/accumulators/statistics/max.hpp>
#include <stat_log/stats/accumulator_types/accumulator_common.h>

namespace stat_log
{
   namespace accumulator
   {
      template <typename AccumSet>
      struct traits<AccumSet, boost::accumulators::tag::max>
         : detail::traits_common<AccumSet, boost::accumulators::tag::max,
            typename AccumSet::sample_type, traits>
      {
         static constexpr const char* stat_name = "max";
      };
   }
}

