#include "sis_stat_tags.h"
#include "stat_proxy_interface.h"

void genStats_SIS()
{
   int counter = 0;
   for(int i = 0; i < 4; ++i)
      for(int j = 0; j < 6; ++j, ++counter)
         writeStatProxy<SIS::MAC_PKTS_DOWN_TAG>(i, j, counter);

}
