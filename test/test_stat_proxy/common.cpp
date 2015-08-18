#include "common.h"

LoggerType& getLoggerRef()
{
   static LoggerType theLogger;
   return theLogger;
}

template <>
void initializeStatistics<true>()
{
   //TODO: pass the logger to init()
   stat_log::getStatSingleton<OpStat>().init("stat_log");
}

template <>
void initializeStatistics<false>()
{
   //TODO: pass the logger to init()
   stat_log::getStatSingleton<ControlStat>().init("stat_log");
}

template <typename Tag, typename ... Args>
void writeStatProxy(Args... args)
{
   stat_log::getStatSingleton<OpStat>().writeStat<Tag>(args...);
}

//EXPLICIT TEMPLATE INSTANTIATIONS
template void writeStatProxy<MAC::IP_PKTS_UP_TAG>(int val);
template void writeStatProxy<SIS::MAC_PKTS_DOWN_TAG>(int val);
template void writeStatProxy<HW_INTERFACE::MISC_FPGA_FAULT_TAG>(int val);

//TODO: this will be super annoying for the user to have to define the
// stat hierarchy AND explicitly instantiate each of them ...
// Think of a way to automate this.





