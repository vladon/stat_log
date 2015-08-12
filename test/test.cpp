#include "stat_log.h"
#include "backends/shared_mem_backend.h"
// #include "parsers/parent_parser.h"
#include <readline/readline.h>
#include <readline/history.h>

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
            << ", parent is " << Parent::name
            << ", depth is " << T::depth
            << std::endl;
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
            << ", depth is " << T::depth
            // << ", addr is " << std::hex << &getValue<Tag>(theOpStats)
            << std::endl;
      }
};
#endif


int main(int argc, char** argv)
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
   tdrsControlStat.assignShmPtr(shm_start);

   std::cout << "\n\n, all strings\n";
   PrintStrings::print(TagHierarchy{}, boost::mpl::true_{});

   // std::cout << "Tags: = " << TypeId<tdrsOpStat>{} << std::endl;

   getValue<sis_adapt::BlahTag>(theOpStats) = 42;
   getValue<sis_adapt::NodeStatsTag::RxCountTag>(theOpStats) = 38;
   std::cout <<  getValue<sis_adapt::BlahTag>(theOpStats) << std::endl;

   std::cout <<  "OPERATIONAL value = "
      << getValue<sis_adapt::BlahTag>(tdrsControlStat.theStats) << std::endl;

   PrintStrings::print(TagHierarchy{}, boost::mpl::true_{});
#endif


#if 0
   auto desc = createTopConfig();
   processCmdLineOptions(*desc.get(), argc, argv);
#endif
   tdrsControlStat.parseUserCommands(argc, argv);

#if 0
   char* line;
   while(1)
   {
      line = readline("> ");


      add_history(line);
   }
#endif

   //
   // std::string query_string = "

}
