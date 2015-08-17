#include "hw_intf.h"

#include "stat_log/stat_log.h"
#include "stat_log/backends/shared_mem_backend.h"
#include "stat_log/stats/stats_common.h"
#include "stat_log/stats/simple_counter.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>

/*********************************
 * Statistic definitions
 *********************************/
struct MacSisBase {};

struct MAC
{
   NAME = "MAC";
   MAKE_STAT_TAGS_NAMED_BASE(
         MacSisBase,
      (IP_PKTS_DOWN)
      (IP_PKTS_UP)
      (BUFFER_OVERFLOW)
   )
};

struct SIS
{
   NAME = "SIS";

   MAKE_STAT_TAG_STRUCT_BASE(MAC_PKTS_DOWN, MacSisBase)
   MAKE_STAT_TAG_STRUCT_BASE(MAC_PKTS_UP, MacSisBase)

   struct PER_NBR_STATS{
      NAME = "PER_NBR_STATS";
      MAKE_STAT_TAGS_NAMED_BASE(
            MacSisBase,
         (LINK_QUALITY)
         (RECEIVE_STATUS)
         (LINK_STATUS)
      )
   };

   using ChildTypes = boost::fusion::vector<
      MAC_PKTS_DOWN_TAG,
      MAC_PKTS_UP_TAG,
      PER_NBR_STATS
   >;
};

struct TOP_MAC_SIS
{
   NAME = "TOP_LEVEL";
   using ChildTypes = boost::fusion::vector<
      MAC,
      SIS
   >;
};


using OpStatMacSis = stat_log::LogStatOperational<TOP_MAC_SIS, LoggerType>;
using ControlStatMacSis = stat_log::LogStatControl<TOP_MAC_SIS, LoggerType>;
/*********************************/

//TODO: threading vs no-threaded policy
//
namespace stat_log
{

   template <typename Tag>
   struct stat_traits<Tag, typename std::enable_if<
      std::is_base_of<MacSisBase, Tag>::value
      >::type
   >
   {
      using StatType = SimpleCounter<int>;
   };
}


template <bool IsOperational>
void initializeStatistics();

LoggerType& getLoggerRef();
