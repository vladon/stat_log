#include "common.h"

using namespace stat_log;

int main(int argc, char** argv)
{
   initializeStatistics<false == IsOperational>();
   auto& stat = getStatSingleton<ControlStat>();

   stat.parseUserCommands(argc, argv);
   return 0;
}
