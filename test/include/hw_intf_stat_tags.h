#pragma once
#include <stat_log/util/make_stat_tags.h>


namespace hw_intf
{
constexpr int num_fpga_faults = 5;
constexpr int num_fpga_interrupts = 3;

struct HwIntfBase {};
SL_MAKE_TAGS_NAMED_BASE(
      StatTags,
      HwIntfBase,
      (FPGA_FAULT)
      (BYTES_SENT)
      (BYTES_RCVD)
      (INTERRUPT)
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





