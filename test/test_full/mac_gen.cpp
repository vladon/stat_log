#include "mac_stat_tags.h"
#include <stat_log/util/compile_proxy.h>

void genStats_MAC()
{
   stat_log::writeStat<mac::IP_PKTS_RCVD_TAG>(1);
}

