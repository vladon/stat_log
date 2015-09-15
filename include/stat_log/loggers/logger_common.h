#pragma once
#include <stat_log/stats/stats_common.h>
#include <stat_log/util/command.h>
#include <boost/any.hpp>
#include <iomanip>
#include <chrono>
#include <array>
#include <ostream>
#include <iostream>
#include <sstream>
#include <assert.h>

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
         using shared_type = LogControlStorage;
         shared_type* shared_ptr = nullptr;
         void setSharedPtr(void* ptr)
         {
            shared_ptr = reinterpret_cast<shared_type*>(ptr);
         }

         unsigned char getLevel(int log_idx)
         {
            assert(log_idx <= (int)shared_type{}.size());
            return (*shared_ptr)[log_idx];
         }

         static constexpr size_t getSharedSize()
         {
            return sizeof(shared_type);
         }
      };

      struct LogControlProxy
      {
         using shared_type = LogControlStorage;
         shared_type* shared_ptr = nullptr;
         void setSharedPtr(void* ptr)
         {
            shared_ptr = reinterpret_cast<shared_type*>(ptr);
         }

         static constexpr size_t getSharedSize()
         {
            return sizeof(shared_type);
         }

         void doCommand(StatCmd cmd, boost::any& cmd_arg, std::string& log_output)
         {
            std::stringstream ss;
            if(false == isLogCommand(cmd))
               return;
            if(cmd == StatCmd::LOG_LEVEL)
            {
               auto logLevelCmd = boost::any_cast<LogLevelCommand>(cmd_arg);
               int log_idx = logLevelCmd.logger_idx;
               if(log_idx >= (int)shared_type{}.size())
               {
                  std::cerr << "Invalid log index = " << log_idx << std::endl;
                  std::exit(1);
               }
               auto& log_level_idx = (*shared_ptr)[log_idx];
               if(logLevelCmd.set_log_level)
               {
                  ss << "previous log level = " << LogLevelNames[log_level_idx] << ", ";
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
                  ss << "new log level = " << LogLevelNames[log_level_idx];
               }
               else
               {
                  ss << "log level = " << LogLevelNames[log_level_idx];
               }
               log_output = ss.str();
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

         virtual ~LoggerGenerator(){}
      private:
         virtual void doWriteData(const char* buf, std::size_t len,
               const char* TagName, const char* LogLevelName,
               std::time_t time_stamp,
               std::chrono::microseconds time_stamp_us) = 0;
   };

   class LoggerRetriever
   {
      public:
      virtual void getLog(LogOutputCommand&) = 0;
      virtual ~LoggerRetriever(){}
   };


   //Returned to user from the getXXLog() methods
   class LogGenProxy
   {
     public:
      LogGenProxy(LoggerGenerator& logger,
            bool is_enabled,
            const char* const tag_name,
            const char* const log_level_name)
         : ss_ptr(std::make_unique<std::stringstream>()),
           theLogger(&logger),
           enabled(is_enabled),
           TagName(tag_name),
           LogLevelName(log_level_name)
      {}

      LogGenProxy(LogGenProxy&& other) = default;

      ~LogGenProxy()
      {
         if(!ss_ptr->str().empty())
            theLogger->writeData(ss_ptr->str(), TagName, LogLevelName);
      }

      template <typename T>
      LogGenProxy& operator<<(T const& value)
      {
         if(false == enabled)
            return *this;
         *ss_ptr << value;
         return *this;
      }

      void hexDump(const unsigned char* buf, std::size_t len, const std::string& label)
      {
         if(false == enabled)
            return;
         *ss_ptr << label << "\n";
         *ss_ptr << std::setfill('0');
         for(std::size_t i = 0; i < len; ++i)
         {
            *ss_ptr << std::hex << std::setw(2) << (unsigned int)buf[i];
            *ss_ptr << (((i+1) % 32 == 0)? "\n" : " ");
         }
      }

     private:
      //Looks like a gcc bug (supposedly fixed in 5.0)
      // makes this not compile if we don't wrap it with
      // a unique_ptr
      std::unique_ptr<std::stringstream> ss_ptr;
      LoggerGenerator* theLogger;
      bool enabled;
      const char* const TagName;
      const char* const LogLevelName;
   };
}
