//                Copyright Robert J McCabe 2015.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)
//
//     Please report any bugs, typos, or suggestions to
//         https://github.com/rjmccabe3701/stat_log/issues

#pragma once
#include <stat_log/util/command.h>
#include <stat_log/defs.h>
#include <string>

namespace stat_log
{
struct PrintOptions
{
   std::string array_indices;
};

struct TagDisplayDesc
{
   std::string tag_hname;
   PrintOptions print_options;
};

//TODO: the user can use this to set enum/dimension_names
struct StatDisplayOptions
{
};

class Printer
{
public:
   void setCommand(StatCmd cmd);
   void setPrintOptions(PrintOptions& print_options);
   void addStatOutput(TagInfo& tag_info, StatCmdOutput&& stat_output);
   void addLogOutput(TagInfo& tag_info, std::string&& log_output);
   void setStatDisplayArgs(std::type_index tag_idx, StatDisplayOptions& stat_display_options);
   void showOutput();
private:
   void printStatType();
   void printDumpStat();
   void printLogOutput();
   void printTags();
};

}
