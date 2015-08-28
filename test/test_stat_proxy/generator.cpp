#include "stat_proxy_interface.h"
#include "stat_log/util/compile_proxy.h"
#include "common.h"
#include <thread>
#include <chrono>

constexpr bool IsOperational = true;
template <>
void initializeStatistics<IsOperational>()
{
   auto logger = std::make_shared<LoggerGenerator>(STAT_LOG_LOGGER_NAME,
         STAT_LOG_LOGGER_SIZE_BYTES);
   auto& theStats = stat_log::getStatSingleton<OpStat>();
   theStats.init(STAT_LOG_SHM_NAME);
   theStats.addLogger(logger);
}


namespace stat_log
{
   template <typename Tag, typename ... Args>
   void writeStat(Args... args)
   {
      stat_log::getStatSingleton<OpStat>().writeStat<Tag>(args...);
   }

   //EXPLICIT TEMPLATE INSTANTIATIONS
   template void writeStat<MAC::IP_PKTS_UP_TAG>(int val);
   template void writeStat<SIS::MAC_PKTS_DOWN_TAG>(int proto_idx, int prio_idx, int val);
   template void writeStat<HW_INTERFACE::MISC_FPGA_FAULT_TAG>(int val);
}

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
