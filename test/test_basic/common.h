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

struct TOP
{
   SL_NAME = "TOP_LEVEL";
   using ChildTypes = MAKE_STAT_LIST
   (
      (MAC)
      (SIS)
      (HW_INTERFACE)
   );
};

constexpr bool IsOperational = true;

using OpStat       = stat_log::LogStatOperational<TOP>;
using ControlStat  = stat_log::LogStatControl<TOP>;
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
