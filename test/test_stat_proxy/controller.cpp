#include "common.h"

using namespace stat_log;

int main(int argc, char** argv)
{
   initializeStatistics<false == IsOperational>();
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
