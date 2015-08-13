#pragma once

#include "parsers/parser_common.h"
#include "parsers/leaf_parser.h"


namespace stat_log
{

namespace detail
{
   namespace po = boost::program_options;

   auto getParentOptions()
   {
      po::options_description desc("Options", TERM_NUM_COLUMNS);
      desc.add_options()
         ("help", po::value<std::string>()->implicit_value(""),
          "Show help memu")
         ("component", po::value<std::string>()->default_value(""),
          "TODO")
         ("list-stats", po::bool_switch()->default_value(false),
          "List all statistics");
      return std::move(desc);
   }
}

inline std::string getComponentName(std::string cmd_line)
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
   std::cout << "component count = " << vm.count("component") << std::endl;
   return vm["component"].as<std::string>();
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

#if 1
template <typename Stat>
struct StatCommander;

template<typename Stat>
struct DoCmd<Stat, true>
{
   template <typename TagNode>
   static void Go(Stat& stat, StatCmd cmd, boost::any& cmd_arg)
   {
      using Children = typename TagNode::child_list;
      for_each(Children{}, [&](auto tag_node)
         {
            using ChildTagNode = decltype(tag_node);
            using IsParent = typename detail::is_parent<ChildTagNode>;
            using TheDoCmd = DoCmd<Stat, IsParent::value>;
            TheDoCmd::template Go<ChildTagNode>(stat, cmd, cmd_arg);
         });
   }
};


#endif
template <typename TagNode, typename Stat>
typename std::enable_if_t<detail::is_parent<TagNode>::value>
processCommands(Stat& stat, const std::string& user_cmds)
{
   namespace po = boost::program_options;
   using namespace boost::fusion;
   std::cout << "PROCESS CMDS parent" << std::endl;

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
      indent(TagNode::depth);
      using Parent = typename TagNode::parent;
      std::cout << "(PARENT) Type is = " << TagNode::name
            << ", parent is " << Parent::name
            << ", depth is " << TagNode::depth
            << std::endl;
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

