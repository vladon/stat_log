#pragma once
#include <stat_log/defs.h>
#include <boost/any.hpp>

#include <string>
#include <vector>

namespace stat_log
{
enum class StatCmd
{
   NO_CMD,
   PRINT_STAT_TYPE,
   DUMP_STAT,
   CLEAR_STAT,
   DUMP_TIMESERIES,
   LOG_LEVEL,
   DUMP_LOG,
   PRINT_TAG,
};

inline bool isStatisticCommand(StatCmd cmd)
{
   switch(cmd)
   {
      case StatCmd::PRINT_STAT_TYPE:
      case StatCmd::DUMP_STAT:
      case StatCmd::CLEAR_STAT:
      case StatCmd::DUMP_TIMESERIES:
      case StatCmd::PRINT_TAG:
         return true;
      default:
         return false;
   }
}

inline bool isLogCommand(StatCmd cmd)
{
   switch(cmd)
   {
      case StatCmd::LOG_LEVEL:
      case StatCmd::DUMP_LOG:
      case StatCmd::PRINT_TAG:
         return true;
      default:
         return false;
   }
}

struct LogLevelCommand
{
   bool set_log_level = false;
   int logger_idx = 0;
   std::string new_log_level = "";
};

struct LogOutputCommand
{
   int logger_idx = 0;
   std::string output_filename;
   std::vector<std::string> exclude_tags;
   std::vector<std::string> exclude_log_levels;
   bool show_tag = true;
   bool show_time_stamp = true;
   bool show_log_level = true;
};

//Used to retrieve the string view of each statistic.
struct StatCmdOutput
{
   std::string entryTitle;
   std::vector<std::string> entries;
   std::vector<size_t> dimensionSizes;
};

}
