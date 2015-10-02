//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

#pragma once

#include <stat_log/util/command.h>
#include <stat_log/util/utils.h>
#include <boost/fusion/include/for_each.hpp>
#include <type_traits>

namespace stat_log
{

namespace detail
{
   //DoCmd
   template<typename StatLogControl, bool IsParent>
   struct DoCmd
   {
      template <typename TagNode>
      static void Go(StatLogControl& stat_log_control, StatCmd cmd, boost::any& cmd_arg)
      {
         using Children = typename TagNode::child_list;
         stat_log_control.template sendCommand<TagNode>(cmd, cmd_arg);

         boost::fusion::for_each(Children{}, [&](auto tag_node)
         {
            using ChildTagNode = decltype(tag_node);
            constexpr bool ChildIsParent = is_parent<ChildTagNode>::value;
            using TheDoCmd = DoCmd<StatLogControl, ChildIsParent>;
            TheDoCmd::template Go<ChildTagNode>(stat_log_control, cmd, cmd_arg);
         });
      }
   };

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
   void processCommand(StatLogControl& stat_log_control, StatCmd cmd, boost::any& cmd_arg)
   {
      if(cmd != StatCmd::NO_CMD)
      {
         constexpr bool IsParentNode = is_parent<TagNode>::value;
         using TheDoCmd = DoCmd<StatLogControl, IsParentNode>;
         TheDoCmd::template Go<TagNode>(stat_log_control, cmd, cmd_arg);
      }
   }
}

//For leaf nodes
template <typename TagNode, typename StatLogControl,
         typename std::enable_if<
            !is_parent<TagNode>::value
         >::type* = nullptr
   >
void tagCommander(StatLogControl& stat_log_control,
      std::string& tag_name, StatCmd cmd, boost::any& cmd_arg)
{
   if(tag_name == TagNode::name || tag_name.empty())
      detail::processCommand<TagNode>(stat_log_control, cmd, cmd_arg);
}

//For non-leaf nodes
template <typename TagNode, typename StatLogControl,
         typename std::enable_if<
            is_parent<TagNode>::value
         >::type* = nullptr
   >
void tagCommander(StatLogControl& stat_log_control,
      std::string& tag_name, StatCmd cmd, boost::any& cmd_arg)
{
   if(tag_name.empty())
   {
      detail::processCommand<TagNode>(stat_log_control, cmd, cmd_arg);
      return;
   }

   auto child_tag_name = tag_name;
   auto tag_head_tail = getHeadTail(tag_name,'.');
   auto& head_c = std::get<0>(tag_head_tail);
   auto& tail_c = std::get<1>(tag_head_tail);

   if(head_c != TagNode::name)
   {
      return;
   }
   if(tail_c.empty())
   {
      detail::processCommand<TagNode>(stat_log_control, cmd, cmd_arg);
      return;
   }
   child_tag_name = tail_c;
   using Children = typename TagNode::child_list;
   boost::fusion::for_each(Children{}, [&](auto ctag_node)
   {
       tagCommander<decltype(ctag_node)>(stat_log_control,
             child_tag_name, cmd, cmd_arg);
   });
}


}

