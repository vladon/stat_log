#pragma once
#include "stat_log/stats/stats_common.h"
#include "stat_log/parsers/parser_common.h"
#include <iomanip>
#include <chrono>
#include <array>
#include <ostream>

namespace stat_log
{

   namespace detail
   {
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
            assert(log_idx <= SharedType{}.size());
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
               if(logLevelCmd.set_log_level)
               {
                  assert(log_idx <= SharedType{}.size());
                  std::cout << "SETTING LOG LEVEL to " << logLevelCmd.new_log_level;
                  //TODO: convert new_log_level to a number ...
                  (*shared_ptr)[log_idx] = 0;
               }
               else
               {
                  std::cout << "LOG LEVEL = " << (int)(*shared_ptr)[log_idx] << std::endl;
               }
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
      virtual void getLog(
            std::ostream&& output,
            bool show_tag,
            bool show_time_stamp,
            bool show_log_level) = 0;
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
