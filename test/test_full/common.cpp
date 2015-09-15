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
   namespace ba = boost::accumulators;
   constexpr int max_nbrs = 10;
   //Default stat
   template <typename Tag, class Enable>
   struct stat_tag_to_type
   {
      using type = SimpleCounter<int>;
   };

   //Mac: Frags sent: Histogram of fragment sizes
   template <>
   struct stat_tag_to_type<mac::FRAGS_SENT_TAG>
   {
      using type = Accumulator<
         stat_log::HistogramCount<
            int,
            1, //start bin
            1500, //stop bin
            10 //num_bits
         >
      >;
   };

   //Mac: Frags rcvd: summary statistics of fragment sizes:
   //  count, min, max, mean
   template <>
   struct stat_tag_to_type<mac::FRAGS_RCVD_TAG>
   {
      using type = Accumulator<
         ba::accumulator_set<
               double
               , ba::stats<
                  ba::tag::count
                  , ba::tag::min
                  , ba::tag::max
                  , ba::tag::mean
               >
            >
         >;
   };

   //Mac: tx power level: array of power levels (ints)
   template <>
   struct stat_tag_to_type<mac::TX_POWER_LEVEL_TAG>
   {
      using type = StatArray<max_nbrs, SimpleStatus<int>>;
   };

   //Mac: rx power level: array of received powers in dB (doubles)
   template <>
   struct stat_tag_to_type<mac::RX_POWER_LEVEL_TAG>
   {
      using type = StatArray<max_nbrs, SimpleStatus<double>>;
   };

   //Sis: prop delay: per-nbr density histogram of delays
   template <>
   struct stat_tag_to_type<sis::PROP_DELAY_TAG>
   {
      using type = StatArray<max_nbrs,
               Accumulator<
                  stat_log::HistogramCount<
                     double,
                     20, //start bin
                     400, //stop bin
                     8 //num_bits
                  >
               >
            >;
   };

   //Sis: channel chality: per-nbr, per-frequency channel quality enumeration
   template <>
   struct stat_tag_to_type<sis::CHANNEL_QUALITY_TAG>
   {
      //4 possible stati
      using type = StatArray<max_nbrs,
                                StatArray<4,
                                    SimpleStatus<int>
                                 >
                            >;
   };

   //Sis: frame rx status: array of counters (per rx status)
   template <>
   struct stat_tag_to_type<sis::FRAME_RX_STATUS_TAG>
   {
      //5 possible stati
      using type = StatArray<5, int>;
   };

   //Sis: link status: per-nbr link status indicator (int)
   template <>
   struct stat_tag_to_type<sis::LINK_STATUS_TAG>
   {
      using type = StatArray<max_nbrs, SimpleStatus<int>>;
   };

   //HwIntf: fault: array of counters (per each fault type)
   template <>
   struct stat_tag_to_type<hw_intf::FPGA_FAULT_TAG>
   {
      //5 fault types
      using type = StatArray<5, int>;
   };

   //HwIntf: interrupt: array of counters (per each interrupt)
   template <>
   struct stat_tag_to_type<hw_intf::INTERRUPT_TAG>
   {
      //3 interrupt types
      using type = StatArray<3, int>;
   };
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
      constexpr auto is_mac_sis_tag = TagBelongsToLog<Tag, MacSisOpStat>::value;
      constexpr auto is_hw_intf_tag = TagBelongsToLog<Tag, HwIntfOpStat>::value;
      static_assert(is_mac_sis_tag || is_hw_intf_tag, "Bad tag in logCommon!\n");
      using TheOpStat = std::conditional_t<
         is_mac_sis_tag, MacSisOpStat, HwIntfOpStat>;
      TheOpStat& stat = stat_log::getStatLogSingleton<TheOpStat>();
      return stat.template getLog<Tag>(ll, log_idx);
   }

   template <typename MacSisStatType, typename HwIntfStatType, typename LogType>
   void initializeCommon()
   {
      auto& macSisStat = stat_log::getStatLogSingleton<MacSisStatType>();
      auto& hwIntfStat = stat_log::getStatLogSingleton<HwIntfStatType>();

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
      constexpr auto is_mac_sis_tag = TagBelongsToStat<Tag, MacSisOpStat>::value;
      constexpr auto is_hw_intf_tag = TagBelongsToStat<Tag, HwIntfOpStat>::value;
      static_assert(is_mac_sis_tag || is_hw_intf_tag, "Bad tag in writeStat!\n");
      using TheOpStat = std::conditional_t<
         is_mac_sis_tag, MacSisOpStat, HwIntfOpStat>;
      stat_log::getStatLogSingleton<TheOpStat>().template writeStat<Tag>(args...);
   }

   //EXPLICIT TEMPLATE INSTANTIATIONS
   //MAC
   template void writeStat<mac::IP_PKTS_SENT_TAG>(int val);
   template void writeStat<mac::IP_PKTS_RCVD_TAG>(int val);
   template void writeStat<mac::FRAGS_SENT_TAG>(int val);
   template void writeStat<mac::FRAGS_RCVD_TAG>(int val);
   template void writeStat<mac::TX_POWER_LEVEL_TAG>(int idx, double val);
   template void writeStat<mac::RX_POWER_LEVEL_TAG>(int idx, double val);

   //SIS
   template void writeStat<sis::PROP_DELAY_TAG>(int nbr_idx, double delay_us);
   template void writeStat<sis::CHANNEL_QUALITY_TAG>(int nbr_idx, int freq_idx, double qual);
   template void writeStat<sis::FRAME_RX_STATUS_TAG>(int rx_status_enum, int val);
   template void writeStat<sis::LINK_STATUS_TAG>(int nbr_idx, int val);

   //HW_INTF
   template void writeStat<hw_intf::FPGA_FAULT_TAG>(int fault_idx, int val);
   template void writeStat<hw_intf::BYTES_SENT_TAG>(int val);
   template void writeStat<hw_intf::BYTES_RCVD_TAG>(int val);
   template void writeStat<hw_intf::INTERRUPT_TAG>(int interrupt_idx, int val);

   template LogGenProxy logger<hw_intf::HW_INTF_LOG>(LogLevel ll);
   template LogGenProxy hexDumper<hw_intf::HW_INTF_LOG>(LogLevel ll);
   template LogGenProxy logger<mac::MAC_LOG>(LogLevel ll);
   template LogGenProxy hexDumper<sis::SIS_LOG>(LogLevel ll);

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
   auto& macSisControlStat = getStatLogSingleton<MacSisControlStat>();
   auto& hwIntfControlStat = getStatLogSingleton<HwIntfControlStat>();

   //Assign a few enumeration and dimension names to make stat-viewing pretty.
   // TODO: design this better
#if 0
   hwIntfControlStat.assignEnumerationNames<
      hw_intf::MISC_FPGA_FAULT_TAG>({"OVER_TEMP", "INVALID_COMMAND", "UNKNOWN_FAULT"});

   macSisControlStat.assignEnumerationNames<
      sis::MAC_PKTS_DOWN_TAG>({"L", "M", "H"});

   macSisControlStat.assignDimensionNames<
      sis::MAC_PKTS_DOWN_TAG>({"Priority", "Traffic Type"});
#endif

   macSisControlStat.parseUserCommands(argc, argv);
   hwIntfControlStat.parseUserCommands(argc, argv);
   macSisControlStat.showOutput();
}

