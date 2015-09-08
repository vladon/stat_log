#pragma once
#include <stat_log/util/utils.h>
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
#include <typeindex>
#include <assert.h>

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

struct TagInfo
{
   const char* const name;
   std::type_index tag_index;
   std::size_t depth;
   bool is_stat_tag;
};

#if 1
inline bool printingRequired(StatCmd cmd)
{
   switch(cmd)
   {
      case StatCmd::PRINT_STAT_TYPE:
      case StatCmd::DUMP_STAT:
      case StatCmd::LOG_LEVEL:
         return true;
      default:
         return false;
   }
}


void setStartingDepth(size_t start_depth);
void printHeader(StatCmd cmd, const TagInfo& tag_info);
void printFooter(StatCmd cmd);

inline bool isStatisticCommand(StatCmd cmd)
{
   switch(cmd)
   {
      case StatCmd::PRINT_STAT_TYPE:
      case StatCmd::DUMP_STAT:
      case StatCmd::CLEAR_STAT:
      case StatCmd::DUMP_TIMESERIES:
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
         return true;
      default:
         return false;
   }
}
#endif

template<typename StatLogControl, bool IsParent>
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

template <typename TagNode, bool IsParentNode, typename StatLogControl>
void processCommandsCommon(StatLogControl& stat_log_control, const std::string& user_cmds,
      StatCmd& cmd, boost::any& cmd_arg)
{
   namespace po = boost::program_options;

   auto desc = detail::getProgramOptions();
   po::variables_map vm;
   po::store(po::command_line_parser(tokenize(user_cmds))
         .options(desc)
         .run(),
         vm);

   if(vm.count("help"))
   {
      std::cout << desc << std::endl;
      std::exit(0);
   }

   setStartingDepth(TagNode::depth);
   cmd = StatCmd::NO_CMD;

   if(vm["stat-types"].as<bool>())
   {
      cmd = StatCmd::PRINT_STAT_TYPE;
   }
   else if(vm["dump-stats"].as<bool>())
   {
      cmd = StatCmd::DUMP_STAT;
   }
   else if(vm["clear-stats"].as<bool>())
   {
      cmd = StatCmd::CLEAR_STAT;
   }
   else if(vm.count("log-level"))
   {
      //Args: <LoggerIdx> [<LogLevel>]
      //No LogLevel arg will print the current log level
      auto arg_vec = vm["log-level"].as<std::vector<std::string>>();
      int logger_idx = 0;
      if(arg_vec.size() > 0)
      {
         try
         {
            logger_idx = boost::lexical_cast<int>(arg_vec[0]);
         }
         catch(boost::bad_lexical_cast&)
         {
            std::cerr << "Invalid logger idx!\n";
         }
      }
      LogLevelCommand logCmd;
      if(arg_vec.size() == 2)
      {
         logCmd.new_log_level = arg_vec[1];
         logCmd.set_log_level = true;
      }
      logCmd.logger_idx = logger_idx;
      cmd = StatCmd::LOG_LEVEL;
      cmd_arg = logCmd;
      using TheDoCmd = DoCmd<StatLogControl, IsParentNode>;
      TheDoCmd::template Go<TagNode>(stat_log_control, cmd, cmd_arg);
      cmd = StatCmd::NO_CMD;
   }
   else if(vm.count("output-log"))
   {
      auto arg_vec = vm["output-log"].as<std::vector<std::string>>();
      std::string output_file;
      int logger_idx = 0;
      if(arg_vec.size() >  0)
      {
         try
         {
            logger_idx = boost::lexical_cast<int>(arg_vec[0]);
         }
         catch(boost::bad_lexical_cast&)
         {
            std::cerr << "Invalid logger idx!\n";
         }
      }
      if(arg_vec.size() > 1)
         output_file = arg_vec[1];
      LogOutputCommand log_cmd;
      log_cmd.output_filename = output_file;
      //TODO: make viewing tags, timestamps and log_levels configurable.
      log_cmd.show_tag = true;
      log_cmd.show_time_stamp = true;
      log_cmd.show_log_level = true;
      boost::any cmd_any = log_cmd;
      stat_log_control.outputLog(logger_idx, cmd_any);
   }
}

}
