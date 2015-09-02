#include "common.h"

using namespace stat_log;

int main(int argc, char** argv)
{
   initializeStatistics<false == IsOperational>();
   auto& macSisStat = getStatSingleton<ControlStatMacSis>();
   auto& hwIntfStat = getStatSingleton<ControlStatHwIntf>();

   //TODO: is this a good way to combine the control
   // functionality of multiple stat controllers?
   macSisStat.parseUserCommands(argc, argv);
   hwIntfStat.parseUserCommands(argc, argv);
   return 0;
}
