#include "stat_proxy_interface.h"

constexpr bool IsOperational = true;

int main(int argc, char** argv)
{
   initializeStatistics<IsOperational>();
   genStats_MAC();
   genStats_SIS();
   genStats_HW_INTF();
   return 0;
}
