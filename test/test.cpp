#include "stat_log/stat_log.h"
#include "stat_log/backends/shared_mem_backend.h"
#include "stat_log/stats/stats_common.h"
#include "stat_log/stats/simple_counter.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

using namespace boost::fusion;
using namespace stat_log;
/*********************************
 * Statistic definitions
 *********************************/
struct sndcf
{
   NAME = "SNDCF";
   MAKE_STAT_TAGS( (IpDwn) (IpUp) )
};

struct sis_adapt
{
   NAME = "SIS_ADAPT";
   struct BlahTag{
      NAME = "BLAH";
   };
   struct NodeStatsTag{
      NAME = "NODE_STATS";
      MAKE_STAT_TAGS( (RxCount) (CorrScore) (LinkInd))
   };
   using ChildTypes = vector<BlahTag, NodeStatsTag>;
};

struct etdrs_stats
{
   NAME = "TOP_LEVEL";
   using ChildTypes = vector<sndcf, sis_adapt>;
};
 /*********************************/

//TODO: threading vs no-threaded policy
//
namespace stat_log
{

#if 1
   template <typename Tag>
   struct stat_traits
   {
      using StatType = SimpleCounter<int>;
   };
#endif


#if 0

   template<>
      struct stat_trait<CorrScoreTag>
      {
         template <bool IsOperational>
            using stat_type = StatTable

      };
#endif

   //Can specify the per-tag wfstat handler
   // that will react to user inputs (clear, view stat, etc).
}

struct MyLogger {};

using TdrsOpStat = LogStatOperational<etdrs_stats, MyLogger>;
using TdrsControlStat = LogStatControl<etdrs_stats, MyLogger>;

int main(int argc, char** argv)
{
#if 0
   using TagHierarchy = TdrsOpStat::TagHierarchy;
   std::cout << "The Type = " << TypeId<TagHierarchy>{} << std::endl;
#endif
   auto& tdrsOpStat = getStatSingleton<TdrsOpStat>();
   tdrsOpStat.init();
   auto& tdrsControlStat = getStatSingleton<TdrsControlStat>();
   tdrsControlStat.init();

   tdrsOpStat.writeStat<sis_adapt::BlahTag>(88);

   tdrsControlStat.parseUserCommands(argc, argv);

   return 0;
}
