#pragma once
#include "stat_log/stats/simple_counter.h"
#include "stat_log/defs.h"
#include <array>
#include <boost/any.hpp>
#include <type_traits>
#include <vector>
#include <string>

namespace stat_log
{
   namespace stat_array_detail
   {
      template <typename StatType, class Enable = void>
      struct traits
      {
         using stat_type = StatType;
         using ShmType = typename StatType::SharedType;

         static void write(void* shared_ptr, ShmType value)
         {
            stat_type::write(shared_ptr, value);
         }

         static void doStatCommand(void* shared_ptr, StatCmd cmd, boost::any& arg,
            const std::vector<std::string>& enumNames,
            const std::vector<std::string>& dimensionNames,
            int dimension_idx)
         {
            stat_type::doStatCommand(shared_ptr, cmd, arg,
                  enumNames, dimensionNames, dimension_idx);
         }
      };

      //For user convenience, we allow the user pass in fundamental types:
      //   StatArray<10, int>
      //If this happens, we simply wrap the "int" in a SimpleCounter type
      template <typename StatType>
      struct traits<StatType,
               typename std::enable_if<std::is_fundamental<StatType>::value>::type
         >
      {
         using ShmType = StatType;
         using stat_type = SimpleCounter<StatType>;

         static void write(void* shared_ptr, ShmType value)
         {
            traits<stat_type>::write(shared_ptr, value);
         }

         static void doStatCommand(void* shared_ptr, StatCmd cmd, boost::any& arg,
            const std::vector<std::string>& enumNames,
            const std::vector<std::string>& dimensionNames,
            int dimension_idx)
         {
            traits<stat_type>::doStatCommand(shared_ptr, cmd, arg,
                  enumNames, dimensionNames, dimension_idx);
         }
      };
   }

   template <size_t Size, typename Repr>
   struct StatArray
   {
      using ShmType = typename stat_array_detail::traits<Repr>::ShmType;
      using SharedType = std::array<ShmType, Size>;
      //The actual statistic write :)
      static void write_idx(void* shared_ptr, int idx, ShmType value)
      {
         auto ptr = reinterpret_cast<ShmType*>(shared_ptr) + idx;
         stat_array_detail::traits<Repr>::write(ptr, value);
      }

      //We "eat" the superfluous indices here:
      template <typename ...Args>
      static void write_idx (void* shared_ptr, int idx, int idx_ignore, Args... args)
      {
         write_idx(shared_ptr, idx, args...);
      }

      //If passed more than one index, we only look at the first and ignore the rest.
      template <typename ...Args>
      static void write(void* shared_ptr, int idx, Args... args)
      {
         write_idx(shared_ptr, idx, args...);
      }

      static void doStatCommand(void* shared_ptr, StatCmd cmd, boost::any& arg,
            const std::vector<std::string>& enumNames,
            const std::vector<std::string>& dimensionNames,
            int dimension_idx)
      {
         auto& theArray = *reinterpret_cast<SharedType*>(shared_ptr);
         if(dimension_idx == 0 && !dimensionNames.empty())
            std::cout << dimensionNames[0] << "\n";
         //TODO: handle all commands
         if(printingRequired(cmd, false))
         {
            std::cout << "STAT_ARRAY: " << std::endl;
            std::cout << std::dec << "[";
         }
         for(size_t i = 0; i < Size; ++i)
         {
            stat_array_detail::traits<Repr>::doStatCommand((void*)&theArray[i], cmd, arg,
                  enumNames, dimensionNames, dimension_idx+1);
            if(printingRequired(cmd, false))
            {
               if(i < Size - 1)
                  std::cout << ", ";
               else
                  std::cout << "]";
            }
         }
      }
   };

   //This specialization handles the case where we embed StatArrays
   template <size_t Size, typename Repr, size_t N>
   struct StatArray<Size, StatArray<N, Repr>>
   {
      using SharedType = std::array<
         typename StatArray<N,Repr>::SharedType, Size>;

      template <typename ...Args>
      static void write(void* shared_ptr, int idx, Args... args)
      {
         auto& theArray = *reinterpret_cast<SharedType*>(shared_ptr);
         auto child_ptr = reinterpret_cast<void*>(&theArray[idx]);
         StatArray<N,Repr>::write(child_ptr, args...);
      }

      static void doStatCommand(void* shared_ptr, StatCmd cmd, boost::any& arg,
            const std::vector<std::string>& enumNames,
            const std::vector<std::string>& dimensionNames,
            int dimension_idx)
      {
         auto& theArray = *reinterpret_cast<SharedType*>(shared_ptr);
         //TODO: handle all commands
         if(printingRequired(cmd, false))
         {
            if(cmd == StatCmd::PRINT_STAT_TYPE)
            {
               std::cout << "STAT_ARRAY" << std::endl;
            }
            if(dimension_idx == 0 && dimensionNames.size() == 2)
            {
               std::cout << "X = " << dimensionNames[0]
                  << ", Y = " << dimensionNames[1] << std::endl;
            }
            std::cout << std::dec << "[";
         }
         for(size_t i = 0; i < Size; ++i)
         {
            auto child_ptr = reinterpret_cast<void*>(&theArray[i]);
            StatArray<N,Repr>::doStatCommand(child_ptr, cmd, arg,
                  enumNames, dimensionNames, dimension_idx+1);
            if(printingRequired(cmd, false))
            {
               if(i < Size - 1)
                  std::cout << ", ";
               else
                  std::cout << "]";
            }
         }
      }
   };


   //The StatArray has deferred serialization only if its
   // underlying representation needs it.
   template <size_t Size, typename Repr>
   struct is_serialization_deferred<StatArray<Size, Repr>>
   {
      static constexpr bool value = is_serialization_deferred<Repr>::value;
   };

   template <int Size, typename Repr>
   struct num_stat_dimensions<StatArray<Size, Repr>>
   {
         static constexpr int value = 1;
   };

   template <int Size, typename Repr, int N>
   struct num_stat_dimensions<StatArray<Size, StatArray<N, Repr>>>
   {
         static constexpr int value = 1 + num_stat_dimensions<StatArray<N,Repr>>::value;
   };
}
