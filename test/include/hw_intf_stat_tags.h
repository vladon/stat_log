#pragma once
#include <stat_log/util/make_stat_tags.h>


namespace hw_intf
{
struct HwIntfBase {};
SL_MAKE_TAGS_NAMED_BASE(
      StatTags,
      HwIntfBase,
      (MISC_FPGA_FAULT)
      (BUFFER_OVERFLOW)
);

struct HW_INTF_STATS
{
   SL_NAME = "HW_INTF_STATS";
   using children = StatTags;
};

struct HW_INTF_LOG
{
   SL_NAME = "HW_INTF_LOG";
};
}





