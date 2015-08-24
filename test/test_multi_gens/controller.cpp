#include "common.h"

constexpr bool IsOperational = false;

template <>
void initializeStatistics<IsOperational>()
{
   auto logger = std::make_shared<LoggerRetriever>(STAT_LOG_LOGGER_NAME,
         STAT_LOG_LOGGER_SIZE_BYTES);
   auto& hwIntfStat = stat_log::getStatSingleton<ControlStatHwIntf>();
   hwIntfStat.init(STAT_LOG_HW_INTF_SHM_NAME);
   hwIntfStat.addLogger(logger);

   auto& macSisStat = stat_log::getStatSingleton<ControlStatMacSis>();
   macSisStat.init(STAT_LOG_HW_INTF_SHM_NAME);
   macSisStat.addLogger(logger);
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
