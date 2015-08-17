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
struct MAC
{
   NAME = "MAC";
   MAKE_STAT_TAGS(
      (IP_PKTS_DOWN)
      (IP_PKTS_UP)
      (BUFFER_OVERFLOW)

   )
};

struct SIS
{
   NAME = "SIS";

   MAKE_STAT_TAG_STRUCT(MAC_PKTS_DOWN)
   MAKE_STAT_TAG_STRUCT(MAC_PKTS_UP)

   struct PER_NBR_STATS{
      NAME = "PER_NBR_STATS";
      MAKE_STAT_TAGS(
         (LINK_QUALITY)
         (RECEIVE_STATUS)
         (LINK_STATUS)
      )
   };

   using ChildTypes = vector<
      MAC_PKTS_DOWN_TAG,
      MAC_PKTS_UP_TAG,
      PER_NBR_STATS
   >;
};

struct HW_INTERFACE
{
   NAME = "HW_INTERFACE";
   MAKE_STAT_TAGS(
      (MISC_FPGA_FAULT)
      (BUFFER_OVERFLOW)
   )
};


struct TOP
{
   NAME = "TOP_LEVEL";
   using ChildTypes = vector<
      MAC,
      SIS,
      HW_INTERFACE
   >;
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

using OpStat = LogStatOperational<TOP, MyLogger>;
using ControlStat = LogStatControl<TOP, MyLogger>;

int main(int argc, char** argv)
{
   auto& opStat = getStatSingleton<OpStat>();
   opStat.init();
   auto& controlStat = getStatSingleton<ControlStat>();
   controlStat.init();
   opStat.writeStat<SIS::MAC_PKTS_DOWN_TAG>(88);
   controlStat.parseUserCommands(argc, argv);
   return 0;
}
