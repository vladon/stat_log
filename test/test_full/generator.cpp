#include "common.h"
#include <thread>
#include <chrono>

int main(int argc, char** argv)
{
   initializeStatistics<true == IsOperational>();
   genStats_MAC();
   genStats_SIS();
   genStats_HW_INTF();

   while(true)
   {
      std::this_thread::sleep_for(std::chrono::seconds{2});
   }
   return 0;
}
