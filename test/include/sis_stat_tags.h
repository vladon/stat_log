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
