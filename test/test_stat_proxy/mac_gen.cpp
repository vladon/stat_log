#include "mac_stat_tags.h"
#include "stat_log/util/compile_proxy.h"

void genStats_MAC()
{
   stat_log::writeStat<MAC::IP_PKTS_UP_TAG>(1);
}

