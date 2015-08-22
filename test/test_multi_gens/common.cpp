#include "common.h"
#include "hw_intf.h"

LoggerGenerator& getLoggerRef()
{
   static LoggerGenerator theLogger;
   return theLogger;
}

