#include "common.h"
#include <thread>
#include <chrono>

int main(int argc, char** argv)
{
   initializeStatistics<true == IsOperational>();
   genStats_MAC();
   genStats_SIS();
   genStats_HW_INTF();

   std::this_thread::sleep_for(std::chrono::seconds{10});
   return 0;
}
