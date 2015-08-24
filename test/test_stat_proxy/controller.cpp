#include "common.h"

constexpr bool IsOperational = false;
template <>
void initializeStatistics<IsOperational>()
{
   auto logger = std::make_shared<LoggerRetriever>(STAT_LOG_LOGGER_NAME,
         STAT_LOG_LOGGER_SIZE_BYTES);
   auto& theStat = stat_log::getStatSingleton<ControlStat>();
   theStat.init(STAT_LOG_SHM_NAME);
   theStat.addLogger(logger);
}

using namespace stat_log;

int main(int argc, char** argv)
{
   initializeStatistics<IsOperational>();
   auto& controlStat = getStatSingleton<ControlStat>();

   controlStat.assignEnumerationNames<
      HW_INTERFACE::MISC_FPGA_FAULT_TAG>({"OVER_TEMP", "INVALID_COMMAND", "UNKNOWN_FAULT"});

   controlStat.assignEnumerationNames<
      SIS::MAC_PKTS_DOWN_TAG>({"L", "M", "H"});

   controlStat.assignDimensionNames<
      SIS::MAC_PKTS_DOWN_TAG>({"Priority", "Traffic Type"});

   controlStat.parseUserCommands(argc, argv);
   return 0;
}
