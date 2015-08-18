#include "mac_stat_tags.h"
#include "sis_stat_tags.h"
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
      std::is_base_of<MacBase, Tag>::value
      >::type
   >
   {
      using StatType = SimpleCounter<int>;
   };

   template <typename Tag>
   struct stat_traits<Tag, typename std::enable_if<
      std::is_base_of<SisBase, Tag>::value
      >::type
   >
   {
      using StatType = SimpleCounter<int>;
   };
}


template <bool IsOperational>
void initializeStatistics();

LoggerType& getLoggerRef();
