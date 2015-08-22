#pragma once
#include "hw_intf_stat_tags.h"

#include "stat_log/stat_log.h"
#include "stat_log/backends/shared_mem_backend.h"
#include "stat_log/stats/stats_common.h"
#include "stat_log/loggers/shared_memory_logger.h"
#include "stat_log/stats/simple_status.h"


namespace stat_log
{

   template <typename Tag>
   struct stat_tag_to_type<Tag, typename std::enable_if<
      std::is_base_of<HwIntfBase, Tag>::value
      >::type
   >
   {
      using type = SimpleStatus<int>;
   };
}


using LoggerGenerator = stat_log::shared_mem_logger_generator;
using OpStatHwIntf = stat_log::LogStatOperational<HW_INTERFACE>;
using ControlStatHwIntf = stat_log::LogStatControl<HW_INTERFACE>;
