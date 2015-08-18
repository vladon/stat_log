#pragma once
#include "stat_log/util/make_stat_tags.h"

struct HwIntfBase {};

struct HW_INTERFACE
{
   NAME = "HW_INTERFACE";
   MAKE_STAT_TAGS_NAMED_BASE(
      HwIntfBase,
      (MISC_FPGA_FAULT)
      (BUFFER_OVERFLOW)
   )
};




