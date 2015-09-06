#include "common.h"
#include "hw_intf_stat_tags.h"
#include <stat_log/stat_log.h>
#include <stat_log/backends/shared_mem_backend.h>
#include <stat_log/loggers/shared_memory_logger.h>
#include <stat_log/util/compile_proxy.h>

//Statistics
#include <stat_log/stats/stats_common.h>
#include <stat_log/stats/simple_counter.h>
#include <stat_log/stats/simple_status.h>
#include <stat_log/stats/stat_array.h>
#if 1
#include <stat_log/stats/accumulator.h>
#include <stat_log/stats/accumulator_types/count.h>
#include <stat_log/stats/accumulator_types/min.h>
#include <stat_log/stats/accumulator_types/max.h>
#include <stat_log/stats/accumulator_types/mean.h>
#include <stat_log/stats/accumulator_types/histogram.h>

#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics.hpp>
#endif

#include <type_traits>

#define STAT_LOG_MAC_SIS_SHM_NAME "STAT_LOG_MAC_SIS"
#define STAT_LOG_HW_INTF_SHM_NAME "STAT_LOG_HW_INTF"
#define STAT_LOG_TEXT_LOGGER_NAME "SHM_LOGGER_TEXT"
#define STAT_LOG_TEXT_LOGGER_SIZE_BYTES 4194304
#define STAT_LOG_HEXDUMP_LOGGER_NAME "SHM_LOGGER_HEXDUMP"
#define STAT_LOG_HEXDUMP_LOGGER_SIZE_BYTES 4194304


using MacSisOpStat = stat_log::LogStatOperational<MAC_SIS_STATS, MAC_SIS_LOG>;
using MacSisControlStat = stat_log::LogStatControl<MAC_SIS_STATS, MAC_SIS_LOG>;

using HwIntfOpStat = stat_log::LogStatOperational<hw_intf::HW_INTF_STATS, hw_intf::HW_INTF_LOG>;
using HwIntfControlStat = stat_log::LogStatControl<hw_intf::HW_INTF_STATS, hw_intf::HW_INTF_LOG>;

using LogGen = stat_log::shared_mem_logger_generator;
using LogRet = stat_log::shared_mem_logger_retriever;

/*********************************
 * Statistic definitions
 *********************************/
namespace stat_log
{
   //Default stat
   template <typename Tag, class Enable>
   struct stat_tag_to_type
   {
      using type = SimpleCounter<int>;
   };

   template <>
   struct stat_tag_to_type<sis::MAC_PKTS_DOWN_TAG>
   {
      using ChildStat = int;
      using type = StatArray<4, StatArray<6, ChildStat>>;
   };

   //Default "HwIntf" stat
   template <typename Tag>
   struct stat_tag_to_type<Tag,
               std::enable_if_t<
                 std::is_base_of<hw_intf::HwIntfBase, Tag>::value
               >
            >
   {
      using type = SimpleCounter<int>;
   };

   namespace ba = boost::accumulators;
#if 1
#if 1
   using TheAccum = stat_log::HistogramCount<
      double,
      0, //start bin
      10,//stop bin
      10 //num bins
         >;
#else
   using TheAccum = ba::accumulator_set<
      double
      , ba::stats<
      ba::tag::count
      , ba::tag::min
      , ba::tag::max
      , ba::tag::mean
      >
      >;
#endif

   //Complete specialization for the HW FAULT
   template <>
   struct stat_tag_to_type<hw_intf::MISC_FPGA_FAULT_TAG>
   {
      using type = Accumulator<TheAccum>;
   };
#endif
}

/*********************************
 * common logger and initializer definitions
 *********************************/
