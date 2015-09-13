#pragma once
#include <stat_log/util/make_stat_tags.h>

namespace sis
{
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
