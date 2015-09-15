#include <stat_log/loggers/shared_memory_logger.h>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <cstring>
#include <fstream>
#include <tuple>

using namespace stat_log;
using std::size_t;
using std::chrono::microseconds;
using std::time_t;

#define CEIL_DIVIDE(a, b) ( ((a) + (b) - 1)/(b) )

namespace
{
   constexpr size_t bytes_per_log_buf = 512;
   constexpr size_t lock_size = sizeof(boost::interprocess::interprocess_mutex);

   using LogBufEntry = std::array<char, bytes_per_log_buf>;
   struct TimeStamp
   {
      time_t sec;
      microseconds usec;
   };

   bool operator>(const TimeStamp& rhs, const TimeStamp& lhs)
   {
      return std::make_tuple(rhs.sec, rhs.usec) > std::make_tuple(lhs.sec, lhs.usec);
   }


   struct LogHeaderCommon
   {
      //This is non-zero only for the first block
      // if the log entry takes up multiple log
      // buffer entries
      unsigned int numBlocks;
   };

   struct LogHeader
   {
      LogHeaderCommon commonHeader;
      TimeStamp timeStamp;
      std::array<char, 16> tagName;
      std::array<char, 8> logLevelName;
   };
   constexpr size_t max_chars_in_first_block = bytes_per_log_buf - sizeof(LogHeader);
   constexpr size_t max_chars_in_other_blocks = bytes_per_log_buf - sizeof(LogHeaderCommon);
}

shared_mem_logger_generator::shared_mem_logger_generator(
      const std::string& shm_name, size_t shm_size)
   : currentLogEntry(0),
     numLogEntries((shm_size - lock_size)/bytes_per_log_buf)
{
   using namespace boost::interprocess;
   shared_memory_object shm_obj
      (open_or_create
       ,shm_name.c_str()
       ,read_write                   //read-write mode
      );
   shm_obj.truncate(shm_size);
   region = mapped_region{shm_obj, read_write};
   mutex_ptr = new (region.get_address()) interprocess_mutex;
   shm_ptr = reinterpret_cast<char*>(region.get_address()) + lock_size;
   std::memset(shm_ptr, 0, shm_size - lock_size);
}

void
shared_mem_logger_generator::doWriteData(const char* buf, size_t len,
               const char* TagName, const char* LogLevelName,
               std::time_t time_stamp,
               std::chrono::microseconds time_stamp_us)
{
#if 0
   //NOTE: localtime is NOT threadsafe, so any calls to it will
   // only be done in "control" mode (the cout here is just for testing).
   char mbstr[100];
   std::strftime(mbstr, sizeof(mbstr), "%T", std::localtime(&time_stamp));
   std::cout << std::dec
      // << "Time = " << std::put_time(std::localtime(&time_stamp), "%T")
      << "Time = " << mbstr
      << "." << time_stamp_us.count()
      << ": BUFFER = " << buf << std::endl;
#endif

   //NOTE: the TagName, LogLevelName, and time_stamp fields
   // will also be written to shared memory, but will be
   // considered metadata.  The logger-retriever can then
   // use this metadata -- e.g. to filter out unwanted entries
   // or to not print the time stamp along, etc.
   using namespace boost::interprocess;
   scoped_lock<interprocess_mutex> lock(*mutex_ptr, try_to_lock);
   if(!lock)
   {
      //The control process must be in the process of reading
      // the log memory.  To avoid hanging the calling
      // process just return (and not log the entry).
      return;
   }

   const size_t num_log_bufs = 1 +
      (
         (len > max_chars_in_first_block)
         ? CEIL_DIVIDE(len-max_chars_in_first_block,
               max_chars_in_other_blocks)
         : 0
      );
   size_t start_log_idx;
   {
      std::unique_lock<std::mutex> lock(mtx);
      start_log_idx = currentLogEntry;
      currentLogEntry = (currentLogEntry + num_log_bufs) % numLogEntries;
   }
   size_t cur_log_idx = start_log_idx;

   const auto log_entry_start_ptr = reinterpret_cast<LogBufEntry*>(shm_ptr);
   auto log_entry_ptr = log_entry_start_ptr + cur_log_idx;
   auto log_hdr_ptr = reinterpret_cast<LogHeader*>(log_entry_ptr);

   log_hdr_ptr->commonHeader.numBlocks = num_log_bufs;
   log_hdr_ptr->timeStamp.sec = time_stamp;
   log_hdr_ptr->timeStamp.usec = time_stamp_us;
   std::strncpy(log_hdr_ptr->tagName.data(), TagName, log_hdr_ptr->tagName.size());
   std::strncpy(log_hdr_ptr->logLevelName.data(), LogLevelName,
         log_hdr_ptr->logLevelName.size());

   auto data_ptr = reinterpret_cast<char*>(log_hdr_ptr+1);
   auto bytes_to_write = std::min(len, max_chars_in_first_block);
   while(bytes_to_write > 0)
   {
      std::copy(buf, buf + bytes_to_write, data_ptr);
      len -= bytes_to_write;
      buf += bytes_to_write;
      bytes_to_write = std::min(len, max_chars_in_other_blocks);
      if(bytes_to_write > 0)
      {
         cur_log_idx = (cur_log_idx + 1) % numLogEntries;
         log_entry_ptr = log_entry_start_ptr + cur_log_idx;
         auto log_sub_hdr_ptr = reinterpret_cast<LogHeaderCommon*>(log_entry_ptr);
         log_sub_hdr_ptr->numBlocks = 0;
         data_ptr = reinterpret_cast<char*>(log_sub_hdr_ptr + 1);
      }
   }
}

