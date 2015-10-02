//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

#include "mac_stat_tags.h"
#include "sis_stat_tags.h"
#include "hw_intf_stat_tags.h"

#include <stat_log/stat_log.h>
#include <stat_log/backends/shared_mem_backend.h>
#include <stat_log/loggers/shared_memory_logger.h>
#include <stat_log/stats/stats_common.h>
#include <stat_log/stats/simple_counter.h>

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

/*********************************
 * Statistic definitions
 *********************************/

struct TOP_STATS
{
   SL_NAME = "TOP_STAT";
   using children = SL_MAKE_LIST
   (
      (mac::MAC_STATS)
      (sis::SIS_STATS)
      (hw_intf::HW_INTF_STATS)
   );
};
struct TOP_LOG
{
   SL_NAME = "TOP_LOG";
   using children = SL_MAKE_LIST
   (
      (mac::MAC_LOG)
      (sis::SIS_LOG)
      (hw_intf::HW_INTF_LOG)
   );
};

constexpr bool IsOperational = true;

using OpStat       = stat_log::LogStatOperational<TOP_STATS, TOP_LOG>;
using ControlStat  = stat_log::LogStatControl<TOP_STATS, TOP_LOG>;
using LoggerGenerator = stat_log::shared_mem_logger_generator;
using LoggerRetriever = stat_log::shared_mem_logger_retriever;
/*********************************/

namespace stat_log
{
   template <typename Tag>
   struct stat_tag_to_type<Tag>
   {
      using type = SimpleCounter<int>;
   };
}


template <bool IsOperational>
void initializeStatistics();

#define STAT_LOG_SHM_NAME "STAT_LOG_BASIC"
#define STAT_LOG_LOGGER_NAME "SHM_LOGGER_BASIC"
#define STAT_LOG_LOGGER_SIZE_BYTES 4194304
