#pragma once
#include <boost/fusion/tuple.hpp>
#include <boost/fusion/view.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/type_traits.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/sequence/intrinsic/at_c.hpp>
#include <boost/fusion/include/at_c.hpp>
#include <boost/fusion/include/begin.hpp>
#include <boost/fusion/include/find.hpp>
#include <boost/fusion/include/find_if.hpp>
#include <boost/fusion/include/deref.hpp>
#include <boost/fusion/include/distance.hpp>
#include <boost/fusion/include/begin.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/iterator_range.hpp>
#include <boost/fusion/include/vector.hpp>
#include <boost/fusion/include/pair.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/fusion/sequence/io/out.hpp>

#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/include/mpl.hpp>
#if 0
#include <boost/fusion/include/flatten_view.hpp>
#endif

#include <boost/mpl/arg.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/contains.hpp>

#include <type_traits>
#include <string>
#include <iostream>
#include <ostream>
#include <array>
#include <functional>
#include <vector>
#include <iterator>
#include <algorithm>
#include <tuple>
#include <ostream>



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
