//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

#pragma once
#include <stat_log/util/stat_log_impl.h>
#include <stat_log/util/tag_commander.h>
#include <stat_log/stats/stats_common.h>
#include <stat_log/loggers/logger_common.h>
#include <stat_log/util/printer.h>
#include <stat_log/util/utils.h>

#include <boost/any.hpp>

#include <thread>
#include <chrono>
#include <memory>
#include <vector>
#include <type_traits>

namespace stat_log
{

namespace detail
{
   template <typename Tag, typename T>
   auto& getHandle(T& type_node_list)
   {
      auto view = getView<Tag>(type_node_list);
      static_assert(boost::fusion::result_of::size<decltype(view)>::value == 1,
            "Require a SINGLE matching tag in getHandle!");
      auto& stat_hdl = boost::fusion::deref(boost::fusion::begin(view));
      return stat_hdl;
   }
}

template <typename StatTagTree, typename LogTagTree>
struct LogStatOperational :
   detail::LogStatBase<StatTagTree, LogTagTree, true, LogStatOperational>
{
   using BaseClass =
      detail::LogStatBase<StatTagTree, LogTagTree, true,
         stat_log::LogStatOperational>; //Note the fully qualified name is required
                                        // due a bug in the clang compiler
   template <typename LogTag>
   LogGenProxy getLog(LogLevel log_level = INFO, std::size_t log_idx = 0)
   {
      assert(log_idx <= loggers.size());
      auto& log_hdl = detail::getHandle<LogTag>(this->theLogs);
      using LogHdlType = std::remove_reference_t<decltype(log_hdl)>;

      auto cur_log_level = log_hdl.getLevel(log_idx);
      return LogGenProxy {
         *loggers[log_idx],
         static_cast<decltype(cur_log_level)>(log_level) >= cur_log_level,
         LogHdlType::tag_node::name,
         LogLevelNames[log_level]
      };
   }

   template <typename StatTag, typename... Args>
   void writeStat(Args... args)
   {
      detail::getHandle<StatTag>(this->theStats).writeVal(args...);
   }

   //This bit of MPL-magic tests if ANY of the statistics in the hierarchy
   //require deferred serialization.
   static constexpr bool need_deferred_serialization =
      detail::is_found_in_list<
         typename BaseClass::TheStats,
         detail::tag_node_query<
            is_serialization_deferred,
            stat_tag_to_type
         >
      >::value;


   //This method will be called by the base class once it is done with
   // its init().
   //TODO: I would really like this method to be private because
   // the user should NOT call it directly.  The problem is
   // i still need the base class to call it...
   void doInit()
   {
      //Only kick off a deferred serialization thread if you need to
      // e.g., if there is at least one stat requiring it.
      if(need_deferred_serialization)
      {
         serialization_thread = std::thread([this]()
         {
            serializer_running = true;
            while(serializer_running)
            {
               for_each(this->theStats, [](auto &stat)
               {
                  stat.doSerialize();
               });
               std::this_thread::sleep_for(std::chrono::seconds{1});
            }
         });
      }
   }

   void doStop()
   {
      if(serializer_running == false)
         return;
      serializer_running = false;
      serialization_thread.join();
   }

   int addLogger(std::shared_ptr<LoggerGenerator> logger)
   {
      loggers.push_back(logger);
      return loggers.size() - 1;
   }

   std::vector<std::shared_ptr<LoggerGenerator>> loggers;

   std::thread serialization_thread;
   bool serializer_running = false;
};

template <typename StatTagTree, typename LogTagTree>
struct LogStatControl :
   detail::LogStatBase<StatTagTree, LogTagTree, false, LogStatControl>
{
   using BaseClass =
      detail::LogStatBase<StatTagTree, LogTagTree, false,
         stat_log::LogStatControl>; //Note the fully qualified name is required
                                    // due a bug in the clang compiler
   using TopNode = typename BaseClass::TopNode;
   using StatTagHierarchy = typename BaseClass::StatTagHierarchy;
   using LogTagHierarchy = typename BaseClass::LogTagHierarchy;

   void parseUserCommands(int argc, char** argv)
   {
      std::vector<TagDisplayDesc> tag_disp_descs;
      StatCmd cmd = StatCmd::NO_CMD;
      boost::any cmd_arg;

      parseCommandLineArgs(argc, argv, tag_disp_descs, cmd, cmd_arg);
      printer.setCommand(cmd);

      for(auto& tag_desc : tag_disp_descs)
      {
         printer.setPrintOptions(tag_desc.print_options);
         if(isStatisticCommand(cmd))
         {
            tagCommander<StatTagHierarchy>(*this, tag_desc.tag_hname, cmd, cmd_arg);
         }
         if(isLogCommand(cmd))
         {
            if(cmd == StatCmd::DUMP_LOG)
               this->outputLog(cmd_arg);
            else
               tagCommander<LogTagHierarchy>(*this, tag_desc.tag_hname, cmd, cmd_arg);
         }
      }
   }

   void showOutput()
   {
      printer.showOutput();
   }

   template <typename StatTagNode>
   std::enable_if_t<
      detail::is_found_in_list
      <
         typename BaseClass::TheStats,
         detail::matches_tag<typename StatTagNode::tag>
      >::value
   >
   setStatDisplayArgs(StatDisplayOptions& stat_display_options)
   {
      using Tag = typename StatTagNode::tag;
      printer.setStatDisplayArgs(std::type_index(typeid(Tag)), stat_display_options);
   }

   template <typename StatTagNode>
   std::enable_if_t<
      detail::is_found_in_list
      <
         typename BaseClass::TheStats,
         detail::matches_tag<typename StatTagNode::tag>
      >::value
   >
   sendCommand(StatCmd cmd, boost::any& cmd_arg)
   {
      using Tag = typename StatTagNode::tag;
      TagInfo tag_info{
         StatTagNode::name,
         std::type_index(typeid(Tag)),
         StatTagNode::depth};

      StatCmdOutput stat_output;
      detail::getHandle<Tag>(this->theStats).doCommand(cmd, cmd_arg, stat_output);
      printer.addStatOutput(tag_info, std::move(stat_output));
   }

   template <typename LogTagNode>
   std::enable_if_t<
      detail::is_found_in_list
      <
         typename BaseClass::TheLogs,
         detail::matches_tag<typename LogTagNode::tag>
      >::value
   >
   sendCommand(StatCmd cmd, boost::any& cmd_arg)
   {
      using Tag = typename LogTagNode::tag;
      TagInfo tag_info{
         LogTagNode::name,
         std::type_index(typeid(Tag)),
         LogTagNode::depth};

      std::string log_output;
      detail::getHandle<Tag>(this->theLogs).doCommand(cmd, cmd_arg, log_output);
      printer.addLogOutput(tag_info, std::move(log_output));
   }

   void outputLog(boost::any& log_args)
   {
      auto log_params = boost::any_cast<LogOutputCommand>(log_args);
      const auto logger_idx = log_params.logger_idx;
      if(logger_idx >= (int)loggers.size())
      {
         std::cout << "Invalid log index = " << logger_idx;
         if(loggers.empty())
            std::cout << ". No configured loggers!\n";
         else
            std::cout << ". Valid indices = 0..."<< loggers.size() - 1 << std::endl;
         std::exit(1);
      }
      loggers[logger_idx]->getLog(log_params);
      std::exit(0);
   }



   void doInit()
   {}

   void doStop()
   {}

   int addLogger(std::shared_ptr<LoggerRetriever> logger)
   {
      loggers.push_back(logger);
      return loggers.size() - 1;
   }
private:
   Printer printer;
   std::vector<std::shared_ptr<LoggerRetriever>> loggers;
};

template <typename StatLog>
auto& getStatLogSingleton()
{
   static StatLog theStatLog;
   return theStatLog;
}


template<typename Tag, typename StatLog>
struct TagBelongsToStat
{
   static constexpr bool value =
      detail::TagBelongsTo<Tag, typename StatLog::TheStats>::value;
};

template<typename Tag, typename StatLog>
struct TagBelongsToLog
{
   static constexpr bool value =
      detail::TagBelongsTo<Tag, typename StatLog::TheLogs>::value;
};
}

