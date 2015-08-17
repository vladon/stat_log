#pragma once
#include "stat_log/parsers/parser_common.h"
#include <boost/any.hpp>

namespace stat_log
{
namespace detail
{
   template <typename StatType>
   struct StatProxyBase : StatType
   {
      using SharedType = typename StatType::SharedType;
      SharedType* shared_ptr = nullptr;
      void setSharedPtr(void* ptr)
      {
         shared_ptr = reinterpret_cast<SharedType*>(ptr);
      }
      StatType statHandler;

      static constexpr size_t getSharedSize()
      {
         return sizeof(SharedType);
      }
   };
}

template <typename StatType>
struct OperationalStatProxy : detail::StatProxyBase<StatType>
{
   template <typename... Args>
   void write(Args... args)
   {
      this->statHandler.write(this->shared_ptr, args...);
   }
};


template <typename StatType>
struct ControlStatProxy : detail::StatProxyBase<StatType>
{
   void doStatCommand(StatCmd cmd, boost::any& cmd_arg)
   {
      this->doCommand(this->shared_ptr, cmd, cmd_arg);
   }
};

template <typename Tag, class Enable = void>
struct stat_traits;

   /* Example:
      template <typename Tag>
      struct stat_traits;
      {
         using StatType = SimpleCounter<int>;
      };
   */

   //TODO: Should the stat_log library provide a default
   // stat trait?
}
