#pragma once
#include <boost/interprocess/mapped_region.hpp>
#include <string>

namespace stat_log
{
   class shared_mem_backend
   {
      public:
         void setParams(const std::string& name, size_t size, bool is_operational);
         char* getMemoryPtr();
      private:
         boost::interprocess::mapped_region region;
   };
}

