#pragma once
#include "stat_log/fusion_includes.h"
#include "stat_log/util/stat_log_impl.h"
#include "stat_log/parsers/parent_parser.h"
#include "stat_log/stats/stats_common.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/any.hpp>

namespace stat_log
{

template <typename UserStatH, typename Logger>
struct LogStatOperational : detail::LogStatBase<UserStatH, true, Logger>
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
};

template <typename UserStatH, typename Logger>
struct LogStatControl : detail::LogStatBase<UserStatH, false, Logger>
{
   using BaseClass = detail::LogStatBase<UserStatH, false, Logger>;
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
};

template <typename Stat>
auto& getStatSingleton()
{
   static Stat theStat;
   return theStat;
}

}

