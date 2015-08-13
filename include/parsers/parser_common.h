#pragma once
#include "util/stat_log_impl.h"

#include <boost/program_options.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include <boost/tokenizer.hpp>
#include <boost/token_functions.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/any.hpp>

#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>
#include <memory>
#include <regex>
#include <vector>
#include <stdexcept>
#include <cstdlib>
#include <assert.h>

#define TERM_NUM_COLUMNS 100
namespace stat_log
{
std::vector<std::string> tokenize(const std::string& input)
{
   typedef boost::escaped_list_separator<char> separator_type;
   separator_type separator("\\",     // The escape characters.
         "= ",     // The separator characters.
         "\"\'");  // The quote characters.

   // Tokenize the intput.
   boost::tokenizer<separator_type> tokens(input, separator);

   // Copy non-empty tokens from the tokenizer into the result.
   std::vector<std::string> result;
   copy_if(tokens.begin(), tokens.end(), std::back_inserter(result),
         [](auto& str)
         {
         return !str.empty();

         });
   return result;
}


enum class StatCmd
{
   NO_CMD,
   PRINT_STAT_TYPE,
   DUMP_STAT,
   CLEAR_STAT,
   DUMP_TIMESERIES,
};



// template <typename Stat>
// struct StatCommander;
template<typename Stat, bool IsParent>
struct DoCmd;

inline void indent(size_t level)
{
   int i = level;
   while(i-- > 0)
      std::cout << "\t";
}

}
