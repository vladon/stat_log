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
