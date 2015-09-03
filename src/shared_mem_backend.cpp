#include <stat_log/backends/shared_mem_backend.h>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <iostream>

namespace stat_log
{
   using std::cerr;

   void shared_mem_backend::setParams(
         const std::string& name, size_t size, bool is_operational)
   {
      try
      {
         using namespace boost::interprocess;
         shared_memory_object shm_obj
            (open_or_create
             ,name.c_str()
             ,read_write                   //read-write mode
            );
         if(is_operational)
            shm_obj.truncate(size);
         region = mapped_region{shm_obj, read_write};
         if(is_operational)
            std::memset(region.get_address(), 0, size);
      }
      catch(boost::interprocess::interprocess_exception& e)
      {
         cerr << __func__ << ": Could not open logger shared memory = " << name;
         if(!is_operational)
           cerr << ".  Is the generator program running?";
         cerr << std::endl;
         std::exit(1);
      }
      catch(...)
      {
         cerr << "Unknown exception in shared_mem_backend ctor!\n";
         std::exit(1);
      }
   }

   char* shared_mem_backend::getMemoryPtr()
   {
      return (char*)region.get_address();
   }
}
