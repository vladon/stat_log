#pragma once
#include <boost/mpl/is_sequence.hpp>
#include <typeindex>

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

   template <typename TheTagNode>
   struct is_parent
   {
      //TheTagNode::child_list should be either another sequence (if parent) or void
      using type = typename boost::mpl::is_sequence<typename TheTagNode::child_list>::type;
      static const bool value = boost::mpl::is_sequence<typename TheTagNode::child_list>::value;
   };

   struct TagInfo
   {
      const char* name;
      std::type_index tag_index;
      std::size_t depth;
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
