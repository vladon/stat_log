#include "mac_stat_tags.h"
#include "sis_stat_tags.h"
#include <stat_log/util/compile_proxy.h>
#include <thread>
#include <random>
#include <chrono>

namespace
{

using namespace stat_log;
std::thread mac_thread;

//This method is responsible for generating artificial MAC stats and logs
void mac_event_handler()
{
   std::random_device rd;
   std::mt19937 gen(rd());
   std::exponential_distribution<> d_time(1); //1 event per second (on average)
   std::discrete_distribution<> d_which_stat({40, 40, 15, 5});
   // std::cauchy_distribution<> d_pkt_size(800, 8.0);
   std::cauchy_distribution<> d_pkt_size(0.0, 0.5);

   while(true)
   {

      auto which_stat = d_which_stat(gen);
      switch(which_stat)
      {
         case 0: //Got IP packet from above
         {
            writeStat<mac::IP_PKTS_SENT_TAG>(1);
            int pkt_size = (int)std::min(1500.0, std::max(10.0, 800 + 400*d_pkt_size(gen)));
            writeStat<mac::IP_PKT_SENT_SIZE_TAG>(pkt_size);
            logger<mac::MAC_LOG>(DEBUG) << "Sending IP packet with size = " << pkt_size;
            break;
         }
         case 1: //Got IP packet from below
         {
            writeStat<mac::IP_PKTS_RCVD_TAG>(1);
            int pkt_size = (int)std::min(1500.0, std::max(10.0, 800 + 400*d_pkt_size(gen)));
            writeStat<mac::IP_PKT_RCVD_SIZE_TAG>(pkt_size);
            logger<mac::MAC_LOG>(DEBUG) << "Receiving IP packet with size = " << pkt_size;
            break;
         }
         case 2: //Time to update the tx and rx power level stats
         {
            for(auto n = 0; n < sis::max_nbrs; ++n)
            {
               writeStat<mac::TX_POWER_LEVEL_TAG>(n, 0);
               writeStat<mac::RX_POWER_LEVEL_TAG>(n, 1.1);
            }
            break;
         }
         case 3: //This simulates a re-calculation of the TDMA slot schedule
         {
            //TODO: print out the new TDMA slot schedule
         }
      }
      auto wait_time  = std::chrono::microseconds{1+1000000*(int)d_time(gen)};
      std::this_thread::sleep_for(wait_time);
   }
}

}

void genStats_MAC()
{

   mac_thread = std::thread([]()
      {
         mac_event_handler();
      });
}

