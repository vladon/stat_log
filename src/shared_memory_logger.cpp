#include <stat_log/loggers/shared_memory_logger.h>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <cstring>
#include <fstream>

using namespace stat_log;
using std::size_t;
using std::chrono::microseconds;
using std::time_t;

namespace
{
   constexpr size_t bytes_per_log_buf = 512;
   constexpr size_t lock_size = sizeof(boost::interprocess::interprocess_mutex);

   using LogBufEntry = std::array<char, bytes_per_log_buf>;
   using TimeStampTuple = std::tuple<time_t, microseconds>;

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
      TimeStampTuple timeStamp;
      std::array<char, 16> tagName;
      std::array<char, 8> logLevelName;
   };
}

shared_mem_logger_generator::shared_mem_logger_generator(
      const std::string& shm_name, size_t shm_size)
   : currentLogEntry(0),
     numLogEntries(shm_size/bytes_per_log_buf)
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

   const size_t num_log_bufs
      = (len + bytes_per_log_buf - 1)/bytes_per_log_buf;
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
   std::get<0>(log_hdr_ptr->timeStamp) = time_stamp;
   std::get<1>(log_hdr_ptr->timeStamp) = time_stamp_us;
   std::strncpy(log_hdr_ptr->tagName.data(), TagName, log_hdr_ptr->tagName.size());
   std::strncpy(log_hdr_ptr->logLevelName.data(), LogLevelName,
         log_hdr_ptr->logLevelName.size());

   auto data_ptr = reinterpret_cast<char*>(log_hdr_ptr+1);
   auto bytes_to_write = std::min(len, bytes_per_log_buf-sizeof(LogHeader));
   while(bytes_to_write > 0)
   {
      std::strncpy(data_ptr, buf, bytes_to_write);
      len -= bytes_to_write;
      buf += bytes_to_write;
      bytes_to_write = std::min(len, bytes_per_log_buf-sizeof(LogHeaderCommon));
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
   shared_memory_object shm_obj
      ( open_only
       ,shm_name.c_str()
       ,read_write
      );
   region = mapped_region{shm_obj, read_write};
   mutex_ptr = reinterpret_cast<interprocess_mutex*>(region.get_address());
   shm_ptr = reinterpret_cast<const char*>(region.get_address()) + lock_size;
}

void
shared_mem_logger_retriever::getLog(boost::any& log_args)
{
   using namespace boost::interprocess;
   auto log_params = boost::any_cast<LogOutputCommand>(log_args);
   std::ostream* output_ptr = &std::cout;
   std::fstream output_file;
   if(!log_params.output_filename.empty())
   {
      output_file.open(log_params.output_filename, std::fstream::out);
      output_ptr = &output_file;
   }
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
   std::vector<char> temp_log_buf(region.get_size());
   {

      //Get the shared lock while we copy the log memory.
      scoped_lock<interprocess_mutex> lock(*mutex_ptr);
      //1:
      std::copy(shm_ptr, shm_ptr + region.get_size(), temp_log_buf.data());
   }

   //2.
   bool found_valid_entry = false;
   auto log_entry_ptr = reinterpret_cast<const LogBufEntry*>(shm_ptr);
   size_t first_log_entry = 0;
   TimeStampTuple firstTimeStamp;


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

      log_entry_ptr = reinterpret_cast<const LogBufEntry*>(shm_ptr) + cur_log_entry;
      auto log_hdr_ptr = reinterpret_cast<const LogHeader*>(log_entry_ptr);

      cur_log_entry = (cur_log_entry + 1) % numLogEntries;
      ++num_log_entries_processed;
      if(log_hdr_ptr->commonHeader.numBlocks == 0)
         continue;

      //4
      if(show_time_stamp)
      {
         auto& time_stamp_sec = std::get<0>(log_hdr_ptr->timeStamp);
         auto& time_stamp_us = std::get<1>(log_hdr_ptr->timeStamp);
         char mbstr[100];
         //TODO: address sanitize sometimes flags this line as an invalid
         // memory access.  I believe it is because of a race condition
         // between the operational and control applications.  We probably
         // need a semaphore to fix ...
         std::strftime(mbstr, sizeof(mbstr), "%T", std::localtime(&time_stamp_sec));
         output << std::dec
            << mbstr
            //std::put_time does not exist in GCC :(
            // << "Time = " << std::put_time(std::localtime(&time_stamp), "%T")
            << "." << time_stamp_us.count()
            << ": ";
      }
      if(show_tag)
      {
         output << log_hdr_ptr->tagName.data() << ": ";
      }
      if(show_log_level)
      {
         output << log_hdr_ptr->logLevelName.data() << ": ";
      }
      auto data_ptr = reinterpret_cast<const char*>(log_hdr_ptr+1);
      output << data_ptr;
      for(unsigned int i = 0; i < log_hdr_ptr->commonHeader.numBlocks - 1; ++i)
      {
         log_entry_ptr += 1;
         auto log_sub_hdr_ptr = reinterpret_cast<const LogHeaderCommon*>(log_entry_ptr);
         data_ptr = reinterpret_cast<const char*>(log_sub_hdr_ptr + 1);
         output << data_ptr;
         ++num_log_entries_processed;
      }
      output << std::endl;
   }
   if(output_file.is_open())
      output_file.close();
}
