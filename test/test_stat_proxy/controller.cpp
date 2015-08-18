#include "common.h"
constexpr bool IsOperational = false;
template <>
void initializeStatistics<IsOperational>()
{
   //TODO: pass the logger to init()
   stat_log::getStatSingleton<ControlStat>().init(STAT_LOG_SHM_NAME);
}

using namespace stat_log;

int main(int argc, char** argv)
{
   initializeStatistics<IsOperational>();
   auto& controlStat = getStatSingleton<ControlStat>();
   controlStat.parseUserCommands(argc, argv);
   return 0;
}
