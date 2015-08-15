#pragma once
#include "util/utils.h"
#include "parsers/parser_common.h"


namespace stat_log
{

template <typename TagNode, typename Stat>
typename std::enable_if_t<!detail::is_parent<TagNode>::value>
processCommands(Stat& stat, const std::string& user_cmds)
{
   std::cout << "PROCESS CMDS child" << std::endl;
}

template <typename TagNode, typename Stat,
         typename std::enable_if<
         !detail::is_parent<TagNode>::value
         >::type* = nullptr
   >
void parse(Stat& stat, std::string& component_name, std::string& user_cmds)
{
   if(component_name == TagNode::name)
   {
      processCommands<TagNode>(stat, user_cmds);
   }
}

template<typename Stat>
struct DoCmd<Stat, false>
{
   template <typename TagNode>
   static void Go(Stat& stat, StatCmd cmd, boost::any& cmd_arg)
   {
      detail::indent(TagNode::depth);
      using Parent = typename TagNode::parent;
      using Tag = typename TagNode::tag;
      std::cout << "Type is = " << TagNode::name
         << ", parent is " << Parent::name << " ";
         // << ", value is " << getValue<Tag>(stat)
      stat.template sendStatCommand<Tag>(cmd, cmd_arg);
      std::cout  << ", depth is " << TagNode::depth
         // << ", addr is " << std::hex << &getValue<Tag>(theOpStats)
         << std::endl;
   }
};
}
