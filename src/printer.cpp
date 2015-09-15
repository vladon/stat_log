#include <stat_log/util/printer.h>
#include <unordered_map>
#include <vector>
#include <numeric>
#include <iostream>
#include <sstream>

using namespace stat_log;

namespace
{

StatCmd the_cmd = StatCmd::NO_CMD;
PrintOptions the_print_options;

void indent(size_t level)
{
   if(level <= 0)
      return;
   int i = level;
   while(i-- > 0)
      std::cout << "\t";
}

struct StatDisplay
{
   StatDisplay(TagInfo& ti, StatCmdOutput&& cmd_out)
    : tag_info(ti), stat_output(std::move(cmd_out))
   {}
   TagInfo tag_info;
   StatCmdOutput stat_output;
};

struct LogDisplay
{
   LogDisplay(TagInfo& ti, std::string&& log_out)
    : tag_info(ti), log_output(std::move(log_out))
   {}
   TagInfo tag_info;
   std::string log_output;
};

std::unordered_map<std::type_index, StatDisplayOptions> stat_options_map;
std::vector<StatDisplay> stat_tags_vector;
std::vector<LogDisplay>  log_tags_vector;

size_t min_tag_depth = std::numeric_limits<size_t>::max();
}

void Printer::setCommand(StatCmd cmd, PrintOptions& print_options)
{
   the_cmd = cmd;
   the_print_options = print_options;
}

void Printer::addStatOutput(TagInfo& tag_info, StatCmdOutput&& stat_output)
{
   if(tag_info.depth < min_tag_depth)
      min_tag_depth = tag_info.depth;
   stat_tags_vector.emplace_back(tag_info, std::move(stat_output));
}

void Printer::addLogOutput(TagInfo& tag_info, std::string&& log_output)
{
   if(tag_info.depth < min_tag_depth)
      min_tag_depth = tag_info.depth;
   log_tags_vector.emplace_back(tag_info, std::move(log_output));
}

void Printer::setStatDisplayArgs(std::type_index tag_idx, StatDisplayOptions& display_options)
{
   //TODO: the user will call this (indirectly) if they want to specify display
   // parameters for a given stat tag -- e.g. dimension names, etc.
}

void Printer::showOutput()
{
   switch(the_cmd)
   {
      case StatCmd::PRINT_STAT_TYPE:
         printStatType();
         break;
      case StatCmd::DUMP_STAT:
         printDumpStat();
         break;
      case StatCmd::LOG_LEVEL:
         printLogOutput();
         break;
      case StatCmd::PRINT_TAG:
         printTags();
         break;
      default:
         break;
   }
}


void Printer::printStatType()
{
   if(!stat_tags_vector.empty())
      std::cout << "STAT TYPES:\n";
   for(auto& elem: stat_tags_vector)
   {
      auto& tag_info = elem.tag_info;
      indent(tag_info.depth - min_tag_depth);
      std::cout << tag_info.name << std::endl;
      if(!elem.stat_output.entryTitle.empty())
      {
         indent(tag_info.depth - min_tag_depth);
         std::cout << "  " << elem.stat_output.entryTitle << std::endl;
      }
   }
}

void Printer::printDumpStat()
{
   if(!stat_tags_vector.empty())
      std::cout << "STAT VALUES:\n";
   for(auto& elem: stat_tags_vector)
   {
      auto& stat_output = elem.stat_output;
      if(!stat_output.entries.empty())
      {
         std::cout << elem.tag_info.name << std::endl;
         if(!stat_output.entryTitle.empty())
            std::cout << "  " << stat_output.entryTitle << std::endl;

         auto idx_to_dim_string = [](size_t idx, const std::vector<size_t>& dimSizes)
         {
            std::vector<size_t> dim_indices(dimSizes.size(), 0);
            for(size_t i = 0; i < dimSizes.size(); ++i)
            {
               auto j = dimSizes.size() - 1 - i;
               dim_indices[j] = idx % dimSizes[j];
               idx = idx / dimSizes[j];
            }
            std::stringstream ss;
            for(size_t i = 0;  i < dim_indices.size(); ++i)
            {
               ss << dim_indices[i];
               if(i < dim_indices.size()-1)
                  ss << ",";
               else
                  ss << ": ";
            }
            return ss.str();
         };

         for(size_t i = 0; i < stat_output.entries.size(); ++i)
         {
            auto& stat_entry = stat_output.entries[i];
            if(elem.stat_output.entries.size() > 0)
               std::cout << idx_to_dim_string(i, stat_output.dimensionSizes);
            std::cout << stat_entry << std::endl;
         }

      }
   }
}

void Printer::printLogOutput()
{
   if(!log_tags_vector.empty())
      std::cout << "LOG LEVELS:\n";
   for(auto& elem: log_tags_vector)
   {
      auto& tag_info = elem.tag_info;
      indent(tag_info.depth - min_tag_depth);
      std::cout << tag_info.name << std::endl;
      indent(tag_info.depth - min_tag_depth);
      std::cout << "  " << elem.log_output << std::endl;
   }
}

void Printer::printTags()
{
   if(!stat_tags_vector.empty())
      std::cout << "STATS:\n";
   for(auto& elem: stat_tags_vector)
   {
      auto& tag_info = elem.tag_info;
      indent(tag_info.depth - min_tag_depth);
      std::cout << tag_info.name << std::endl;
   }

   if(!log_tags_vector.empty())
      std::cout << "LOG LEVELS:\n";
   for(auto& elem: log_tags_vector)
   {
      auto& tag_info = elem.tag_info;
      indent(tag_info.depth - min_tag_depth);
      std::cout << tag_info.name << std::endl;
   }
}
