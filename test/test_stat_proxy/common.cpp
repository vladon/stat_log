#include "common.h"

template <>
void initializeStatistics<true == IsOperational>()
{
   auto logger = std::make_shared<LoggerGenerator>(STAT_LOG_LOGGER_NAME,
         STAT_LOG_LOGGER_SIZE_BYTES);
   auto& theStats = stat_log::getStatSingleton<OpStat>();
   theStats.init(STAT_LOG_SHM_NAME);
   theStats.addLogger(logger);
}

template <>
void initializeStatistics<false == IsOperational>()
{
   auto logger = std::make_shared<LoggerRetriever>(STAT_LOG_LOGGER_NAME,
         STAT_LOG_LOGGER_SIZE_BYTES);
   auto& theStat = stat_log::getStatSingleton<ControlStat>();
   theStat.init(STAT_LOG_SHM_NAME);
   theStat.addLogger(logger);
}

