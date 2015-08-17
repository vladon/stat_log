#include "common.h"
#include "hw_intf.h"

LoggerType& getLoggerRef()
{
   static LoggerType theLogger;
   return theLogger;
}

template <>
void initializeStatistics<true>()
{
   //TODO: pass the logger to init()
   stat_log::getStatSingleton<OpStatHwIntf>().init("stat_log_hw_intf");
   stat_log::getStatSingleton<OpStatMacSis>().init("stat_log_mac_sis");
}

template <>
void initializeStatistics<false>()
{
   //TODO: pass the logger to init()
   stat_log::getStatSingleton<ControlStatHwIntf>().init("stat_log_hw_intf");
   stat_log::getStatSingleton<ControlStatMacSis>().init("stat_log_mac_sis");
}
