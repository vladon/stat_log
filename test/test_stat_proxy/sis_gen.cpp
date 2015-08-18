#include "sis_stat_tags.h"
#include "stat_proxy_interface.h"

void genStats_SIS()
{
   writeStatProxy<SIS::MAC_PKTS_DOWN_TAG>(88);
}
