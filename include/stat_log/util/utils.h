#pragma once

#include <string>
#include <vector>
#include <tuple>

namespace stat_log
{
struct PrintOptions;

void parseCommandLineArgs(int argc, char** argv,
      std::vector<std::string>& tag_strings,
      StatCmd& cmd, boost::any& cmd_arg,
      PrintOptions& printOptions);

std::tuple<std::string, std::string> getHeadTail(std::string s, char delim);

}
