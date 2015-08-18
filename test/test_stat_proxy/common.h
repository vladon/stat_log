#include "mac_stat_tags.h"
#include "sis_stat_tags.h"
#include "hw_intf_stat_tags.h"

#include "stat_proxy_interface.h"

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

struct TOP_STAT
{
   NAME = "TOP_LEVEL";
   using ChildTypes = boost::fusion::vector<
      MAC,
      SIS,
      HW_INTERFACE
   >;
};


using OpStat = stat_log::LogStatOperational<TOP_STAT, LoggerType>;
using ControlStat = stat_log::LogStatControl<TOP_STAT, LoggerType>;
/*********************************/

namespace stat_log
{

   template <typename Tag>
   struct stat_traits<Tag>
   {
      using StatType = SimpleCounter<int>;
   };
}


//TODO
#if 0
namespace stat_log
{

   template <typename Tag>
      struct stat_traits<Tag, typename std::enable_if<
      std::is_base_of<HwIntfBase, Tag>::value
      >::type
      >
      {
         using StatType = SimpleCounter<char>;
      };
}

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
#endif

LoggerType& getLoggerRef();
