#pragma once

#include "stat_log/parsers/parser_common.h"
#include "stat_log/parsers/leaf_parser.h"
#include "stat_log/util/utils.h"

#include <boost/lexical_cast.hpp>

namespace stat_log
{


//DoCmd specialization for non-leaf nodes
template<typename Stat>
struct DoCmd<Stat, true>
{
   template <typename TagNode>
   static void Go(Stat& stat, StatCmd cmd, boost::any& cmd_arg)
   {
      using Children = typename TagNode::child_list;
      detail::indent(TagNode::depth);
      using Parent = typename TagNode::parent;
      using Tag = typename TagNode::tag;
      std::cout << TagNode::name << std::endl;
      if(cmd == StatCmd::LOG_LEVEL)
      {
         std::cout << "LOG_LEVEL cmd for node " << TagNode::name << std::endl;
         stat.template sendCommand<Tag>(cmd, cmd_arg);
      }
      for_each(Children{}, [&](auto tag_node)
      {
         using ChildTagNode = decltype(tag_node);
         using IsParent = typename detail::is_parent<ChildTagNode>;
         using TheDoCmd = DoCmd<Stat, IsParent::value>;
         TheDoCmd::template Go<ChildTagNode>(stat, cmd, cmd_arg);
      });
   }
};


template <typename TagNode, typename Stat>
typename std::enable_if_t<detail::is_parent<TagNode>::value>
processCommands(Stat& stat, const std::string& user_cmds)
{
   namespace po = boost::program_options;
   using namespace boost::fusion;
   std::cout << "PROCESS CMDS, node = " << TagNode::name << std::endl;

   auto desc = detail::getParentOptions();
   po::variables_map vm;
   po::store(po::command_line_parser(tokenize(user_cmds))
         .options(desc)
         .run(),
         vm);

   if(vm.count("help"))
   {
      std::cout << desc << std::endl;
   }

   auto cmd = StatCmd::NO_CMD;
   boost::any cmd_arg;
   using Children = typename TagNode::child_list;

   if(vm["list-stats"].as<bool>())
   {
      cmd = StatCmd::PRINT_STAT_TYPE;
      detail::indent(TagNode::depth);
      using Parent = typename TagNode::parent;
      std::cout << TagNode::name << std::endl;
   }
   if(vm.count("log-level"))
   {
      std::cout << "log-level, tagNode = " << TagNode::name << std::endl;
      //Args: <LoggerIdx> [<LogLevel>]
      //No LogLevel arg will print the current log level
      auto arg_vec = vm["log-level"].as<std::vector<std::string>>();
      int logger_idx = 0;
      if(arg_vec.size() == 0 || arg_vec.size() > 2)
      {
         std::cerr << "Invalid number for arguments!\n";
         return;
      }
      try
      {
         logger_idx = boost::lexical_cast<int>(arg_vec[0]);
      }
      catch(boost::bad_lexical_cast&)
      {
         std::cerr << "Invalid logger idx!\n";
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
      DoCmd<Stat, true>::template Go<TagNode>(stat, cmd, cmd_arg);
   }
   if(vm.count("output-log"))
   {
      //TODO: extract the user params
      stat.outputLog(0, "output.txt");
   }
   if(cmd != StatCmd::NO_CMD)
   {
      for_each(Children{}, [&](auto tag_node)
         {
            using ChildTagNode = decltype(tag_node);
            using IsParent = typename detail::is_parent<ChildTagNode>;
            using TheDoCmd = DoCmd<Stat, IsParent::value>;
            TheDoCmd::template Go<ChildTagNode>(stat, cmd, cmd_arg);
         });
   }
}

template <typename TagNode, typename Stat,
         typename std::enable_if<
         detail::is_parent<TagNode>::value
         >::type* = nullptr
   >
void parse(Stat& stat, std::string& component_name, std::string& user_cmds)
{
   if(component_name.empty())
   {
      processCommands<TagNode>(stat, user_cmds);
      return;
   }

   auto child_component_name = component_name;
   if(TagNode::depth > 0)
   {
      auto component_head_tail = getHeadTail(component_name,'-');
      auto& head_c = std::get<0>(component_head_tail);
      auto& tail_c = std::get<1>(component_head_tail);
      if(head_c != TagNode::name)
      {
         return;
      }
      if(tail_c.empty())
      {
         processCommands<TagNode>(stat, user_cmds);
         return;
      }
      child_component_name = tail_c;
   }
   using Children = typename TagNode::child_list;
   boost::fusion::for_each(Children{},
      [&](auto ctag_node)
      {
          parse<decltype(ctag_node)>(stat, child_component_name, user_cmds);
      });
}

}

