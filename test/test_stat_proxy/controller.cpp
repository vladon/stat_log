#include "common.h"
constexpr bool IsOperational = false;

using namespace stat_log;

int main(int argc, char** argv)
{
   initializeStatistics<IsOperational>();
   auto& controlStat = getStatSingleton<ControlStat>();
   controlStat.parseUserCommands(argc, argv);
   return 0;
}
