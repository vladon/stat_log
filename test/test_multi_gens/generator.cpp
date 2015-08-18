#include "common.h"

constexpr bool IsOperational = true;

using namespace stat_log;

int main(int argc, char** argv)
{
   initializeStatistics<IsOperational>();
   auto& macSisStat = getStatSingleton<OpStatMacSis>();
   auto& hwIntfStat = getStatSingleton<OpStatHwIntf>();
   macSisStat.writeStat<SIS::MAC_PKTS_DOWN_TAG>(88);
   //TODO: write stats
   return 0;
}
