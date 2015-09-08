#pragma once
#include <stat_log/util/stat_log_impl.h>
#include <stat_log/parsers/parent_parser.h>
#include <stat_log/stats/stats_common.h>

#include <boost/algorithm/string/join.hpp>
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
   using BaseClass = typename
      detail::LogStatBase<StatTagTree, LogTagTree, true,
         stat_log::LogStatOperational>; //Note the fully qualified name is required
                                        // due a bug in the clang compiler
   template <typename LogTag>
   LogGenProxy getLog(std::size_t log_idx, int log_level)
   {
      assert(log_idx <= loggers.size());
      auto& log_hdl = detail::getHandle<LogTag>(this->theLogs);
      using LogHdlType = std::remove_reference_t<decltype(log_hdl)>;

      auto cur_log_level = log_hdl.getLevel(log_idx);
      return LogGenProxy {
         *loggers[log_idx],
         log_level >= cur_log_level,
         LogHdlType::tag_node::name,
         LogLevelNames[log_level]
      };
   }

   template <typename LogTag>
   LogGenProxy getDebugLog(std::size_t log_idx = 0)
   {
      return getLog<LogTag>(log_idx, 0);
   }

   template <typename LogTag>
   LogGenProxy getInfoLog(std::size_t log_idx = 0)
   {
      return getLog<LogTag>(log_idx, 1);
   }

   template <typename LogTag>
   LogGenProxy getAlertLog(std::size_t log_idx = 0)
   {
      return getLog<LogTag>(log_idx, 2);
   }

   template <typename LogTag>
   LogGenProxy getErrorLog(std::size_t log_idx = 0)
   {
      return getLog<LogTag>(log_idx, 3);
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
   using BaseClass = typename
      detail::LogStatBase<StatTagTree, LogTagTree, false,
         stat_log::LogStatControl>; //Note the fully qualified name is required
                                    // due a bug in the clang compiler
   using TopNode = typename BaseClass::TopNode;
   using StatTagHierarchy = typename BaseClass::StatTagHierarchy;
   using LogTagHierarchy = typename BaseClass::LogTagHierarchy;

   void parseUserCommands(int argc, char** argv)
   {
      //First extract the Component hierarchy portion of
      // the user_input (if it exists)
      std::vector<std::string> user_strings;
      for(int i = 1; i < argc; ++i)
         user_strings.push_back(argv[i]);

      std::string user_cmd_line = boost::algorithm::join(user_strings, " ");
      std::string component_str = getComponentName(user_cmd_line);
      parse<StatTagHierarchy>(*this, component_str, user_cmd_line);
      parse<LogTagHierarchy>(*this, component_str, user_cmd_line);
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
         StatTagNode::depth,
         true};
      detail::getHandle<typename StatTagNode::tag>(this->theStats)
         .doCommand(cmd, cmd_arg, tag_info);
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
         LogTagNode::depth,
         false};
      detail::getHandle<Tag>(this->theLogs)
         .doCommand(cmd, cmd_arg, tag_info);
   }

   void outputLog(int logger_idx, boost::any& log_args)
   {
      if(logger_idx >= (int)loggers.size())
      {
         std::cout << "Invalid log index = " << logger_idx;
         if(loggers.empty())
            std::cout << ". No configured loggers!\n";
         else
            std::cout << ". Valid indices = 0..."<< loggers.size() - 1 << std::endl;
         std::exit(1);
      }
      loggers[logger_idx]->getLog(log_args);
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

