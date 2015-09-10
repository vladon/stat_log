#pragma once

#include <stat_log/parsers/parser_common.h>
#include <stat_log/parsers/leaf_parser.h>
#include <stat_log/util/utils.h>

#include <boost/lexical_cast.hpp>

namespace stat_log
{

//DoCmd specialization for non-leaf nodes
template<typename StatLogControl>
struct DoCmd<StatLogControl, true>
{
   template <typename TagNode>
   static void Go(StatLogControl& stat_log_control, StatCmd cmd, boost::any& cmd_arg)
   {
      using Children = typename TagNode::child_list;
      stat_log_control.template sendCommand<TagNode>(cmd, cmd_arg);

      for_each(Children{}, [&](auto tag_node)
      {
         using ChildTagNode = decltype(tag_node);
         constexpr bool IsParent = is_parent<ChildTagNode>::value;
         using TheDoCmd = DoCmd<StatLogControl, IsParent>;
         TheDoCmd::template Go<ChildTagNode>(stat_log_control, cmd, cmd_arg);
      });
   }
};

template <typename TagNode, typename StatLogControl,
         typename std::enable_if<
            is_parent<TagNode>::value
         >::type* = nullptr
   >
void parse(StatLogControl& stat_log_control, std::string& component_name, std::string& user_cmds)
{
   if(component_name.empty())
   {
      processCommands<TagNode>(stat_log_control, user_cmds);
      return;
   }

   auto child_component_name = component_name;
   auto component_head_tail = getHeadTail(component_name,'.');
   auto& head_c = std::get<0>(component_head_tail);
   auto& tail_c = std::get<1>(component_head_tail);

   if(head_c != TagNode::name)
   {
      return;
   }
   if(tail_c.empty())
   {
      processCommands<TagNode>(stat_log_control, user_cmds);
      return;
   }
   child_component_name = tail_c;
   using Children = typename TagNode::child_list;
   boost::fusion::for_each(Children{}, [&](auto ctag_node)
   {
       parse<decltype(ctag_node)>(stat_log_control, child_component_name, user_cmds);
   });
}

}

