#pragma once
#include <stat_log/util/command.h>
#include <stat_log/util/utils.h>
#include <boost/any.hpp>
#include <type_traits>
#include <vector>
#include <string>
#include <sstream>

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
   //Some StatTypes may have different implementations based on whether we are
   //in "operational" mode or not.  I provide this trait to allow the specific
   //stat types to switch their implementations accordingly.
   //NOTE: the default trait specifies that the implementation is the same as
   //the StatType itself.
   template <typename StatType, bool IsOperational>
   struct stat_type_to_impl
   {
      using type = StatType;
   };


   template <typename Repr, typename WritePolicy>
   struct SimpleStat
   {
      using shared_type = Repr;
      using sample_type = shared_type;
      static void write(void* shared_ptr, sample_type value)
      {
         auto ptr = reinterpret_cast<shared_type*>(shared_ptr);
         WritePolicy::write(ptr, value);
      }

      static void doStatCommand(
            void* shared_ptr,
            StatCmd cmd,
            boost::any& arg,
            StatCmdOutput& stat_output)
      {
         auto ptr = reinterpret_cast<shared_type*>(shared_ptr);
         //TODO: handle all commands
         auto& val = *ptr;
         if(cmd == StatCmd::DUMP_STAT)
         {
            std::stringstream ss;
            ss << (unsigned long int)val;
            stat_output.entries.push_back(ss.str());
         }
         else if(cmd == StatCmd::PRINT_STAT_TYPE)
         {
            stat_output.entryTitle = "Simple Stat";
         }
         else if(cmd == StatCmd::CLEAR_STAT)
         {
            val = 0;
         }
      }
   };

   template <typename StatType, bool IsOperational>
   struct StatProxyBase : stat_type_to_impl<StatType, IsOperational>::type
   {
      using StatImpl =
         typename stat_type_to_impl<StatType, IsOperational>::type;
      using shared_type = typename StatImpl::shared_type;
      shared_type* shared_ptr = nullptr;
      void setSharedPtr(void* ptr)
      {
         shared_ptr = reinterpret_cast<shared_type*>(ptr);
      }

      static constexpr size_t getSharedSize()
      {
         return sizeof(shared_type);
      }
   };
}

template <typename Repr, typename WritePolicy>
struct is_serialization_deferred<detail::SimpleStat<Repr, WritePolicy>>
{
   static constexpr bool value = false;
};

template <typename StatType>
void doSerializeStat(StatType& stat, void* ptr, std::true_type)
{
   stat.serialize(ptr);
}

template <typename StatType>
void doSerializeStat(StatType& stat, void* ptr, std::false_type)
{
   //Do nothing if this stat does not require
   // deferred serialization.
}

template <typename StatType>
struct OperationalStatProxy : detail::StatProxyBase<StatType, true>
{
   template <typename... Args>
   void writeVal(Args... args)
   {
      this->write(this->shared_ptr, args...);
   }

   void serialize()
   {
      using StatImpl = typename detail::StatProxyBase<StatType, true>::StatImpl;
      doSerializeStat(static_cast<StatImpl&>(*this), this->shared_ptr,
            std::integral_constant<bool, is_serialization_deferred<StatType>::value>{});
   }
};


template <typename StatType>
struct ControlStatProxy : detail::StatProxyBase<StatType, false>
{
   void doCommand(StatCmd cmd, boost::any& cmd_arg, StatCmdOutput& stat_output)
   {
      if(false == isStatisticCommand(cmd))
         return;
      this->doStatCommand(this->shared_ptr, cmd, cmd_arg, stat_output);
   }
};

}
