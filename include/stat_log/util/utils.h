#pragma once

#include <string>
#include <vector>
#include <tuple>

namespace stat_log
{
void parseCommandLineArgs(int argc, char** argv,
      std::vector<std::string>& component_strings,
      StatCmd& cmd, boost::any& cmd_arg);

std::tuple<std::string, std::string> getHeadTail(std::string s, char delim);

}
