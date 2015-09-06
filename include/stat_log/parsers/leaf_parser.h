#pragma once
#include <stat_log/util/utils.h>
#include <stat_log/parsers/parser_common.h>


namespace stat_log
{

//DoCmd specialization for leaf nodes.
template<typename StatLogControl>
struct DoCmd<StatLogControl, false>
{
   template <typename TagNode>
   static void Go(StatLogControl& stat_log_control, StatCmd cmd, boost::any& cmd_arg)
   {
      stat_log_control.template sendCommand<TagNode>(cmd, cmd_arg);
   }
};

template <typename TagNode, typename StatLogControl>
typename std::enable_if_t<!detail::is_parent<TagNode>::value>
processCommands(StatLogControl& stat_log_control, const std::string& user_cmds)
{
   auto cmd = StatCmd::NO_CMD;
   boost::any cmd_arg;
   processCommandsCommon<TagNode, false>(stat_log_control, user_cmds, cmd, cmd_arg);
   using TheDoCmd = DoCmd<StatLogControl, false>;
   setStartingDepth(TagNode::depth);
   TheDoCmd::template Go<TagNode>(stat_log_control, cmd, cmd_arg);
}

template <typename TagNode, typename StatLogControl,
         typename std::enable_if<
         !detail::is_parent<TagNode>::value
         >::type* = nullptr
   >
void parse(StatLogControl& stat_log_control, std::string& component_name, std::string& user_cmds)
{
   if(component_name == TagNode::name)
      processCommands<TagNode>(stat_log_control, user_cmds);
}

}
