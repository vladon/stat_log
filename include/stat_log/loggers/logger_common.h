#pragma once
#include "stat_log/stats/stats_common.h"
namespace stat_log
{
   using LogControlWord = uint32_t;
   template <typename SharedType>
   struct LogProxy
   {
      SharedType* shared_ptr = nullptr;
      void setSharedPtr(void* ptr)
      {
         shared_ptr = reinterpret_cast<SharedType*>(ptr);
      }

      void setLevel(SharedType newLevel)
      {
         *shared_ptr = newLevel;
      }

      SharedType getLevel()
      {
         return *shared_ptr;
      }

      static constexpr size_t getSharedSize()
      {
         return sizeof(SharedType);
      }
   };
}
