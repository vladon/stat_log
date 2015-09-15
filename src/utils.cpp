#include <stat_log/util/command.h>
#include <stat_log/util/utils.h>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/token_functions.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <string>
#include <algorithm>
#include <vector>
#include <iostream>

#define TERM_NUM_COLUMNS 100

namespace po = boost::program_options;
namespace
{
std::vector<std::string> tokenize(const std::string& input)
{
   typedef boost::escaped_list_separator<char> separator_type;
   separator_type separator("\\",     // The escape characters.
         "= ",     // The separator characters.
         "\"\'");  // The quote characters.

   // Tokenize the intput.
   boost::tokenizer<separator_type> tokens(input, separator);

   // Copy non-empty tokens from the tokenizer into the result.
   std::vector<std::string> result;
   copy_if(tokens.begin(), tokens.end(), std::back_inserter(result),
         [](auto& str)
         {
         return !str.empty();

         });
   return result;
}
}

namespace stat_log
{
po::options_description getProgramOptions()
{
   po::options_description desc("Options", TERM_NUM_COLUMNS);
   desc.add_options()
   ("help", po::value<std::string>()->implicit_value(""),
    "Show help memu")

   ("component", po::value<std::string>()->default_value(""),
    "TODO")

   ("show-tags", po::bool_switch()->default_value(false),
    "Show the tag hierarchy.")

   ("dump-stats", po::bool_switch()->default_value(false),
    "Dump all statistics")

   ("stat-types", po::bool_switch()->default_value(false),
    "Print the type of each statistic.")

   ("clear-stats", po::bool_switch()->default_value(false),
    "Zero out statistics.")

   ("log-level", po::value<std::vector<std::string>>()->multitoken()->zero_tokens(),
    "Set/Show per component log level. Args\n"
    "\t<loggerIdx> [<LogLevel>]\n"
    "where loggerIdx is the index of the logger, and LogLevel is\n"
    "the new value for the log level (if this argument is not\n"
    "specified the current log level is displayed).")

   ("output-log", po::value<std::vector<std::string>>()->multitoken()->zero_tokens(),
    "type \"output-log --help\" for all options")
   ;
   return desc;
}


#if 0
std::string getComponentName(std::string cmd_line)
{
   namespace po = boost::program_options;
   std::string component_str;
   auto desc = detail::getProgramOptions();

   po::variables_map vm;
   po::store(po::command_line_parser(tokenize(cmd_line))
         .options(desc)
         .allow_unregistered()
         .run(),
         vm);
   po::notify(vm);
   return vm["component"].as<std::string>();
}
#endif

void parseCommandLineArgs(int argc, char** argv,
      std::vector<std::string>& component_strings,
      StatCmd& cmd, boost::any& cmd_arg)
{
   //TODO: the "component" arg could have multiple
   // components --> put each in the component_strings arg.
   cmd = StatCmd::NO_CMD;
   std::vector<std::string> user_strings;
   for(int i = 1; i < argc; ++i)
      user_strings.push_back(argv[i]);

   std::string user_cmd_line = boost::algorithm::join(user_strings, " ");

   auto desc = getProgramOptions();
   po::variables_map vm;
   auto parsed = po::command_line_parser(tokenize(user_cmd_line))
         .options(desc)
         .allow_unregistered()
         .run();
   po::store(parsed, vm);

   // setStartingDepth(TagNode::depth);

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
         log_cmd.logger_idx = vm["log-idx"].as<int>();
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
      // stat_log_control.outputLog(logger_idx, cmd_any);
      cmd = StatCmd::LOG_LEVEL;
      cmd_arg = log_cmd;
   }
   if(vm.count("help"))
   {
      std::cout << desc << std::endl;
      std::exit(0);
   }
}

std::tuple<std::string, std::string>
getHeadTail(std::string s, char delim)
{
   auto pos = s.find(delim);
   std::tuple<std::string, std::string> ret;
   if(pos != std::string::npos)
   {
      std::get<0>(ret) =  s.substr(0,pos);
      std::get<1>(ret) =  s.substr(pos+1);
   }
   else
   {
      std::get<0>(ret) =  s;
   }
   return ret;
}


#if 0
void printHeader(StatCmd cmd, const TagInfo& tag_info)
{
   if(printingRequired(cmd))
   {
      detail::indent(tag_info.depth - starting_tag_depth);
      std::cout << tag_info.name << std::endl;
      if(cmd != StatCmd::PRINT_TAG)
      {
         detail::indent(tag_info.depth - starting_tag_depth);
         std::cout << "  ";
      }
   }
}

void printFooter(StatCmd cmd)
{
   if(printingRequired(cmd) && cmd != StatCmd::PRINT_TAG)
   {
      std::cout << std::endl;
   }
}
#endif
}
