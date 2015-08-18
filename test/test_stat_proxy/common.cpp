#include "common.h"

LoggerType& getLoggerRef()
{
   static LoggerType theLogger;
   return theLogger;
}


