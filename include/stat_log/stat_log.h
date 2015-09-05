#pragma once
#include <stat_log/util/stat_log_impl.h>
#include <stat_log/parsers/parent_parser.h>
#include <stat_log/stats/stats_common.h>

#include <boost/algorithm/string/join.hpp>
#include <boost/mpl/find_if.hpp>
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
   template <typename StatTag, typename Stats>
   auto& getStatHandle(Stats& theStats)
   {
      auto statHdlView = getStatHandleView<StatTag>(theStats);
      static_assert(boost::fusion::result_of::size<decltype(statHdlView)>::value == 1,
            "Require a SINGLE matching tag in getStatHandle!");
      auto& stat_hdl = boost::fusion::deref(boost::fusion::begin(statHdlView));
      return stat_hdl;
   }
}

template <typename UserStatH>
struct LogStatOperational :
   detail::LogStatBase<UserStatH, true, LogStatOperational<UserStatH>>
{
   using BaseClass = typename detail::LogStatBase<UserStatH, true, LogStatOperational<UserStatH>>;
   template <typename LogTag>
   LogGenProxy getLog(std::size_t log_idx, int log_level)
   {
      assert(log_idx <= loggers.size());
      auto& log_hdl = detail::getStatHandle<LogTag>(this->theStats);
      using LogHdlType = std::remove_reference_t<decltype(log_hdl)>;
      static_assert(
            LogHdlType::IsParent == true,
            "Require a parent_node for getLog!");

      auto cur_log_level = log_hdl.theProxy.getLevel(log_idx);
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
      detail::getStatHandle<StatTag>(this->theStats).theProxy.writeVal(args...);
   }

   //This bit of MPL-magic tests if ANY of the statistics in the hierarchy
   //require deferred serialization.
   static constexpr bool need_deferred_serialization =
     !std::is_same
        <
           typename boost::mpl::end<typename BaseClass::TheStats>::type,
           typename boost::mpl::find_if
           <
              typename BaseClass::TheStats,
              detail::tag_node_query<is_serialization_deferred,
                                     stat_tag_to_type>
           >::type
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

template <typename UserStatH>
struct LogStatControl :
   detail::LogStatBase<UserStatH, false, LogStatControl<UserStatH>>
{
   using BaseClass = detail::LogStatBase<
      UserStatH, false, LogStatControl<UserStatH>>;
   using TopNode = typename BaseClass::TopNode;
   using TagHierarchy = typename BaseClass::TagHierarchy;

   void parseUserCommands(int argc, char** argv)
   {
      //First extract the Component hierarchy portion of
      // the user_input (if it exists)
      std::vector<std::string> user_strings;
      for(int i = 1; i < argc; ++i)
         user_strings.push_back(argv[i]);

      std::string user_cmd_line = boost::algorithm::join(user_strings, " ");
      std::string component_str = getComponentName(user_cmd_line);
      parse<TagHierarchy>(*this, component_str, user_cmd_line);
   }

   template <typename StatTag>
   void sendCommand(StatCmd cmd, boost::any& cmd_arg)
   {
      detail::getStatHandle<StatTag>(this->theStats).theProxy.doCommand(cmd, cmd_arg);
   }

   template <typename StatTag>
   void assignEnumerationNames(const std::vector<std::string>& enumNames)
   {
      detail::getStatHandle<StatTag>(this->theStats)
         .theProxy.enumerationNames = enumNames;
   }

   template <typename StatTag>
   void assignDimensionNames(const std::vector<std::string>& dimNames)
   {
      detail::getStatHandle<StatTag>(this->theStats)
         .theProxy.dimensionNames = dimNames;
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

template <typename Stat>
auto& getStatSingleton()
{
   static Stat theStat;
   return theStat;
}


template<typename Tag, typename Stat>
struct TagBelongsToStat
{
   using TheStats = typename std::remove_const_t<typename Stat::TheStats>;
   static_assert(boost::mpl::is_sequence<TheStats>::value,
         "Stat::TheStats must be a sequence!");
   using Iter = typename boost::fusion::result_of::find_if<
         TheStats,
         detail::matches_tag<Tag>
      >::type;
   static constexpr bool value = !std::is_same<
         Iter,
         typename boost::fusion::result_of::end<TheStats>::type
      >::value;
};

}