namespace
{
   using namespace stat_log;
   template <typename Tag>
   inline LogGenProxy logCommon(LogLevel ll, int log_idx)
   {
      using TheOpStat = std::conditional_t<
         TagBelongsToStat<Tag, MacSisOpStat>::value,
         MacSisOpStat,
         HwIntfOpStat>;
      TheOpStat& stat = stat_log::getStatSingleton<TheOpStat>();
      switch(ll)
      {
         case LogLevel::DEBUG:
            return stat.template getDebugLog<Tag>(log_idx);
         case LogLevel::INFO:
            return stat.template getInfoLog<Tag>(log_idx);
         case LogLevel::ALERT:
            return stat.template getAlertLog<Tag>(log_idx);
         case LogLevel::ERROR:
            return stat.template getErrorLog<Tag>(log_idx);
      }
      stat.template getErrorLog<Tag>(0) << "Invalid log level!";
      return stat.template getErrorLog<Tag>(log_idx);
   }

   template <typename MacSisStatType, typename HwIntfStatType, typename LogType>
   void initializeCommon()
   {
      auto& macSisStat = stat_log::getStatSingleton<MacSisStatType>();
      auto& hwIntfStat = stat_log::getStatSingleton<HwIntfStatType>();

      macSisStat.init(STAT_LOG_MAC_SIS_SHM_NAME);
      hwIntfStat.init(STAT_LOG_HW_INTF_SHM_NAME);
      auto text_logger = std::make_shared<LogType>(
            STAT_LOG_TEXT_LOGGER_NAME,
            STAT_LOG_TEXT_LOGGER_SIZE_BYTES);

      macSisStat.addLogger(text_logger);
      hwIntfStat.addLogger(text_logger);

      auto hexdump_logger = std::make_shared<LogType>(
            STAT_LOG_HEXDUMP_LOGGER_NAME,
            STAT_LOG_HEXDUMP_LOGGER_SIZE_BYTES);

      macSisStat.addLogger(hexdump_logger);
      hwIntfStat.addLogger(hexdump_logger);
   }
}

/*********************************
 * definitions of the "compile_proxy" API's methods
 *********************************/
namespace stat_log
{
   template <typename Tag>
   LogGenProxy logger(LogLevel ll)
   {
      return logCommon<Tag>(ll, 0);
   }

   template <typename Tag>
   LogGenProxy hexDumper(LogLevel ll)
   {
      return logCommon<Tag>(ll, 1);
   }

   template <typename Tag, typename ...Args>
   void writeStat(Args... args)
   {
      using TheOpStat = std::conditional_t<
         TagBelongsToStat<Tag, MacSisOpStat>::value,
         MacSisOpStat,
         HwIntfOpStat>;
      stat_log::getStatSingleton<TheOpStat>().template writeStat<Tag>(args...);
   }

   //EXPLICIT TEMPLATE INSTANTIATIONS
   template void writeStat<mac::IP_PKTS_UP_TAG>(int val);
   template void writeStat<sis::MAC_PKTS_DOWN_TAG>(int proto_idx, int prio_idx, int val);
   template void writeStat<hw_intf::MISC_FPGA_FAULT_TAG>(int val);
//TODO: this will be super annoying for the user to have to define the
// stat hierarchy AND explicitly instantiate each of them ...
// Think  of a way to automate this.

}

template <>
void initializeStatistics<false == IsOperational>()
{
    initializeCommon<MacSisControlStat, HwIntfControlStat, LogRet>();
}

template <>
void initializeStatistics<true == IsOperational>()
{
   initializeCommon<MacSisOpStat, HwIntfOpStat, LogGen>();
}

void handleCommandLineArgs(int argc, char** argv)
{
   auto& macSisControlStat = getStatSingleton<MacSisControlStat>();
   auto& hwIntfControlStat = getStatSingleton<HwIntfControlStat>();

   //Assign a few enumeration and dimension names to make stat-viewing pretty.
   hwIntfControlStat.assignEnumerationNames<
      hw_intf::MISC_FPGA_FAULT_TAG>({"OVER_TEMP", "INVALID_COMMAND", "UNKNOWN_FAULT"});

   macSisControlStat.assignEnumerationNames<
      sis::MAC_PKTS_DOWN_TAG>({"L", "M", "H"});

   macSisControlStat.assignDimensionNames<
      sis::MAC_PKTS_DOWN_TAG>({"Priority", "Traffic Type"});

   macSisControlStat.parseUserCommands(argc, argv);
   hwIntfControlStat.parseUserCommands(argc, argv);
}

