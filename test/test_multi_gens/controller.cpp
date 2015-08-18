#include "common.h"

constexpr bool IsOperational = false;

template <>
void initializeStatistics<IsOperational>()
{
   //TODO: pass the logger to init()
   stat_log::getStatSingleton<ControlStatHwIntf>().init(STAT_LOG_HW_INTF_SHM_NAME);
   stat_log::getStatSingleton<ControlStatMacSis>().init(STAT_LOG_MAC_SIS_SHM_NAME);
}

using namespace stat_log;

int main(int argc, char** argv)
{
   initializeStatistics<IsOperational>();
   auto& macSisStat = getStatSingleton<ControlStatMacSis>();
   auto& hwIntfStat = getStatSingleton<ControlStatHwIntf>();

   //TODO: is this a good way to combine the control
   // functionality of multiple stat controllers?
   macSisStat.parseUserCommands(argc, argv);
   hwIntfStat.parseUserCommands(argc, argv);
   return 0;
}
