//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

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

