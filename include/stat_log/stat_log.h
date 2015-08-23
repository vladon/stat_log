#pragma once
#include "stat_log/util/stat_log_impl.h"
#include "stat_log/parsers/parent_parser.h"
#include "stat_log/stats/stats_common.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/any.hpp>

#include <thread>
#include <chrono>
#include <memory>
#include <vector>
#include <fstream>

namespace stat_log
{

namespace detail
{
   template <typename StatTag, typename Stats>
   auto& getStatHandle(Stats& theStats)
   {
      auto statHdlView = getStatHandleView<StatTag>(theStats);
      static_assert(boost::fusion::result_of::size<decltype(statHdlView)>::value == 1,
            "Too many matching tags in getStatHandle!");
      auto& stat_hdl = boost::fusion::deref(boost::fusion::begin(statHdlView));
      return stat_hdl;
   }
}

template <typename UserStatH>
struct LogStatOperational :
   detail::LogStatBase<UserStatH, true, LogStatOperational<UserStatH>
   >
{
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
      std::cout << "Current log level is " << cur_log_level << std::endl;
      return LogGenProxy {
         *loggers[log_idx],
         log_level >= cur_log_level,
         "TEST_TAG_NAME", //TODO: need to map the LogTag to a name
         "TEST_LOG_LEVEL"};
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
      detail::getStatHandle<StatTag>(this->theStats).theProxy.write(args...);
   }

   //This method will be called by the base class once it is done with
   // its init().
   //TODO: I would really like this method to be private because
   // the user should NOT call it directly.  The problem is
   // i still need the base class to call it...
   //TODO: The creation of this thread should either be a policy
   // OR we should do a compile-time check for any stats requiring
   // deferred serialization, and if there are any, THEN start
   // the thread.
   void doInit()
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
   detail::LogStatBase<
      UserStatH, false, LogStatControl<UserStatH>
   >
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
      std::cout << "User cmd line = " << user_cmd_line << std::endl;
      std::string component_str = getComponentName(user_cmd_line);
      std::cout << "COMPONENT = " << component_str << std::endl;
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

   void outputLog(int logger_idx, const std::string& output_filename)
   {
      assert(logger_idx <= loggers.size());
      std::fstream output{output_filename, std::ios::out};
      //TODO: assign boolean flags as appropriate
      loggers[logger_idx]->getLog(std::move(output),
            true, //show_tag
            true, //show_time_stamp
            true  //show_log_level
            );
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

}

