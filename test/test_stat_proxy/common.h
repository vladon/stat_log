#include "mac_stat_tags.h"
#include "sis_stat_tags.h"
#include "hw_intf_stat_tags.h"

#include "stat_log/stat_log.h"
#include "stat_log/backends/shared_mem_backend.h"
#include "stat_log/loggers/shared_memory_logger.h"
#include "stat_log/stats/stats_common.h"
#include "stat_log/stats/simple_counter.h"
#include "stat_log/stats/simple_status.h"
#include "stat_log/stats/stat_array.h"
#include "stat_log/util/compile_proxy.h"

#include <iostream>
#include <vector>
#include <string>
#include <sstream>


/*********************************
 * Statistic definitions
 *********************************/

struct TOP_STAT
{
   SL_NAME = "TOP_LEVEL";
   using ChildTypes = boost::fusion::vector<
      MAC,
      SIS,
      HW_INTERFACE
   >;
};

constexpr bool IsOperational = false;

using OpStat = stat_log::LogStatOperational<TOP_STAT>;
using ControlStat = stat_log::LogStatControl<TOP_STAT>;
/*********************************/
using LoggerGenerator = stat_log::shared_mem_logger_generator;
using LoggerRetriever = stat_log::shared_mem_logger_retriever;

template <bool IsOperational>
void initializeStatistics();

void genStats_MAC();
void genStats_SIS();
void genStats_HW_INTF();

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

   template <typename Tag, typename ... Args>
   void writeStat(Args... args)
   {
      stat_log::getStatSingleton<OpStat>().writeStat<Tag>(args...);
   }

   //EXPLICIT TEMPLATE INSTANTIATIONS
   template void writeStat<MAC::IP_PKTS_UP_TAG>(int val);
   template void writeStat<SIS::MAC_PKTS_DOWN_TAG>(int proto_idx, int prio_idx, int val);
   template void writeStat<HW_INTERFACE::MISC_FPGA_FAULT_TAG>(int val);
//TODO: this will be super annoying for the user to have to define the
// stat hierarchy AND explicitly instantiate each of them ...
// Think  of a way to automate this.

}

#define STAT_LOG_SHM_NAME "stat_log"
#define STAT_LOG_LOGGER_NAME "SHM_LOGGER1"
#define STAT_LOG_LOGGER_SIZE_BYTES 4194304
