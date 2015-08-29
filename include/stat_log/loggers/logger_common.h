#pragma once
#include "stat_log/stats/stats_common.h"
#include "stat_log/parsers/parser_common.h"
#include <iomanip>
#include <chrono>
#include <array>
#include <ostream>
#include <boost/any.hpp>

namespace stat_log
{

   constexpr std::array<const char* const,  4> LogLevelNames
   {{
       "DEBUG", "INFO", "ALERT", "ERROR"
   }};

   namespace detail
   {
      //We support a maximum of 4 loggers
      using LogControlStorage = std::array<unsigned char, 4>;

      struct LogOpProxy
      {
         using SharedType = LogControlStorage;
         SharedType* shared_ptr = nullptr;
         void setSharedPtr(void* ptr)
         {
            shared_ptr = reinterpret_cast<SharedType*>(ptr);
         }

         unsigned char getLevel(int log_idx)
         {
            assert(log_idx <= (int)SharedType{}.size());
            return (*shared_ptr)[log_idx];
         }

         static constexpr size_t getSharedSize()
         {
            return sizeof(SharedType);
         }
      };

      struct LogControlProxy
      {
         using SharedType = LogControlStorage;
         SharedType* shared_ptr = nullptr;
         void setSharedPtr(void* ptr)
         {
            shared_ptr = reinterpret_cast<SharedType*>(ptr);
         }

         static constexpr size_t getSharedSize()
         {
            return sizeof(SharedType);
         }

         void doCommand(StatCmd cmd, boost::any& cmd_arg)
         {
            if(cmd == StatCmd::LOG_LEVEL)
            {
               auto logLevelCmd = boost::any_cast<LogLevelCommand>(cmd_arg);
               int log_idx = logLevelCmd.logger_idx;
               assert(log_idx < (int)SharedType{}.size());
               auto& log_level_idx = (*shared_ptr)[log_idx];
               if(logLevelCmd.set_log_level)
               {
                  std::cout << "PREV_LOG_LEVEL = " << LogLevelNames[log_level_idx] << ", ";
                  auto it = std::find(LogLevelNames.begin(), LogLevelNames.end(),
                        logLevelCmd.new_log_level);
                  if(it != LogLevelNames.end())
                  {
                     log_level_idx = std::distance(LogLevelNames.begin(), it);
                  }
                  else
                  {
                     std::cerr << "Invalid log level: " << logLevelCmd.new_log_level << std::endl;
                  }
               }
               std::cout << "LOG LEVEL = " << LogLevelNames[log_level_idx] << std::endl;
            }
         }
      };
   }

   class LoggerGenerator
   {
      public:
         void writeData(const std::string& log_entry,
               const char* TagName, const char* LogLevelName)
         {
            using namespace std::chrono;
            auto now = system_clock::now();
            auto tt = system_clock::to_time_t(now);
            auto us = duration_cast<microseconds>(now.time_since_epoch());
            doWriteData(log_entry.c_str(), log_entry.size()+1, TagName, LogLevelName, tt, us % seconds{1});
         }
      private:
         virtual void doWriteData(const char* buf, std::size_t len,
               const char* TagName, const char* LogLevelName,
               std::time_t time_stamp,
               std::chrono::microseconds time_stamp_us) = 0;
   };

   class LoggerRetriever
   {
      public:
      virtual void getLog(boost::any& log_arg) = 0;
   };


   //Returned to user from the getXXLog() methods
   class LogGenProxy
   {
     public:
      LogGenProxy(LoggerGenerator& logger,
            bool is_enabled,
            const char* const tag_name,
            const char* const log_level_name)
         : theLogger(logger),
           enabled(is_enabled),
           TagName(tag_name),
           LogLevelName(log_level_name)
      {}

      template <typename T>
      LogGenProxy& operator<<(T const& value)
      {
         if(false == enabled)
            return *this;
         std::stringstream ss;
         ss << value;
         theLogger.writeData(ss.str(), TagName, LogLevelName);
         return *this;
      }

      void hexDump(const char* buf, std::size_t len, const std::string& label)
      {
         std::stringstream ss;
         ss << label << "\n";
         ss << std::setfill('0');
         for(std::size_t i = 0; i < len; ++i)
         {
            ss << std::hex << std::setw(2) << (int)buf[i];
            ss << (((i+1) % 32 == 0)? "\n" : " ");
         }
         theLogger.writeData(ss.str(), TagName, LogLevelName);
      }

     private:
      LoggerGenerator& theLogger;
      bool enabled;
      const char* const TagName;
      const char* const LogLevelName;
   };
}
