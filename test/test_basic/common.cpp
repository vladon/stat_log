#include "common.h"

namespace
{
   template <typename StatType, typename LogType>
   void initializeCommon()
   {
      auto& stat = stat_log::getStatLogSingleton<StatType>();
      stat.init(STAT_LOG_SHM_NAME);
      auto text_logger = std::make_shared<LogType>(
            STAT_LOG_LOGGER_NAME,
            STAT_LOG_LOGGER_SIZE_BYTES);
      stat.addLogger(text_logger);
   }
}

template <>
void initializeStatistics<false == IsOperational>()
{
    initializeCommon<ControlStat, LoggerRetriever>();
}

template <>
void initializeStatistics<true == IsOperational>()
{
   initializeCommon<OpStat, LoggerGenerator>();
}
