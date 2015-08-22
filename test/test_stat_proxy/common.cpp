#include "common.h"

LoggerGenerator& getLoggerRef()
{
   static LoggerGenerator theLogger;
   return theLogger;
}


