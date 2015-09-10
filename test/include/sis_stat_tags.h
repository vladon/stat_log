#pragma once
#include <stat_log/util/make_stat_tags.h>

namespace sis
{
   struct SisBase{};

   SL_MAKE_TAGS_NAMED_BASE(
               StatTags,
               SisBase,
               (MAC_PKTS_DOWN)
               (MAC_PKTS_UP)
               (LINK_QUALITY)
               (RECEIVE_STATUS)
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
