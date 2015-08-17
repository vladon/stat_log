#pragma once
#include "stat_log/stat_log.h"
#include "stat_log/backends/shared_mem_backend.h"
#include "stat_log/stats/stats_common.h"
#include "stat_log/stats/simple_counter.h"
#include "stat_log/loggers/shared_memory_logger.h"


struct HwIntfBase {};

struct HW_INTERFACE
{
   NAME = "HW_INTERFACE";
   MAKE_STAT_TAGS_NAMED_BASE(
      HwIntfBase,
      (MISC_FPGA_FAULT)
      (BUFFER_OVERFLOW)
   )
};
namespace stat_log
{

   template <typename Tag>
   struct stat_traits<Tag, typename std::enable_if<
      std::is_base_of<HwIntfBase, Tag>::value
      >::type
   >
   {
      using StatType = SimpleCounter<char>;
   };
}


using LoggerType = stat_log::shared_mem_logger;
using OpStatHwIntf = stat_log::LogStatOperational<HW_INTERFACE, LoggerType>;
using ControlStatHwIntf = stat_log::LogStatControl<HW_INTERFACE, LoggerType>;
