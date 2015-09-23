#include "hw_intf_stat_tags.h"
#include <stat_log/util/compile_proxy.h>

#include <thread>
#include <random>
#include <chrono>

namespace
{

using namespace stat_log;
std::thread fpga_thread;

//This method is responsible for generating artificial FPGA stats and logs
void fpga_event_handler()
{
   std::random_device rd;
   std::mt19937 gen(rd());
   std::exponential_distribution<> d_time(1); //1 event per second (on average)
   // std::uniform_int_distribution<> d_which_stat(0, 3);
   std::discrete_distribution<> d_which_stat({10, 30, 30, 30});
   std::uniform_int_distribution<> d_which_fault(0, hw_intf::num_fpga_faults - 1);
   std::uniform_int_distribution<> d_which_interrupt(0, hw_intf::num_fpga_interrupts - 1);
   while(true)
   {

      auto which_stat = d_which_stat(gen);
      switch(which_stat)
      {
         case 0: //FPGA_FAULT
         {
            auto which_fault = d_which_fault(gen);
            logger<hw_intf::HW_INTF_LOG>(ALERT) << "FPGA fault = " << which_fault;
            writeStat<hw_intf::FPGA_FAULT_TAG>(which_fault, 1);
            break;
         }
         case 1: //BYTES_SENT
         {
            logger<hw_intf::HW_INTF_LOG>(DEBUG) << "sending 100 bytes to the FPGA";
            writeStat<hw_intf::BYTES_SENT_TAG>(100);
            break;
         }
         case 2: //BYTES_RECEIVED
         {
            logger<hw_intf::HW_INTF_LOG>(DEBUG) << "receiving 100 bytes from the FPGA";
            writeStat<hw_intf::BYTES_RCVD_TAG>(100);
            break;
         }
         case 3: //INTERRUPT
         {
            auto which_int = d_which_interrupt(gen);
            logger<hw_intf::HW_INTF_LOG>(DEBUG) << "Got interrupt from FPGA: " << which_int;
            writeStat<hw_intf::INTERRUPT_TAG>(which_int, 1);
            break;
         }
      }

      auto wait_time  = std::chrono::microseconds{1+1000000*(int)d_time(gen)};
      std::this_thread::sleep_for(wait_time);
   }
}

}

void genStats_HW_INTF()
{
   fpga_thread = std::thread([]()
      {
         fpga_event_handler();
      });
}
