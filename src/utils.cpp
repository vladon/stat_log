#include "stat_log/util/utils.h"
#include "stat_log/parsers/parser_common.h"

#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>
#include <boost/algorithm/string.hpp>

#include <string>
#include <algorithm>
#include <vector>

namespace stat_log
{
namespace detail
{
   namespace po = boost::program_options;
   po::options_description getParentOptions()
   {
      po::options_description desc("Options", TERM_NUM_COLUMNS);
      desc.add_options()
         ("help", po::value<std::string>()->implicit_value(""),
          "Show help memu")

         ("component", po::value<std::string>()->default_value(""),
          "TODO")

         ("list-stats", po::bool_switch()->default_value(false),
          "List all statistics")

         ("log-level", po::value<std::vector<std::string>>()->multitoken()->zero_tokens(),
          "Set/Show per component log level. Args\n"
          "\t<loggerIdx> [<LogLevel>]\n"
          "where loggerIdx is the index of the logger, and LogLevel is new value\n"
          "for the log level (if this argument is not specified the\n"
          "current log level is displayed).")

         ("output-log", po::value<std::vector<std::string>>()->multitoken()->zero_tokens(),
          "Show log output. Args\n"
          "\t<loggerIdx> [<outputFile>]\n"
          "If outputFile is not provided the log is dumped to stdout.\n")
         ;
      return std::move(desc);
   }

   void indent(size_t level)
   {
      int i = level;
      while(i-- > 0)
         std::cout << "\t";
   }
}

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

std::string getComponentName(std::string cmd_line)
{
   namespace po = boost::program_options;
   std::string component_str;
   auto desc = detail::getParentOptions();

   po::variables_map vm;
   po::store(po::command_line_parser(tokenize(cmd_line))
         .options(desc)
         .run(),
         vm);
   po::notify(vm);
   return vm["component"].as<std::string>();
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
