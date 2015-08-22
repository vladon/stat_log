#pragma once
#include "stat_log/loggers/logger_common.h"
#include <iostream>
#include <ctime>
#include <iomanip>
#include <chrono>

namespace stat_log
{
   class shared_mem_logger_generator : public LoggerGenerator
   {
      //TODO: The constructor should take the name of the
      // shared memory segment and the size
      private:
         virtual void doWriteData(const char* buf, std::size_t len,
               const char* TagName, const char* LogLevelName,
               std::time_t time_stamp,
               std::chrono::microseconds time_stamp_us)
         {
            //NOTE: localtime is NOT threadsafe, so any calls to it will
            // only be done in "control" mode (the cout here is just for testing).
            char mbstr[100];
            std::strftime(mbstr, sizeof(mbstr), "%T", std::localtime(&time_stamp));
            std::cout << std::dec
               // << "Time = " << std::put_time(std::localtime(&time_stamp), "%T")
               << "Time = " << mbstr
               << "." << time_stamp_us.count()
               << ": BUFFER = " << buf << std::endl;

            // TODO: Need to first figure out how many sub shm bufs
            //  this log entry will require.  Then need to acquire
            //  a lock and push the log entry to shm.
            //NOTE: the TagName, LogLevelName, and time_stamp fields
            // will also be written to shared memory, but will be
            // considered metadata.  The logger-retriever can then
            // use this metadata -- e.g. to filter out unwanted entries
            // or to not print the time stamp along, etc.
         }
   };

   //TODO
#if 0
   class shared_mem_logger_retriever : public LoggerRetriever
   {
   };
#endif

}
