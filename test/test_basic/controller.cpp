#include "common.h"

using namespace stat_log;

int main(int argc, char** argv)
{
   initializeStatistics<false == IsOperational>();
   auto& stat = getStatLogSingleton<ControlStat>();

   stat.parseUserCommands(argc, argv);
   stat.showOutput();
   return 0;
}
