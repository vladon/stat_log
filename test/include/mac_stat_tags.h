#pragma once
#include <stat_log/util/make_stat_tags.h>

namespace mac
{
struct MacBase {};

SL_MAKE_TAGS_NAMED_BASE(
      StatTags,
      MacBase,
   (IP_PKTS_SENT)
   (IP_PKTS_RCVD)
   (IP_PKT_SENT_SIZE)
   (IP_PKT_RCVD_SIZE)
   (TX_POWER_LEVEL)
   (RX_POWER_LEVEL)
);

#if 1
struct MAC_STATS
{
   SL_NAME = "MAC_STATS";
   using children = StatTags;
};

struct MAC_LOG
{
   SL_NAME = "MAC_LOG";
};

#else
//Added a bunch more stats just to get a feel for the scalability of this library
// w.r.t. compile time.  Adding these extra stats increased the compile
// time by ~40%.  Much more benchmarking should be done here.

struct TEST_TAG1
{
   SL_NAME = "TEST1";
   MAKE_STAT_TAGS_NAMED_BASE(
         MacBase,
         (TEST1_STAT1)
         (TEST1_STAT2)
         );
};

struct TEST_TAG2
{
   SL_NAME = "TEST2";
   MAKE_STAT_TAGS_NAMED_BASE(
         MacBase,
         (TEST2_STAT1)
         (TEST2_STAT2)
         );
};


struct MAC
{
   SL_NAME = "MAC";
   MAKE_STAT_TAG_STRUCT_BASE( IP_PKTS_DOWN, MacBase);
   MAKE_STAT_TAG_STRUCT_BASE( IP_PKTS_UP, MacBase);
   MAKE_STAT_TAG_STRUCT_BASE( BUFFER_OVERFLOW, MacBase);
   MAKE_STAT_TAG_STRUCT_BASE( BUFFER_OVERFLOW1, MacBase);
   MAKE_STAT_TAG_STRUCT_BASE( BUFFER_OVERFLOW2, MacBase);
   MAKE_STAT_TAG_STRUCT_BASE( BUFFER_OVERFLOW3, MacBase);
   MAKE_STAT_TAG_STRUCT_BASE( BUFFER_OVERFLOW4, MacBase);
   MAKE_STAT_TAG_STRUCT_BASE( BUFFER_OVERFLOW5, MacBase);
   MAKE_STAT_TAG_STRUCT_BASE( BUFFER_OVERFLOW6, MacBase);
   MAKE_STAT_TAG_STRUCT_BASE( BUFFER_OVERFLOW7, MacBase);
   MAKE_STAT_TAG_STRUCT_BASE( BUFFER_OVERFLOW8, MacBase);
   MAKE_STAT_TAG_STRUCT_BASE( BUFFER_OVERFLOW9, MacBase);
   MAKE_STAT_TAG_STRUCT_BASE( BUFFER_OVERFLOW10, MacBase);

   using ChildTypes = MAKE_STAT_LIST(
         (IP_PKTS_DOWN_TAG)
         (IP_PKTS_UP_TAG)
         (BUFFER_OVERFLOW_TAG)
         (TEST_TAG1)
         (TEST_TAG2)
         (BUFFER_OVERFLOW1_TAG)
         (BUFFER_OVERFLOW2_TAG)
         (BUFFER_OVERFLOW3_TAG)
         (BUFFER_OVERFLOW4_TAG)
         (BUFFER_OVERFLOW5_TAG)
         (BUFFER_OVERFLOW6_TAG)
         (BUFFER_OVERFLOW7_TAG)
         (BUFFER_OVERFLOW8_TAG)
         (BUFFER_OVERFLOW9_TAG)
         (BUFFER_OVERFLOW10_TAG)
         );

};

#endif
}
