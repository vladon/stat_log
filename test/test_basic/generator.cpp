#include "common.h"
#include <memory>


using namespace stat_log;

int main(int argc, char** argv)
{
   initializeStatistics<true == IsOperational>();
   auto& stat = getStatSingleton<OpStat>();

   stat.writeStat<SIS::MAC_PKTS_DOWN_TAG>(88);
   stat.writeStat<HW_INTERFACE::MISC_FPGA_FAULT_TAG>(1.0);

   stat.getInfoLog<MAC>() << "HELLO from MAC";
   stat.getAlertLog<HW_INTERFACE>() << "HELLO from HW";

   while(true)
   {
      std::this_thread::sleep_for(std::chrono::seconds{2});
      stat.getInfoLog<MAC>() << "HELLO from MAC (loop)";
      stat.getAlertLog<HW_INTERFACE>() << "HELLO from HW (loop)";
      stat.writeStat<HW_INTERFACE::MISC_FPGA_FAULT_TAG>(1.0);
   }
   stat.stop();
   return 0;
}
