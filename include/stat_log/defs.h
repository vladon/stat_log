#pragma once

#define SL_NAME static constexpr const char* name

namespace stat_log
{
   //void_t is broken on gcc :(
   // so I will use "AlwaysVoid" instead.
   // template <class...> using void_t = void;
   template <typename U> struct AlwaysVoid {
      typedef void type;
   };


   template <typename T>
   struct Identity
   {
      using type = T;
   };
}


//Useful for printing types (for debugging)
#if 0
#include <cxxabi.h>
#include <iostream>
template <typename T>
struct TypeId { };

   template<typename T>
inline std::ostream& operator<<(std::ostream& out, TypeId<T> typeId)
{
   int status;
   auto name = typeid(T).name();
   out << abi::__cxa_demangle(name, 0,0, & status);
   return out;
}
#endif
