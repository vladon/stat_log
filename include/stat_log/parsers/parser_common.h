#pragma once
#include <stat_log/util/utils.h>
#include <stat_log/defs.h>
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
   PRINT_TAG,
};

//TODO: move this somewhere else (in the "printing" code?)
struct TagInfo
{
   const char* const name;
   std::type_index tag_index;
   std::size_t depth;
   bool is_stat_tag;
};

inline bool printingRequired(StatCmd cmd)
{
   switch(cmd)
   {
      case StatCmd::PRINT_STAT_TYPE:
      case StatCmd::DUMP_STAT:
      case StatCmd::LOG_LEVEL:
      case StatCmd::PRINT_TAG:
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
      case StatCmd::PRINT_TAG:
         return true;
      default:
         return false;
   }
}

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
   std::vector<std::string> exclude_tags;
   std::vector<std::string> exclude_log_levels;
   bool show_tag = true;
   bool show_time_stamp = true;
   bool show_log_level = true;
};

template <typename TagNode, typename StatLogControl>
void processCommands(StatLogControl& stat_log_control, const std::string& user_cmds)
{
   namespace po = boost::program_options;

   auto cmd = StatCmd::NO_CMD;
   boost::any cmd_arg;

   auto desc = detail::getProgramOptions();
   po::variables_map vm;
   auto parsed = po::command_line_parser(tokenize(user_cmds))
         .options(desc)
         .allow_unregistered()
         .run();
   po::store(parsed, vm);

   setStartingDepth(TagNode::depth);
   cmd = StatCmd::NO_CMD;

   if(vm["show-tags"].as<bool>())
   {
      cmd = StatCmd::PRINT_TAG;
   }
   else if(vm["stat-types"].as<bool>())
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
   }
   else if(vm.count("output-log"))
   {
      LogOutputCommand log_cmd;
      auto arg_vec = vm["output-log"].as<std::vector<std::string>>();
      po::options_description log_desc("output log options");
      log_desc.add_options()
         ("help", "Show help")
         ("out-file", po::value<std::string>(),
          "Output file name, if not given will default to stdout.")
         ("log-idx", po::value<int>(),  "Logger index")
         ("no-tag", "Don't show tag")
         ("no-time-stamp", "Don't show time stamp")
         ("no-log-level", "Don't show log level")
         ("exclude-tags", po::value<std::vector<std::string>>()->multitoken()->zero_tokens(),
            "Do not output log entries corresponding to the supplied list of tags")
         ("exclude-log-levels", po::value<std::vector<std::string>>()->multitoken()->zero_tokens(),
            "Do not output log entries corresponding to the supplied list of log-levels\n"
            "For example \"DEBUG INFO\"")
         ;
      int logger_idx = 0;
      auto opts = po::collect_unrecognized(parsed.options, po::include_positional);
      po::store(po::command_line_parser(opts).options(log_desc).run(), vm);
      if(vm.count("help"))
      {
         std::cout << log_desc << std::endl;
         std::exit(0);
      }
      if(vm.count("log-file"))
      {
         log_cmd.output_filename = vm["log-file"].as<std::string>();
      }
      if(vm.count("log-idx"))
      {
         logger_idx = vm["log-idx"].as<int>();
      }
      if(vm.count("no-tag"))
      {
         log_cmd.show_tag = false;
      }
      if(vm.count("no-time-stamp"))
      {
         log_cmd.show_time_stamp = false;
      }
      if(vm.count("no-log-level"))
      {
         log_cmd.show_log_level = false;
      }
      if(vm.count("exclude-tags"))
      {
         log_cmd.exclude_tags = vm["exclude-tags"].as<std::vector<std::string>>();
      }
      if(vm.count("exclude-log-levels"))
      {
         log_cmd.exclude_log_levels = vm["exclude-log-levels"].as<std::vector<std::string>>();
      }
      boost::any cmd_any = log_cmd;
      stat_log_control.outputLog(logger_idx, cmd_any);
   }
   if(vm.count("help"))
   {
      std::cout << desc << std::endl;
      std::exit(0);
   }

   if(cmd != StatCmd::NO_CMD)
   {
      constexpr bool IsParentNode = is_parent<TagNode>::value;
      using TheDoCmd = DoCmd<StatLogControl, IsParentNode>;
      TheDoCmd::template Go<TagNode>(stat_log_control, cmd, cmd_arg);
   }
}

}
