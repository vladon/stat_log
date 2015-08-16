#pragma once
#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>

#include <string>
#include <vector>
#include <tuple>

namespace stat_log
{
namespace detail
{
   boost::program_options::options_description getParentOptions();
   void indent(size_t level);
}

std::string getComponentName(std::string cmd_line);

std::vector<std::string> tokenize(const std::string& input);

std::tuple<std::string, std::string> getHeadTail(std::string s, char delim);
}
