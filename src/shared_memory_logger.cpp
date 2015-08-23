#include "stat_log/loggers/shared_memory_logger.h"

using namespace stat_log;

shared_mem_logger_generator::shared_mem_logger_generator(
      const std::string& shm_name, std::size_t shm_size)
{
   using namespace boost::interprocess;
   shared_memory_object shm_obj
      (open_or_create
       ,shm_name.c_str()
       ,read_write                   //read-write mode
      );
   shm_obj.truncate(shm_size);
   region = mapped_region{shm_obj, read_write};
   std::memset(region.get_address(), 0, shm_size);
}

void
shared_mem_logger_generator::doWriteData(const char* buf, std::size_t len,
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

shared_mem_logger_retriever::shared_mem_logger_retriever(
      const std::string& shm_name, std::size_t shm_size)
{
   using namespace boost::interprocess;
   shared_memory_object shm_obj
      (open_or_create
       ,shm_name.c_str()
       ,read_write                   //read-write mode
      );
   shm_obj.truncate(shm_size);
   region = mapped_region{shm_obj, read_write};
   std::memset(region.get_address(), 0, shm_size);
}

void
shared_mem_logger_retriever::getLog(
               std::ostream&& output,
               bool show_tag,
               bool show_time_stamp,
               bool show_log_level)
{
   //TODO:
   //
   //1. Copy the logging shm to a temporary buffer to avoid a possible race
   //   condition (note: this may not be enough to avoid all race conditions
   //   between the generator and controlling processes. Consider additional
   //   inter-process semaphores to mitigate).
   //
   //2. Find the "beginning" of the log by searching the buffer for the
   //   smallest time stamp.
   //
   //3. Start with this first entry and output each entry to the provided "output"
   //   stream.
   //
   //4. The boolean "show_tag", "show_time_stamp" and "show_log_level" will
   //   be used to specify the level of verbosity when outputting the log entries.
}
