#pragma once
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <stat_log/loggers/logger_common.h>
#include <iostream>
#include <ctime>
#include <iomanip>
#include <chrono>
#include <ostream>
#include <mutex>
#include <boost/any.hpp>

namespace stat_log
{
   class shared_mem_logger_generator : public LoggerGenerator
   {
      public:
         shared_mem_logger_generator(const std::string& shm_name,
               std::size_t shm_size);
      private:
         virtual void doWriteData(const char* buf, std::size_t len,
               const char* TagName, const char* LogLevelName,
               std::time_t time_stamp,
               std::chrono::microseconds time_stamp_us);

         boost::interprocess::mapped_region region;
         std::mutex mtx;
         char* shm_ptr;
         std::size_t currentLogEntry;
         const std::size_t numLogEntries;
   };

   class shared_mem_logger_retriever : public LoggerRetriever
   {
      public:
         shared_mem_logger_retriever(const std::string& shm_name,
               std::size_t shm_size);

         void getLog(boost::any& log_cmd);

      private:
         boost::interprocess::mapped_region region;
         const char* shm_ptr;
         const std::size_t numLogEntries;
   };
}
