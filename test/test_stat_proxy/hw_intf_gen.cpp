#include "hw_intf_stat_tags.h"
#include "stat_proxy_interface.h"

void genStats_HW_INTF()
{
   stat_log::writeStat<HW_INTERFACE::MISC_FPGA_FAULT_TAG>(1);
}
