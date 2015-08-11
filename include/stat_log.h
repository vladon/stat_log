#pragma once
#include "fusion_includes.h"
#include "util/stat_log_impl.h"
//using namespace boost::fusion;

#define NAME static constexpr const char* name
namespace rc
{
namespace stat_log
{


template <int N, typename T>
struct StatTable
{

};

//Operational proxies
struct OpProxyBasic
{
   int* serial_ptr = nullptr;
   void setShmPtr(void* ptr)
   {
      serial_ptr = reinterpret_cast<int*>(ptr);
   }

   void write(int i)
   {
      (*serial_ptr) += i;
   }

   //TODO: temporary
   auto& getValue()
   {
      return (*serial_ptr);
   }
};


//Observer/Control proxies
//TODO:
//

template<typename Tag, typename T>
auto getStatHandleView(T& stats)
{
   return boost::fusion::filter_view<T, detail::matches_tag<Tag>>(stats);
}

template <typename Tag, typename T>
auto& getValue(T& stats)
{
   using namespace boost::fusion;
   auto statHdlView = getStatHandleView<Tag>(stats);
   static_assert(result_of::size<decltype(statHdlView)>::value == 1,
         "getValues requires a Leaf Tag!");
    return deref(begin(statHdlView)).getValue();
}

template <typename UserStatH>
struct LogStatOperational : detail::LogStatBase<UserStatH, true>
{
};

template <typename UserStatH>
struct LogStatControl : detail::LogStatBase<UserStatH, false>
{
};

}
}
