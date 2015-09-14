#include "common.h"
#include <memory>

using namespace stat_log;

int main(int argc, char** argv)
{
   initializeStatistics<true == IsOperational>();
   auto& stat = getStatLogSingleton<OpStat>();

   stat.writeStat<sis::CHANNEL_QUALITY_TAG>(88);
   stat.writeStat<hw_intf::FPGA_FAULT_TAG>(0x123);

   stat.getInfoLog<mac::MAC_LOG>() << "HELLO from MAC";
   stat.getAlertLog<hw_intf::HW_INTF_LOG>() << "HELLO from HW";

   while(true)
   {
      std::this_thread::sleep_for(std::chrono::seconds{2});
      stat.getInfoLog<mac::MAC_LOG>() << "HELLO from MAC (loop)";
      stat.getAlertLog<hw_intf::HW_INTF_LOG>() << "HELLO from HW (loop)";
      stat.writeStat<hw_intf::FPGA_FAULT_TAG>(1.0);
   }
   stat.stop();
   return 0;
}
