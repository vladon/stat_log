#include "common.h"
constexpr bool IsOperational = false;

using namespace stat_log;

int main(int argc, char** argv)
{
   initializeStatistics<IsOperational>();
   auto& macSisStat = getStatSingleton<ControlStatMacSis>();
   auto& hwIntfStat = getStatSingleton<ControlStatHwIntf>();

   //TODO: Combine stat processing
   macSisStat.parseUserCommands(argc, argv);
   return 0;
}
