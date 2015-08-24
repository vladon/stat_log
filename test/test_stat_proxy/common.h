#include "mac_stat_tags.h"
#include "sis_stat_tags.h"
#include "hw_intf_stat_tags.h"

#include "stat_proxy_interface.h"

#include "stat_log/stat_log.h"
#include "stat_log/backends/shared_mem_backend.h"
#include "stat_log/stats/stats_common.h"
#include "stat_log/stats/simple_counter.h"
#include "stat_log/stats/simple_status.h"
#include "stat_log/stats/stat_array.h"

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


using OpStat = stat_log::LogStatOperational<TOP_STAT>;
using ControlStat = stat_log::LogStatControl<TOP_STAT>;
/*********************************/

namespace stat_log
{

   template <typename Tag>
   struct stat_tag_to_type<Tag>
   {
      using type = SimpleCounter<int>;
   };

   template <>
   struct stat_tag_to_type<SIS::MAC_PKTS_DOWN_TAG>
   {
      // using ChildStat = SimpleCounter<int>;
      using ChildStat = int;
      using type = StatArray<4, StatArray<6, ChildStat>>;
   };

   template <>
   struct stat_tag_to_type<HW_INTERFACE::MISC_FPGA_FAULT_TAG>
   {
      using type = SimpleStatus<int>;
   };
}

#define STAT_LOG_SHM_NAME "stat_log"
#define STAT_LOG_LOGGER_NAME "SHM_LOGGER1"
#define STAT_LOG_LOGGER_SIZE_BYTES 4194304
