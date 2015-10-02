//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

#include "common.h"
#include <memory>

using namespace stat_log;

int main(int argc, char** argv)
{
   initializeStatistics<true == IsOperational>();
   auto& stat = getStatLogSingleton<OpStat>();

   stat.writeStat<sis::CHANNEL_QUALITY_TAG>(88);
   stat.writeStat<hw_intf::FPGA_FAULT_TAG>(0x123);

   stat.getLog<mac::MAC_LOG>(INFO) << "HELLO from MAC";
   stat.getLog<hw_intf::HW_INTF_LOG>(ERROR) << "HELLO from HW";

   while(true)
   {
      std::this_thread::sleep_for(std::chrono::seconds{2});
      stat.getLog<mac::MAC_LOG>(INFO) << "HELLO from MAC (loop)";
      stat.getLog<hw_intf::HW_INTF_LOG>(ERROR) << "HELLO from HW (loop)";
      stat.writeStat<hw_intf::FPGA_FAULT_TAG>(1.0);
   }
   stat.stop();
   return 0;
}
