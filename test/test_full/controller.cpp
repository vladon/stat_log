#include "common.h"

using namespace stat_log;

int main(int argc, char** argv)
{
   initializeStatistics<false == IsOperational>();
   handleCommandLineArgs(argc, argv);
   return 0;
}
