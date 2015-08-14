#include "stat_log.h"
#include "backends/shared_mem_backend.h"

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

   template <typename Repr>
   struct Wrapper
   {
      static constexpr size_t getSize()
      {
         return sizeof(Repr);
      }
      Repr storage;
   };

   template <typename T, bool IsOperational>
   struct stat_traits
   {
      using SerialType = Wrapper<int>;
      using ProxyType = OpProxyBasic;
   };

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

using TdrsOpStat = LogStatOperational<etdrs_stats>;
TdrsOpStat tdrsOpStat;
using TdrsControlStat = LogStatControl<etdrs_stats>;
TdrsControlStat tdrsControlStat;

auto& theOpStats = tdrsOpStat.theStats;

int main(int argc, char** argv)
{
#if 0
   using TagHierarchy = TdrsOpStat::TagHierarchy;
   std::cout << "The Type = " << TypeId<TagHierarchy>{} << std::endl;
#endif


#if 1
   shared_mem_backend shm_backend;
   shm_backend.setParams("ROB", sizeof(theOpStats));
   auto shm_start = shm_backend.getMemoryPtr();
   std::cout << "SHM size = " << sizeof(theOpStats)
      <<" , shm_start = " << std::hex << shm_start << std::endl;

   //Next, need to inform tdrsOpStat of the start of shared_memory
   tdrsOpStat.assignShmPtr(shm_start);
   tdrsControlStat.assignShmPtr(shm_start);

   getValue<sis_adapt::BlahTag>(theOpStats) = 42;
   getValue<sis_adapt::NodeStatsTag::RxCountTag>(theOpStats) = 38;
   std::cout <<  getValue<sis_adapt::BlahTag>(theOpStats) << std::endl;

   std::cout <<  "OPERATIONAL value = "
      << getValue<sis_adapt::BlahTag>(tdrsControlStat.theStats) << std::endl;

#endif

   tdrsControlStat.parseUserCommands(argc, argv);

}
