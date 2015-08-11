#include "stat_log.h"
#include "backends/shared_mem_backend.h"

using namespace boost::fusion;
using namespace rc::stat_log;
/*Statistic definitions */
struct sndcf
{
   NAME = "SNDCF";
   struct IpDwnTag{
      NAME = "IP_DOWN";
   };
   struct IpUpTag{
      NAME = "IP_UP";
   };

   using ChildTypes = vector<IpDwnTag, IpUpTag>;
};

struct sis_adapt
{
   NAME = "SIS_ADAPT";
   struct BlahTag{
      NAME = "BLAH";
   };
   struct NodeStatsTag{
      NAME = "NODE_STATS";
      struct RxCountTag{
         NAME = "RX_COUNTERS";
      };
      struct CorrScoreTag{
         NAME = "CORR_SCORE";
      };
      struct LinkIndTag{
         NAME = "LINK_IND";
      };

      using ChildTypes = vector<RxCountTag, CorrScoreTag, LinkIndTag>;
   };

   using ChildTypes = vector<BlahTag, NodeStatsTag>;
};

struct etdrs_stats
{
   NAME = "TOP_LEVEL";
   using ChildTypes = vector<sndcf, sis_adapt>;
};

//TODO: threading vs no-threaded policy
//

namespace rc
{
namespace stat_log
{

   template <typename T, bool IsOperational>
   struct stat_traits
   {
      using SerialType = int;
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
}

using TdrsOpStat = LogStatOperational<etdrs_stats>;
TdrsOpStat tdrsOpStat;
using TdrsControlStat = LogStatControl<etdrs_stats>;
TdrsControlStat tdrsControlStat;

int level = 0;
#if 1
auto& theOpStats = tdrsOpStat.theStats;

struct PrintStrings
{
   static void indent()
   {
      int i = level;
      while(i-- > 0)
         std::cout << "\t";
   }
   template <typename T>
      void operator()(T& t) const
      {
         print(t, typename traits::is_sequence<typename T::child_list>::type{});
      }

   template <typename T>
      static void print(T type, boost::mpl::true_)
      {
         indent();
         using Parent = typename T::parent;
         std::cout << "(PARENT) Type is = " << T::name
            << ", parent is " << Parent::name << std::endl;
         level++;
         using ChildList = typename T::child_list;
         for_each(ChildList{}, PrintStrings{});
         level--;
      }

   template <typename T>
      static void print(T type, boost::mpl::false_)
      {
         indent();
         using Parent = typename T::parent;
         using Tag = typename T::tag;
         std::cout << "Type is = " << T::name
            << ", parent is " << Parent::name
            << ", value is " << getValue<Tag>(theOpStats)
            // << ", addr is " << std::hex << &getValue<Tag>(theOpStats)
            << std::endl;
      }
};
#endif

int main(void)
{
   using TagHierarchy = TdrsOpStat::TagHierarchy;
   std::cout << "The Type = " << TypeId<TagHierarchy>{} << std::endl;
#if 1

   shared_mem_backend shm_backend;
   shm_backend.setParams("ROB", sizeof(theOpStats));
   auto shm_start = shm_backend.getMemoryPtr();
   std::cout << "SHM size = " << sizeof(theOpStats)
         <<" , shm_start = " << std::hex << shm_start << std::endl;

   //Next, need to inform tdrsOpStat of the start of shared_memory
   tdrsOpStat.assignShmPtr(shm_start);
#if 1
   tdrsControlStat.assignShmPtr(shm_start);
#endif



   std::cout << "\n\n, all strings\n";
   PrintStrings::print(TagHierarchy{}, boost::mpl::true_{});

   // std::cout << "Tags: = " << TypeId<tdrsOpStat>{} << std::endl;

   getValue<sis_adapt::BlahTag>(theOpStats) = 42;
   std::cout <<  getValue<sis_adapt::BlahTag>(theOpStats) << std::endl;

#if 1
   std::cout <<  "OPERATIONAL value = "
      << getValue<sis_adapt::BlahTag>(tdrsControlStat.theStats) << std::endl;
#endif

   PrintStrings::print(TagHierarchy{}, boost::mpl::true_{});

#endif

}
