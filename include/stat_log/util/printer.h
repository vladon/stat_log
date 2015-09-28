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
