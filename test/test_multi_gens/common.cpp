#include "common.h"
#include "hw_intf.h"

LoggerType& getLoggerRef()
{
   static LoggerType theLogger;
   return theLogger;
}

