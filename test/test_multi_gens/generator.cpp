#include "common.h"
#include "hw_intf.h"
#include <memory>

constexpr bool IsOperational = true;

template <>
void initializeStatistics<IsOperational>()
{
   auto logger = std::make_shared<LoggerGenerator>(STAT_LOG_LOGGER_NAME,
         STAT_LOG_LOGGER_SIZE_BYTES);

   auto& hwIntfStat = stat_log::getStatSingleton<OpStatHwIntf>();
   hwIntfStat.init(STAT_LOG_HW_INTF_SHM_NAME);
   hwIntfStat.addLogger(logger);

   auto& macSisStat = stat_log::getStatSingleton<OpStatMacSis>();
   macSisStat.init(STAT_LOG_HW_INTF_SHM_NAME);
   macSisStat.addLogger(logger);
}


using namespace stat_log;

int main(int argc, char** argv)
{
   initializeStatistics<IsOperational>();
   auto& macSisStat = getStatSingleton<OpStatMacSis>();
   auto& hwIntfStat = getStatSingleton<OpStatHwIntf>();

   macSisStat.writeStat<SIS::MAC_PKTS_DOWN_TAG>(88);
   hwIntfStat.writeStat<HW_INTERFACE::MISC_FPGA_FAULT_TAG>(2);
   hwIntfStat.writeStat<HW_INTERFACE::MISC_FPGA_FAULT_TAG>(2);

   macSisStat.getInfoLog<MAC>() << "HELLO from MAC";
   hwIntfStat.getAlertLog<HW_INTERFACE>() << "HELLO from HW";

   while(true)
   {
      std::this_thread::sleep_for(std::chrono::seconds{2});
      macSisStat.getInfoLog<MAC>() << "HELLO from MAC (loop)";
      hwIntfStat.getAlertLog<HW_INTERFACE>() << "HELLO from HW (loop)";
   }
   macSisStat.stop();
   hwIntfStat.stop();
   return 0;
}
