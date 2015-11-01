//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

#include <stat_log/util/compile_proxy.h>
#include <tuple>
#include <utility>
#include <bitset>

//Defines the convenience method writeBitStatus.
//This method will call stat_log::WriteStat(idx..., bit_idx, 1)
//for each bit_idx in which the status bits are set.
//
//Example usage:
//   stat_log::writeBitStatus<TAG>(status);
// Or
//   stat_log::writeBitStatus<TAG>(status, idx1);
// Or
//   stat_log::writeBitStatus<TAG>(status, idx1, idx2);
// Etc. (for higher dimensioned stats).
//
//
//NOTE: my own definition of invoke().  I would have liked
//to use std::invoke, but I am not using a C++17 compiler (yet).

namespace stat_log
{
namespace detail
{
   template <typename TAG, typename Args, std::size_t... Indices>
   void invoke(const Args& theArgs, std::index_sequence<Indices...>)
   {
      stat_log::writeStat<TAG>(std::get<Indices>(theArgs)...);
   }
}

template <typename TAG, typename U, typename... Indices>
void writeBitStatus(U bitstatus, Indices... indices)
{
   auto idx_tuple = std::make_tuple(indices...);
   std::bitset<sizeof(U)*8> bs(bitstatus);
   for(size_t i = 0; i < bs.size(); ++i)
   {
      if(bs[i])
      {
         auto bit_idx_1_tuple = std::make_tuple(i, 1);
         auto args_tuple = std::tuple_cat(idx_tuple, bit_idx_1_tuple);
         detail::invoke<TAG>(args_tuple,
               std::make_index_sequence<std::tuple_size<decltype(args_tuple)>::value>());
      }
   }
}
}
