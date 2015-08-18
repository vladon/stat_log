#pragma once
#include "stat_log/fusion_includes.h"
#include "stat_log/util/stat_log_impl.h"
#include "stat_log/parsers/parent_parser.h"
#include "stat_log/stats/stats_common.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/any.hpp>

#include <thread>
#include <chrono>

namespace stat_log
{

template <typename UserStatH, typename Logger>
struct LogStatOperational :
   detail::LogStatBase<
      UserStatH, true, Logger, LogStatOperational<UserStatH, Logger>
   >
{
   //TODO: add
   // logDebug, logInfo ...
   template <typename StatTag, typename... Args>
   void writeStat(Args... args)
   {
      using namespace boost::fusion;
      auto statHdlView = detail::getStatHandleView<StatTag>(this->theStats);
      static_assert(result_of::size<decltype(statHdlView)>::value == 1,
            "Too many matching tags in writeStat!");
      auto& stat_hdl = deref(begin(statHdlView));
      using StatHdlType = std::remove_reference_t<decltype(stat_hdl)>;
      static_assert(
            StatHdlType::IsParent == false,
            "Require a leaf node for writeStat!");
      stat_hdl.theProxy.write(args...);
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

   std::thread serialization_thread;
   bool serializer_running = false;
};

template <typename UserStatH, typename Logger>
struct LogStatControl :
   detail::LogStatBase<
      UserStatH, false, Logger, LogStatControl<UserStatH, Logger>
   >
{
   using BaseClass = detail::LogStatBase<
      UserStatH, false, Logger, LogStatControl<UserStatH, Logger>>;
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
   void sendStatCommand(StatCmd cmd, boost::any& cmd_arg)
   {
      using namespace boost::fusion;
      auto statHdlView = detail::getStatHandleView<StatTag>(this->theStats);
      static_assert(result_of::size<decltype(statHdlView)>::value == 1,
            "Too many matching tags in sendStatCommand!");
      auto& stat_hdl = deref(begin(statHdlView));
      using StatHdlType = std::remove_reference_t<decltype(stat_hdl)>;
      static_assert(//decltype(stat_hdl)::IsParent == false,
            StatHdlType::IsParent == false,
            "Require a leaf node for sendStatCommand!");
      stat_hdl.theProxy.doStatCommand(cmd, cmd_arg);
   }

   void doInit()
   {}

   void doStop()
   {}
};

template <typename Stat>
auto& getStatSingleton()
{
   static Stat theStat;
   return theStat;
}

}

