#pragma once
#include "stat_log/parsers/parser_common.h"
#include <boost/any.hpp>
#include <type_traits>

namespace stat_log
{

template <typename Tag, class Enable = void>
struct stat_tag_to_type;
/* Example:
   template <typename Tag>
   struct stat_tag_to_type;
   {
      using type = SimpleCounter<int>;
   };
*/

//TODO: Should the stat_log library provide a default
// stat type?

template <typename StatType>
struct is_serialization_deferred
{
   static constexpr bool value = false;
};


namespace detail
{

   template <typename Repr, typename WritePolicy>
   struct SimpleStat
   {
      using SharedType = Repr;
      static void write(void* shared_ptr, Repr value)
      {
         auto ptr = reinterpret_cast<Repr*>(shared_ptr);
         WritePolicy::write(ptr, value);
      }

      static void doCommand(void* shared_ptr, StatCmd cmd, boost::any& arg)
      {
         auto ptr = reinterpret_cast<Repr*>(shared_ptr);
         //TODO: handle all commands
         std::cout << std::dec << (unsigned long int)*ptr;
      }
   };

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

template <typename Repr, typename WritePolicy>
struct is_serialization_deferred<detail::SimpleStat<Repr, WritePolicy>>
{
   static constexpr bool value = false;
};

template <typename StatType,
   typename std::enable_if<
      true == is_serialization_deferred<StatType>::value
   >::type* = nullptr
>
void doSerializeStat(StatType& stat, void* ptr)
{
   stat.serialize(ptr);
}

template <typename StatType,
   typename = std::enable_if_t<
      false == is_serialization_deferred<StatType>::value
   >
>
void doSerializeStat(StatType& stat, void* ptr)
{
   //Do nothing if this stat does not require
   // deferred serialization.
}

template <typename StatType>
struct OperationalStatProxy : detail::StatProxyBase<StatType>
{
   template <typename... Args>
   void write(Args... args)
   {
      this->statHandler.write(this->shared_ptr, args...);
   }

   void serialize()
   {
      doSerializeStat(this->statHandler, this->shared_ptr);
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

}
