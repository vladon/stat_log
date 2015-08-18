#pragma once
#include "stat_log/util/make_stat_tags.h"

struct MacBase {};

struct MAC
{
   NAME = "MAC";
   MAKE_STAT_TAGS_NAMED_BASE(
         MacBase,
      (IP_PKTS_DOWN)
      (IP_PKTS_UP)
      (BUFFER_OVERFLOW)
   )
};

