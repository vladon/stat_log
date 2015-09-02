#include "common.h"
#include "hw_intf.h"
#include <memory>


using namespace stat_log;

int main(int argc, char** argv)
{
   initializeStatistics<true == IsOperational>();
   auto& macSisStat = getStatSingleton<OpStatMacSis>();
   auto& hwIntfStat = getStatSingleton<OpStatHwIntf>();

   macSisStat.writeStat<SIS::MAC_PKTS_DOWN_TAG>(88);
   hwIntfStat.writeStat<HW_INTERFACE::MISC_FPGA_FAULT_TAG>(1.0);

   macSisStat.getInfoLog<MAC>() << "HELLO from MAC";
   hwIntfStat.getAlertLog<HW_INTERFACE>() << "HELLO from HW";

   double val = 0.0;
   while(true)
   {
      std::this_thread::sleep_for(std::chrono::seconds{2});
      macSisStat.getInfoLog<MAC>() << "HELLO from MAC (loop)";
      hwIntfStat.getAlertLog<HW_INTERFACE>() << "HELLO from HW (loop)";
      hwIntfStat.writeStat<HW_INTERFACE::MISC_FPGA_FAULT_TAG>(val);
      val = val + 1.0;
   }
   macSisStat.stop();
   hwIntfStat.stop();
   return 0;
}
