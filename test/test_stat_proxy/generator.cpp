#include "stat_proxy_interface.h"
#include "common.h"
#include <thread>
#include <chrono>


constexpr bool IsOperational = true;
template <>
void initializeStatistics<IsOperational>()
{
   //TODO: pass the logger to init()
   stat_log::getStatSingleton<OpStat>().init(STAT_LOG_SHM_NAME);
}


template <typename Tag, typename ... Args>
void writeStatProxy(Args... args)
{
   stat_log::getStatSingleton<OpStat>().writeStat<Tag>(args...);
}

//EXPLICIT TEMPLATE INSTANTIATIONS
template void writeStatProxy<MAC::IP_PKTS_UP_TAG>(int val);
template void writeStatProxy<SIS::MAC_PKTS_DOWN_TAG>(int proto_idx, int prio_idx, int val);
template void writeStatProxy<HW_INTERFACE::MISC_FPGA_FAULT_TAG>(int val);

//TODO: this will be super annoying for the user to have to define the
// stat hierarchy AND explicitly instantiate each of them ...
// Think  of a way to automate this.

int main(int argc, char** argv)
{
   initializeStatistics<IsOperational>();
   genStats_MAC();
   genStats_SIS();
   genStats_HW_INTF();

   std::this_thread::sleep_for(std::chrono::seconds{10});
   stat_log::getStatSingleton<OpStat>().stop();
   return 0;
}
