#include <stat_log/util/printer.h>
#include <unordered_map>
#include <vector>
#include <numeric>
#include <iostream>
#include <sstream>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace stat_log;

namespace
{

StatCmd the_cmd = StatCmd::NO_CMD;
PrintOptions printOptions;

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
   StatDisplay(TagInfo& ti, PrintOptions& po, StatCmdOutput&& cmd_out)
    : tag_info(ti), print_options(po), stat_output(std::move(cmd_out))
   {}

   TagInfo tag_info;
   PrintOptions print_options;
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

template <typename T>
void removeTag(T& v, const TagInfo& tag_info)
{
   v.erase(std::remove_if(v.begin(), v.end(), [&](const auto& elem)
            { return elem.tag_info.tag_index == tag_info.tag_index;}),
           v.end());
}

}

void Printer::setCommand(StatCmd cmd)
{
   the_cmd = cmd;
}

void Printer::setPrintOptions(PrintOptions& print_options)
{
   printOptions = print_options;
}

void Printer::addStatOutput(TagInfo& tag_info, StatCmdOutput&& stat_output)
{
   if(tag_info.depth < min_tag_depth)
      min_tag_depth = tag_info.depth;
   removeTag(stat_tags_vector, tag_info);
   stat_tags_vector.emplace_back(tag_info, printOptions, std::move(stat_output));
}

void Printer::addLogOutput(TagInfo& tag_info, std::string&& log_output)
{
   if(tag_info.depth < min_tag_depth)
      min_tag_depth = tag_info.depth;
   removeTag(log_tags_vector, tag_info);
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

         auto convert_str_to_number = [](std::string& str, size_t& num)
         {
            try
            {
               num = boost::lexical_cast<size_t>(str);
            }
            catch(boost::bad_lexical_cast&)
            {
            }
         };

         auto belongs_to_range = [&](const std::string& range_str, size_t val)
         {
            std::vector<std::string> ranges;
            boost::split(ranges, range_str, boost::is_any_of("."));
            for(auto& range: ranges)
            {
               std::vector<std::string> first_last;
               boost::split(first_last, range, boost::is_any_of("-"));
               if(first_last.size() == 1)
               {
                  const auto invalid_num = std::numeric_limits<size_t>::max();
                  size_t the_val = invalid_num;
                  convert_str_to_number(first_last[0], the_val);
                  if(the_val == val || the_val == invalid_num)
                     return true;
               }
               else
               {
                  auto low_val = std::numeric_limits<size_t>::min();
                  auto high_val = std::numeric_limits<size_t>::max();

                  std::vector<size_t*> vals = {&low_val, &high_val};
                  for(int i = 0; i < (int)first_last.size(); ++i)
                  {
                     convert_str_to_number(first_last[i], *vals[i]);
                  }
                  if(val >= low_val && val <= high_val)
                  {
                     return true;
                  }
               }
            }
            return false;
         };

         const auto& array_indices = elem.print_options.array_indices;
         auto idx_to_dim_string = [&](size_t idx, bool& do_print)
         {
            do_print = true;
            const auto& dimSizes = stat_output.dimensionSizes;
            std::vector<size_t> dim_indices(dimSizes.size(), 0);
            for(size_t i = 0; i < dimSizes.size(); ++i)
            {
               auto j = dimSizes.size() - 1 - i;
               dim_indices[j] = idx % dimSizes[j];
               idx = idx / dimSizes[j];
            }
            std::stringstream ss;
            std::vector<std::string> per_dim_ranges;
            if(array_indices.empty() == false)
               boost::split(per_dim_ranges, array_indices, boost::is_any_of(","));

            for(size_t i = 0;  i < dim_indices.size(); ++i)
            {
               if(i < per_dim_ranges.size() && do_print == true)
               {
                  if(!belongs_to_range(per_dim_ranges[i], dim_indices[i]))
                     do_print = false;
               }

               ss << dim_indices[i];
               if(i < dim_indices.size()-1)
                  ss << ",";
               else
                  ss << ": ";
            }
            return ss.str();
         };

         bool do_print = true;
         if(!stat_output.entryTitle.empty())
         {
            const auto idx_string = idx_to_dim_string(0, do_print);
            for(size_t i = 0; i < idx_string.size(); ++i)
               std::cout << " ";
            std::cout << stat_output.entryTitle << std::endl;
         }
         for(size_t i = 0; i < stat_output.entries.size(); ++i)
         {
            auto& stat_entry = stat_output.entries[i];
            std::string idx_str = idx_to_dim_string(i, do_print);
            if(do_print)
               std::cout << idx_str << stat_entry << std::endl;
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
