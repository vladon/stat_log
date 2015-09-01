#include "hw_intf_stat_tags.h"
#include "stat_log/util/compile_proxy.h"

void genStats_HW_INTF()
{
   stat_log::writeStat<HW_INTERFACE::MISC_FPGA_FAULT_TAG>(1);
}