shared_mem_logger_retriever::shared_mem_logger_retriever(
      const std::string& shm_name, size_t shm_size)
   :
     numLogEntries((shm_size - lock_size)/bytes_per_log_buf)
{
   using namespace boost::interprocess;
   try
   {
      shared_memory_object shm_obj
         ( open_only
           ,shm_name.c_str()
           ,read_write
         );
      region = mapped_region{shm_obj, read_write};
      mutex_ptr = reinterpret_cast<interprocess_mutex*>(region.get_address());
      shm_ptr = reinterpret_cast<const char*>(region.get_address()) + lock_size;
   }
   catch(boost::interprocess::interprocess_exception& e)
   {
      std::cerr << "Could not open logger shared memory = " << shm_name
         << ".  Is the generator program running?" << std::endl;
      std::exit(1);
   }
   catch(...)
   {
      std::cerr << "Unknown exception in shared_mem_logger_retriever ctor!\n";
      std::exit(1);
   }
}

void
shared_mem_logger_retriever::getLog(LogOutputCommand& log_params)
{
   using namespace boost::interprocess;
   std::ostream* output_ptr = &std::cout;
   std::fstream output_file;
   if(!log_params.output_filename.empty())
   {
      output_file.open(log_params.output_filename, std::fstream::out);
      output_ptr = &output_file;
   }
   const auto& exclude_tags = log_params.exclude_tags;
   const auto& exclude_log_levels = log_params.exclude_log_levels;
   auto& output = *output_ptr;
   bool show_tag = log_params.show_tag;
   bool show_time_stamp = log_params.show_time_stamp;
   bool show_log_level = log_params.show_log_level;

   //1. Copy the logging shm to a temporary buffer so we avoid possible
   //   race conditions (a lock is held _while_ we are doing the copying).
   //
   //2. Find the "beginning" of the log by searching the buffer for the
   //   smallest time stamp.
   //
   //3. Start with this first entry and output each entry to the provided "output"
   //   stream.
   //
   //4. The boolean "show_tag", "show_time_stamp" and "show_log_level" will
   //   be used to specify the level of verbosity when outputting the log entries.
   std::vector<char> temp_log_buf(region.get_size() - lock_size);
   {

      //Get the shared lock while we copy the log memory.
      scoped_lock<interprocess_mutex> lock(*mutex_ptr);
      //1:
      std::copy(shm_ptr, shm_ptr + region.get_size() - lock_size, temp_log_buf.data());
   }

   //2.
   bool found_valid_entry = false;
   auto log_entry_ptr = reinterpret_cast<const LogBufEntry*>(temp_log_buf.data());
   size_t first_log_entry = 0;
   TimeStamp firstTimeStamp;


   for(size_t i = 0; i < numLogEntries; ++i, ++log_entry_ptr)
   {
      auto log_hdr_ptr = reinterpret_cast<const LogHeader*>(log_entry_ptr);
      if(log_hdr_ptr->commonHeader.numBlocks > 0)
      {
         if(found_valid_entry == false ||
               firstTimeStamp > log_hdr_ptr->timeStamp)
         {
            firstTimeStamp = log_hdr_ptr->timeStamp;
            found_valid_entry = true;
            first_log_entry = i;
         }
      }
   }

   if(false == found_valid_entry)
   {
      output << "Empty log!" << std::endl;
      return;
   }

   //3.
   size_t cur_log_entry = first_log_entry;
   size_t num_log_entries_processed = 0;
   while(num_log_entries_processed < numLogEntries)
   {

      log_entry_ptr = reinterpret_cast<const LogBufEntry*>(temp_log_buf.data()) + cur_log_entry;
      auto log_hdr_ptr = reinterpret_cast<const LogHeader*>(log_entry_ptr);

      cur_log_entry = (cur_log_entry + 1) % numLogEntries;
      ++num_log_entries_processed;
      if(log_hdr_ptr->commonHeader.numBlocks == 0)
         continue;

      bool should_print_entry = true;
      auto found_string_in_list = [&](const auto & string_list, const auto& string_to_find)
      {
          auto it = std::find_if(string_list.cbegin(), string_list.cend(),
                  [&](const auto& str)
                  {
                     return str == string_to_find;
                  });

          return it != string_list.cend();
      };

      if(exclude_tags.empty() == false)
      {
         should_print_entry =
            !found_string_in_list(exclude_tags, log_hdr_ptr->tagName.data());
      }
      if(should_print_entry && exclude_log_levels.empty() == false)
      {
         should_print_entry =
            !found_string_in_list(exclude_log_levels, log_hdr_ptr->logLevelName.data());
      }
      //4
      if(show_time_stamp)
      {
         const auto& time_stamp_sec = log_hdr_ptr->timeStamp.sec;
         const auto& time_stamp_us = log_hdr_ptr->timeStamp.usec;
         char mbstr[100];
         auto loc_time_ptr = std::localtime(&time_stamp_sec);
         if(loc_time_ptr == nullptr)
         {
            std::cerr << "Invalid time stamp!\n";
            std::exit(1);
         }
         std::strftime(mbstr, sizeof(mbstr), "%T", loc_time_ptr);
         if(should_print_entry)
            output << std::dec
               << mbstr
               //std::put_time does not exist in GCC :(
               // << "Time = " << std::put_time(std::localtime(&time_stamp), "%T")
               << "." << time_stamp_us.count()
               << ": ";
      }
      if(show_tag)
      {
         if(should_print_entry)
            output << log_hdr_ptr->tagName.data() << ": ";
      }
      if(show_log_level)
      {
         if(should_print_entry)
            output << log_hdr_ptr->logLevelName.data() << ": ";
      }

      auto data_ptr = reinterpret_cast<const char*>(log_hdr_ptr+1);
      if(should_print_entry)
         output << data_ptr;
      for(unsigned int i = 0; i < log_hdr_ptr->commonHeader.numBlocks - 1; ++i)
      {
         log_entry_ptr = reinterpret_cast<const LogBufEntry*>(temp_log_buf.data()) + cur_log_entry;
         auto log_sub_hdr_ptr = reinterpret_cast<const LogHeaderCommon*>(log_entry_ptr);
         data_ptr = reinterpret_cast<const char*>(log_sub_hdr_ptr + 1);
         if(should_print_entry)
            output << data_ptr;
         ++num_log_entries_processed;
         cur_log_entry = (cur_log_entry + 1) % numLogEntries;
      }
      if(should_print_entry)
         output << std::endl;
   }
   if(output_file.is_open())
      output_file.close();
}
