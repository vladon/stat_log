#pragma once

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>
#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <regex>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <assert.h>

#define TERM_NUM_COLUMNS 100

namespace stat_log
{
namespace po = boost::program_options;

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
void printComponents(std::string blah)
{
   std::cout << "In print components" << std::endl;
}

   template <typename Tag, typename T>
auto createParentConfig()
{
   auto topLevelOptionHandler = std::make_unique<po::options_description>();

   topLevelOptionHandler->add_options()
      ("list-components",
       // po::value<std::string>()->notifier(&printComponents),
       po::value<std::string>()->implicit_value("")->zero_tokens()->notifier(&printComponents),
       "List the statistic components\n");
   // po::value<std::string>()->composing()->notifier(printComponents),
   return topLevelOptionHandler;
}


inline std::string getComponentName(std::string cmd_line)
{
   std::string component_str;
   auto component_extractor = po::options_description{};
   component_extractor.add_options()
      ("component",
       po::value<std::string>()->notifier([&](std::string comp){ component_str = comp;}));
   po::variables_map vm;
   po::store(po::command_line_parser(tokenize(cmd_line))
         .options(component_extractor)
         .run(),
         vm);
   po::notify(vm);
   return component_str;
}

inline auto getHeadTail(std::string s, char delim)
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


inline void processCmdLineOptions(po::options_description& option_desc, int argc, char** argv)
{
   using std::cout;
   using std::endl;
   try
   {
      po::variables_map vm;
      po::options_description desc("Command Line Options", TERM_NUM_COLUMNS);
      desc.add(option_desc);
      po::store(po::command_line_parser(argc, argv)
            .options(desc)
            .run(),
            vm);
      po::notify(vm);

   }
   catch (std::exception& e)
   {
      std::cout << e.what() << "\n";
      std::exit(1);
   }
}

template <typename TagNode, typename Stat,
         typename std::enable_if<
         !detail::is_parent<TagNode>::value
         >::type* = nullptr
   >
void parse(Stat& stat, std::string& component_name, std::string& user_cmds)
{
   std::cout << "IN PARSE LEAF: " << TagNode::name
      << ", component name = " << component_name << std::endl;
   if(component_name == TagNode::name)
   {
      std::cout << "Matches leaf!" << std::endl;
   }
   // processCommands<TagNode, false>(user_cmds);
}

template <typename TagNode, typename Stat,
         typename std::enable_if<
         detail::is_parent<TagNode>::value
         >::type* = nullptr
   >
void parse(Stat& stat, std::string& component_name, std::string& user_cmds)
{
   std::cout << "IN PARSE PARENT: " << TagNode::name << std::endl;
   if(component_name.empty())
   {
      // processCommands<TagNode, true>(user_cmds);
      return;
   }

   auto component_head_tail = getHeadTail(component_name,'-');
   auto& head_c = std::get<0>(component_head_tail);
   auto& tail_c = std::get<1>(component_head_tail);
   std::cout << "Head = " << head_c << ", tail = " << tail_c << std::endl;
   if(head_c != TagNode::name)
   {
      std::cout << "NO Matches!" << std::endl;
      return;
   }
   std::cout << "Matches!" << std::endl;
   if(tail_c.empty())
   {
      // processCommands<TagNode, true>(user_cmds);
      return;
   }
   using Children = typename TagNode::child_list;
   boost::fusion::for_each(Children{},
         [&](auto ctag_node)
         {
          parse<decltype(ctag_node)>(stat, tail_c, user_cmds);
         });
}


}

