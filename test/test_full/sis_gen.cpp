//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

#include "sis_stat_tags.h"
#include <stat_log/util/compile_proxy.h>

#include <thread>
#include <random>
#include <chrono>

namespace
{

using namespace stat_log;
std::thread sis_thread;

struct Neighbor
{
   std::uniform_real_distribution<> d_prop_delay;
   std::array<std::normal_distribution<>, sis::num_frequencies> d_per_freq_qual;
   std::discrete_distribution<> d_rx_frame_status;
   std::discrete_distribution<> d_link_status;
   int prev_link_status = 0;
   int idx = -1;
};

//This method is responsible for generating artificial SiS stats and logs
void sis_event_handler()
{
   std::random_device rd;
   std::mt19937 gen(rd());
   std::exponential_distribution<> d_time(1); //1 event per second (on average)
   std::discrete_distribution<> d_which_stat({80, 20});

   std::array<Neighbor, sis::max_nbrs> neighbors;
   for(auto n = 0; n < sis::max_nbrs; ++n)
   {
      neighbors[n].idx = n;
      double total_channel_quality = 0.0;
      double mean_prop_delay = std::uniform_real_distribution<>{0,400}(gen) + 10;
      neighbors[n].d_prop_delay = std::uniform_real_distribution<>{mean_prop_delay - 80,
         mean_prop_delay + 80};
      for(auto f = 0; f < sis::num_frequencies; ++f)
      {
         double slope = std::uniform_real_distribution<>{0.1, 0.5}(gen);
         double mean_chan_qual = 100.0 - std::min(100.0, slope*mean_prop_delay);
         total_channel_quality += mean_chan_qual;
         neighbors[n].d_per_freq_qual[f] = std::normal_distribution<>{mean_chan_qual,
            mean_chan_qual/10.0};
      }

      double mean_chan_quality = total_channel_quality/sis::num_frequencies;
      if(mean_chan_quality < 25.0)
      {
         //Horrible link
         neighbors[n].d_rx_frame_status = neighbors[n].d_link_status
            = std::discrete_distribution<>{{0, 0, 40, 60}};
      }
      else if(mean_chan_quality < 50.0)
      {
         //Bad link
         neighbors[n].d_rx_frame_status = neighbors[n].d_link_status
            = std::discrete_distribution<>{{20, 30, 20, 20}};
      }
      else if(mean_chan_quality < 75.0)
      {
         //Fair link
         neighbors[n].d_rx_frame_status = neighbors[n].d_link_status
            = std::discrete_distribution<>{{40, 40, 10, 10}};
      }
      else
      {
         //Great link
         neighbors[n].d_rx_frame_status = neighbors[n].d_link_status
            = std::discrete_distribution<>{{80, 20, 0, 0}};
      }
   }

   while(true)
   {

      auto which_stat = d_which_stat(gen);
      switch(which_stat)
      {
         case 0: //"Got Rx pkt" event --> update all stats
         {
            for(auto n = 0; n < sis::max_nbrs; ++n)
            {
               //update prop_delay statistic
               auto prop_delay = std::max(0.0, neighbors[n].d_prop_delay(gen));
               writeStat<sis::PROP_DELAY_TAG>(n, prop_delay);
               //update frame rx_status stat
               writeStat<sis::FRAME_RX_STATUS_TAG>(n, neighbors[n].d_rx_frame_status(gen), 1);
               //update per-frequency channel quality
               for(auto f = 0; f < sis::num_frequencies; ++f)
               {
                  auto chan_qual = neighbors[n].d_per_freq_qual[f](gen);
                  writeStat<sis::CHANNEL_QUALITY_TAG>(n, f,
                        std::min(100.0, std::max(0.0, chan_qual)));
               }
            }
            break;
         }
         case 1: //Time to access the per link status
         {
            for(auto n = 0; n < sis::max_nbrs; ++n)
            {
               auto new_link_status = neighbors[n].d_link_status(gen);
               if(new_link_status != neighbors[n].prev_link_status)
               {
                  logger<sis::SIS_LOG>(INFO) << "neighbor " << n
                     << ", link status changed from " << neighbors[n].prev_link_status
                     << " to " << new_link_status;
                  neighbors[n].prev_link_status = new_link_status;
               }
               writeStat<sis::LINK_STATUS_TAG>(n, new_link_status);
            }
            break;
         }
      }
      auto wait_time  = std::chrono::microseconds{1+1000000*(int)d_time(gen)};
      std::this_thread::sleep_for(wait_time);
   }
}

}

void genStats_SIS()
{

   sis_thread = std::thread([]()
      {
         sis_event_handler();
      });

}
