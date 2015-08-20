#pragma once

//TODO: rename this to something that is less likely
// to collide with user code
#define NAME static constexpr const char* name

namespace stat_log
{
   template <typename U> struct AlwaysVoid {
      typedef void type;
   };
}


//Useful for printing types (for debugging)
#if 0
#include <cxxabi.h>
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
