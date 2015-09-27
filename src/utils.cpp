#include <stat_log/util/command.h>
#include <stat_log/util/utils.h>
#include <stat_log/util/printer.h>

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

namespace stat_log
{
po::options_description getProgramOptions()
{
   po::options_description desc("Options", TERM_NUM_COLUMNS);
   desc.add_options()
   ("help,h", po::value<std::string>()->implicit_value(""),
    "Show help menu.\n")

   ("show-tags,s", po::bool_switch()->default_value(false),
    "Show the tag hierarchy.\n")

   ("tags,t", po::value<std::vector<std::string>>()->multitoken()->zero_tokens(),
    "Limit the effects to the descendants of the given tag(s).\n"
    "Delimit multiple tags with a space.\n"
    "\".\" should be used when specifying hierarchical tags.\n")

   //TODO: add option if TimeSeries statistics are to be dumped as well.
   ("dump-stats,d", po::bool_switch()->default_value(false),
    "Dump all statistics.\n")

   ("stat-types", po::bool_switch()->default_value(false),
    "Print the type of each statistic.\n")

   ("clear-stats,c", po::bool_switch()->default_value(false),
    "Zero out statistics.\n")

   ("log-level,L", po::value<std::vector<std::string>>()->multitoken()->zero_tokens(),
    "Set/Show per tag log level. Args\n"
    "\t<loggerIdx> [<LogLevel>]\n"
    "where loggerIdx is the index of the logger, and LogLevel is\n"
    "the new value for the log level (if this argument is not\n"
    "specified the current log level is displayed).\n")

   ("output-log,D", po::value<std::vector<std::string>>()->multitoken()->zero_tokens(),
    "type \"output-log --help\" for all options.\n"
    "This option is NOT compatible with the --tags option.\n")
   ;
   return desc;
}


std::vector<std::string> getTagNames(int argc, char** argv)
{
   namespace po = boost::program_options;
   std::string tag_str;
   auto desc = getProgramOptions();

   po::variables_map vm;
   po::store(po::command_line_parser(argc, argv)
         .options(desc)
         .allow_unregistered()
         .run(),
         vm);
   po::notify(vm);
   if(vm.count("tags"))
      return vm["tags"].as<std::vector<std::string>>();
   return std::vector<std::string>{};
}

void parseCommandLineArgs(int argc, char** argv,
      std::vector<std::string>& tag_strings,
      StatCmd& cmd,
      boost::any& cmd_arg,
      PrintOptions& print_options)
{
   //TODO: fill in the print_options arg.
   //  --indices, --table, etc.
   cmd = StatCmd::NO_CMD;
   auto tags = getTagNames(argc, argv);
   tag_strings.insert(tag_strings.end(), tags.begin(), tags.end());
   if(tag_strings.empty())
      tag_strings.push_back("");

   auto desc = getProgramOptions();
   po::variables_map vm;
   auto parsed = po::command_line_parser(argc, argv)
         .options(desc)
         .allow_unregistered()
         .run();
   po::store(parsed, vm);

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
      if(vm.count("out-file"))
      {
         log_cmd.output_filename = vm["out-file"].as<std::string>();
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
      cmd = StatCmd::DUMP_LOG;
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

}
