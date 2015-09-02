#pragma once
#include <stat_log/util/utils.h>
#include <stat_log/parsers/parser_common.h>


namespace stat_log
{

//DoCmd specialization for leaf nodes.
template<typename Stat>
struct DoCmd<Stat, false>
{
   template <typename TagNode>
   static void Go(Stat& stat, StatCmd cmd, boost::any& cmd_arg)
   {
      // using Parent = typename TagNode::parent;
      using Tag = typename TagNode::tag;

      if(printingRequired(cmd, false))
      {
         detail::indent(TagNode::depth);
         std::cout << TagNode::name << std::endl;
         detail::indent(TagNode::depth);
         std::cout << "\t";
      }
      stat.template sendCommand<Tag>(cmd, cmd_arg);
      if(printingRequired(cmd, false))
      {
         std::cout << "\n";
      }
   }
};

template <typename TagNode, typename Stat>
typename std::enable_if_t<!detail::is_parent<TagNode>::value>
processCommands(Stat& stat, const std::string& user_cmds)
{
   auto cmd = StatCmd::NO_CMD;
   boost::any cmd_arg;

   //TODO: process child options
   cmd = StatCmd::PRINT_STAT_TYPE;
   using TheDoCmd = DoCmd<Stat, false>;
   TheDoCmd::template Go<TagNode>(stat, cmd, cmd_arg);
}

template <typename TagNode, typename Stat,
         typename std::enable_if<
         !detail::is_parent<TagNode>::value
         >::type* = nullptr
   >
void parse(Stat& stat, std::string& component_name, std::string& user_cmds)
{
   if(component_name == TagNode::name)
      processCommands<TagNode>(stat, user_cmds);
}

}
