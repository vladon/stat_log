//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

#pragma once
#include <stat_log/loggers/logger_common.h>
// The purpose of this header is to provide the user a means to reduce the
// compile-time dependency issues with the stat log library.  Due to the large
// amount of template meta-programming involved, it will take awhile to compile
// each translation unit that includes the full-definition of the stat hierarchy.
//
//If this becomes unbearable, include this header in your client (operational)
// code and do things like:
//   writeStat<ThisStatTag>(newValue);
//   log<ThisLogTag>() << "Logging Message";
//   hexDump<ThisLogTag>().hexDump(buf, buf_size, "Label");
//
//NOTE: that some translation unit should actually provide definitions for
//   each of these methods otherwise you will get link errors.
//
// For example (in some .cpp file):
/*
using OpStat = LogStatOperational<YourStatHierarchy>;
template <typename Tag>
inline LogGenProxy& logCommon(LogLevel ll)
{
   return getOpStat<OpStat>().getLog<Tag>(0, ll) << "Invalid log level!";
}
template <typename Tag>
LogGenProxy log(LogLevel ll)
{
   return logCommon<Tag>(ll, 0)
}

LogGenProxy hexDump(LogLevel ll)
{
   return logCommon<Tag>(ll, 1)
}

template <typename Tag, typename ... Args>
void writeStat(Args... args)
{
   stat_log::getStatLogSingleton<OpStat>().writeStat<Tag>(args...);
}

//Explicit template declarations
// you need to do this for each stat/log tag that will
// be used in your program
template void writeStat<ThisStatTag>(int);
template LogGenProxy& log<ThisLogTag>(LogLevel ll);
template LogGenProxy& hexDump<ThisLogTag>(LogLevel ll);
*/
namespace stat_log
{
   template<typename Tag, typename ...Args>
   void writeStat(Args... args);

   template<typename Tag>
   LogGenProxy logger(LogLevel ll = LogLevel::INFO);

   template<typename Tag>
   LogGenProxy hexDumper(LogLevel = LogLevel::INFO);
}
