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


