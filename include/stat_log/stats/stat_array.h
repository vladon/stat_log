//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

#pragma once
#include <stat_log/stats/simple_counter.h>
#include <stat_log/stats/stats_common.h>
#include <stat_log/defs.h>
#include <array>
#include <boost/any.hpp>
#include <type_traits>
#include <vector>
#include <string>
#include <assert.h>

namespace stat_log
{
template <size_t Size, typename Repr>
struct StatArray
{
};
namespace stat_array_detail
{
   template <typename StatType, bool IsOperational, class Enable = void>
      struct traits
      {
         using stat_type = typename detail::stat_type_to_impl<StatType, IsOperational>::type;

         template <typename T>
         static void write(void* ptr, stat_type& stat, T value)
         {
            stat.write(ptr, value);
         }

      static void doStatCommand(void* ptr,
         stat_type& stat,
         StatCmd cmd,
         boost::any& arg,
         StatCmdOutput& stat_output)
      {
         stat.doStatCommand(ptr, cmd, arg, stat_output);
      }
   };

   //For user convenience, we allow the user pass in fundamental types:
   //   StatArray<10, int>
   //If this happens, we simply wrap the "int" in a SimpleCounter type
   template <typename StatType, bool IsOperational>
   struct traits<StatType, IsOperational,
            typename std::enable_if<std::is_fundamental<StatType>::value>::type
      >
   {
      using stat_type = SimpleCounter<StatType>;

      template <typename T>
      static void write(void* ptr, stat_type& stat, T value)
      {
         traits<stat_type, IsOperational>::write(ptr, stat, value);
      }

      static void doStatCommand(void* ptr, stat_type& stat,
            StatCmd cmd, boost::any& arg, StatCmdOutput& stat_output)
      {
         traits<stat_type, IsOperational>::doStatCommand(ptr,
               stat, cmd, arg, stat_output);
      }
   };

   template <size_t Size, typename Repr, bool IsOperational>
   struct StatArrayImpl
   {
      using TheTraits = typename stat_array_detail::traits<Repr, IsOperational>;
      using stat_type = typename TheTraits::stat_type;
      using StatEntries = std::array<stat_type, Size>;
      using PerEntrySharedType = typename stat_type::shared_type;
      using shared_type = std::array<PerEntrySharedType, Size>;
      using sample_type = typename stat_type::sample_type;

      StatEntries statEntries;

      //The actual statistic write :)
      void write_idx(void* ptr, int idx, sample_type value)
      {
         auto& shared_array = *reinterpret_cast<shared_type*>(ptr);
         auto this_ptr = reinterpret_cast<void*>(&shared_array[idx]);
         assert(idx >= 0 && idx < (int)Size);
         statEntries[idx].write(this_ptr, value);
      }

      //We "eat" the superfluous indices here:
      template <typename ...Args>
      void write_idx (void* ptr, int idx, int idx_ignore, Args... args)
      {
         write_idx(ptr, idx, args...);
      }

      //If passed more than one index, we only look at the first and ignore the rest.
      template <typename ...Args>
      void write(void* ptr, int idx, Args... args)
      {
         write_idx(ptr, idx, args...);
      }

      //Called via the deferred processing thread
      void serialize(void* ptr)
      {
         auto& shared_array = *reinterpret_cast<shared_type*>(ptr);
         for(size_t i = 0; i < Size; ++i)
         {
            statEntries[i].serialize((void*)&shared_array[i]);
         }
      }

      void doStatCommand(void* ptr, StatCmd cmd,
            boost::any& arg, StatCmdOutput& stat_output)
      {
         auto& shared_array = *reinterpret_cast<shared_type*>(ptr);
         stat_output.dimensionSizes.push_back(Size);
         if(cmd == StatCmd::PRINT_STAT_TYPE)
         {
            std::stringstream ss;
            statEntries[0].doStatCommand(ptr, cmd, arg, stat_output);
            ss << "STAT_ARRAY [" <<  stat_output.entryTitle
               << "] size = " << Size;
            stat_output.entryTitle = ss.str();
            return;
         }

         for(size_t i = 0; i < Size; ++i)
         {
            TheTraits::doStatCommand(
                  (void*)&shared_array[i],
                  statEntries[i],
                  cmd, arg, stat_output);
         }
      }
   };

