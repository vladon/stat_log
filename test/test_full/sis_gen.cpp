#include "sis_stat_tags.h"
#include <stat_log/util/compile_proxy.h>

void genStats_SIS()
{
   int counter = 0;
   for(int i = 0; i < 4; ++i)
      for(int j = 0; j < 6; ++j, ++counter)
         stat_log::writeStat<sis::MAC_PKTS_DOWN_TAG>(i, j, counter);

}
