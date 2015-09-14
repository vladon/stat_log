#include "sis_stat_tags.h"
#include <stat_log/util/compile_proxy.h>
#include <random>

void genStats_SIS()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 1);
   #if 1
   int counter = 0;
   while(counter < 100)
   {
      for(int i = 0; i < 10; ++i)
            stat_log::writeStat<sis::PROP_DELAY_TAG>(i, dis(gen)*(i+1)*10 + i*100);
      ++counter;
   }
   #endif


}
