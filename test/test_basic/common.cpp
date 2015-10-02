//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

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
