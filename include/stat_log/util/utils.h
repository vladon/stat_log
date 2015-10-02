//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

#pragma once

#include <stat_log/util/printer.h>
#include <string>
#include <vector>
#include <tuple>

namespace stat_log
{
struct PrintOptions;

void parseCommandLineArgs(int argc, char** argv,
      std::vector<TagDisplayDesc>& tag_disp_descs,
      StatCmd& cmd, boost::any& cmd_arg);

std::tuple<std::string, std::string> getHeadTail(std::string s, char delim);

}
