//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

#pragma once
#include <stat_log/util/make_stat_tags.h>

namespace sis
{
constexpr int max_nbrs = 5;
constexpr int num_frequencies = 4;
constexpr int num_rx_frame_status = 4;

struct SisBase{};

SL_MAKE_TAGS_NAMED_BASE(
      StatTags,
      SisBase,
      (PROP_DELAY)
      (CHANNEL_QUALITY)
      (FRAME_RX_STATUS)
      (LINK_STATUS)
      );


struct SIS_STATS
{
   SL_NAME = "SIS_STAT";
   using children = StatTags;
};

struct SIS_LOG
{
   SL_NAME = "SIS_LOG";
};
}
