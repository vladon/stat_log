#pragma once
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <iostream>
#include <thread>
#include <chrono>

namespace stat_log
{
   class shared_mem_backend
   {
      public:
         void setParams(const std::string& name, size_t size, bool is_operational)
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

         char* getMemoryPtr()
         {
            return (char*)region.get_address();
         }

      private:
         boost::interprocess::mapped_region region;

   };
}

