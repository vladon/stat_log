#include "hw_intf_stat_tags.h"
#include <stat_log/util/compile_proxy.h>

void genStats_HW_INTF()
{
   #if 0
   stat_log::writeStat<hw_intf::FPGA_FAULT_TAG>(1);
   for(auto i = 0; i < 100; ++i)
   {
      stat_log::writeStat<hw_intf::FPGA_FAULT_TAG>(i%5);
   }
   #endif
}