   //This specialization handles the case where we embed StatArrays
   template <size_t Size, size_t M, typename Repr, bool IsOperational>
   struct StatArrayImpl<Size, StatArrayImpl<M, Repr, IsOperational>, IsOperational>
   {
      using EmbeddedStatArray = StatArrayImpl<M,Repr,IsOperational>;
      using shared_type = std::array<typename EmbeddedStatArray::shared_type, Size>;

      std::array<EmbeddedStatArray, Size> statEntries;

      template <typename ...Args>
      void write(void* ptr, int idx, Args... args)
      {
         auto& theArray = *reinterpret_cast<shared_type*>(ptr);
         auto child_ptr = reinterpret_cast<void*>(&theArray[idx]);
         assert(idx >= 0 && idx < (int)Size);
         statEntries[idx].write(child_ptr, args...);
      }

      //Called via the deferred processing thread
      void serialize(void* ptr)
      {
         auto& shared_array = *reinterpret_cast<shared_type*>(ptr);
         for(size_t i = 0; i < Size; ++i)
         {
            statEntries[i].serialize((void*)&shared_array[i]);
         }
      }

      void doStatCommand(void* ptr, StatCmd cmd,
            boost::any& arg, StatCmdOutput& stat_output)
      {
         auto& theArray = *reinterpret_cast<shared_type*>(ptr);
         stat_output.dimensionSizes.push_back(Size);
         if(cmd == StatCmd::PRINT_STAT_TYPE)
         {
            std::stringstream ss;
            statEntries[0].doStatCommand(ptr, cmd, arg, stat_output);
            ss << "STAT_ARRAY [" <<  stat_output.entryTitle
               << "] size = " << Size;
            stat_output.entryTitle = ss.str();
            return;
         }
         auto dimSizes = stat_output.dimensionSizes;
         for(size_t i = 0; i < Size; ++i)
         {
            auto child_ptr = reinterpret_cast<void*>(&theArray[i]);
            statEntries[i].doStatCommand(child_ptr, cmd, arg, stat_output);
            if(i == 0)
               dimSizes = stat_output.dimensionSizes;
            stat_output.dimensionSizes.clear();
         }
         stat_output.dimensionSizes = dimSizes;
      }
   };
}

namespace detail{
   template <size_t N, typename T>
   struct stat_type_to_impl<StatArray<N, T>, true>
   {
      using type = stat_array_detail::StatArrayImpl<N,T, true>;
   };

   template <size_t N, size_t M, typename T>
   struct stat_type_to_impl<StatArray<N, StatArray<M, T>>, true>
   {
      using type = stat_array_detail::StatArrayImpl<N,
            stat_array_detail::StatArrayImpl<M,
               typename stat_type_to_impl<T, true>::type,
               true>,
            true>;
   };

   template <size_t N, typename T>
   struct stat_type_to_impl<StatArray<N, T>, false>
   {
      using type = stat_array_detail::StatArrayImpl<N,T, false>;
   };

   template <size_t N, size_t M, typename T>
   struct stat_type_to_impl<StatArray<N, StatArray<M, T>>, false>
   {
      using type = stat_array_detail::StatArrayImpl<N,
            stat_array_detail::StatArrayImpl<M,
               typename stat_type_to_impl<T, false>::type,
               false>,
            false>;
   };
}

//The StatArray has deferred serialization only if its
// underlying representation needs it.
template <size_t Size, typename Repr>
struct is_serialization_deferred<StatArray<Size, Repr>>
{
   static constexpr bool value = is_serialization_deferred<Repr>::value;
};
}
