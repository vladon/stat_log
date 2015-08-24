#pragma once
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <boost/any.hpp>

#include <algorithm>
#include <string>
#include <iostream>
#include <memory>
#include <regex>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <assert.h>

#define TERM_NUM_COLUMNS 100

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
};

inline bool printingRequired(StatCmd cmd, bool is_parent)
{
   switch(cmd)
   {
      case StatCmd::PRINT_STAT_TYPE:
      case StatCmd::DUMP_STAT:
         return true;
      case StatCmd::LOG_LEVEL:
         return is_parent;
      default:
         return false;
   }
}

template<typename Stat, bool IsParent>
struct DoCmd;

struct LogLevelCommand
{
   bool set_log_level = false;
   int logger_idx = 0;
   std::string new_log_level = "";
};

struct LogOutputCommand
{
   std::string output_filename;
   bool show_tag;
   bool show_time_stamp;
   bool show_log_level;
};
}


