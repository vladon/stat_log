#include "common.h"

template <>
void initializeStatistics<false == IsOperational>()
{
   auto logger = std::make_shared<LoggerRetriever>(STAT_LOG_LOGGER_NAME,
         STAT_LOG_LOGGER_SIZE_BYTES);
   auto& hwIntfStat = stat_log::getStatSingleton<ControlStatHwIntf>();
   hwIntfStat.init(STAT_LOG_HW_INTF_SHM_NAME);
   hwIntfStat.addLogger(logger);

   auto& macSisStat = stat_log::getStatSingleton<ControlStatMacSis>();
   macSisStat.init(STAT_LOG_MAC_SIS_SHM_NAME);
   macSisStat.addLogger(logger);
}

template <>
void initializeStatistics<true == IsOperational>()
{
   auto logger = std::make_shared<LoggerGenerator>(STAT_LOG_LOGGER_NAME,
         STAT_LOG_LOGGER_SIZE_BYTES);

   auto& hwIntfStat = stat_log::getStatSingleton<OpStatHwIntf>();
   hwIntfStat.init(STAT_LOG_HW_INTF_SHM_NAME);
   hwIntfStat.addLogger(logger);

   auto& macSisStat = stat_log::getStatSingleton<OpStatMacSis>();
   macSisStat.init(STAT_LOG_MAC_SIS_SHM_NAME);
   macSisStat.addLogger(logger);
}
