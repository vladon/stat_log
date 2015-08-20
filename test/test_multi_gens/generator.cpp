#include "common.h"

constexpr bool IsOperational = true;

template <>
void initializeStatistics<IsOperational>()
{
   //TODO: pass the logger to init()
   stat_log::getStatSingleton<OpStatHwIntf>().init(STAT_LOG_HW_INTF_SHM_NAME);
   stat_log::getStatSingleton<OpStatMacSis>().init(STAT_LOG_MAC_SIS_SHM_NAME);
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

   std::this_thread::sleep_for(std::chrono::seconds{10});
   macSisStat.stop();
   hwIntfStat.stop();
   return 0;
}
