#pragma once
#include "fusion_includes.h"
#include "util/stat_log_impl.h"
#include "parsers/parent_parser.h"

#include <boost/algorithm/string/join.hpp>

namespace stat_log
{


template <int N, typename T>
struct StatTable
{

};

//Operational proxies
struct OpProxyBasic
{
   int* serial_ptr = nullptr;
   void setShmPtr(void* ptr)
   {
      serial_ptr = reinterpret_cast<int*>(ptr);
   }

   void write(int i)
   {
      (*serial_ptr) += i;
   }

   //TODO: temporary
   auto& getValue()
   {
      return (*serial_ptr);
   }
};


//Observer/Control proxies
//TODO:
//

template <typename UserStatH>
struct LogStatOperational : detail::LogStatBase<UserStatH, true>
{
};

template <typename UserStatH>
struct LogStatControl : detail::LogStatBase<UserStatH, false>
{
   using BaseClass = detail::LogStatBase<UserStatH, false>;
   using TopNode = typename BaseClass::TopNode;
   using TagHierarchy = typename BaseClass::TagHierarchy;

   void parseUserCommands(int argc, char** argv)
   {
      //First extract the Component hierarchy portion of
      // the user_input (if it exists)

      std::vector<std::string> user_strings;
      for(int i = 1; i < argc; ++i)
         user_strings.push_back(argv[i]);

      std::string user_cmd_line = boost::algorithm::join(user_strings, " ");
      std::cout << "User cmd line = " << user_cmd_line << std::endl;
      std::string component_str = getComponentName(user_cmd_line);
      std::cout << "COMPONENT = " << component_str << std::endl;

      std::cout << "Is parent = " << detail::is_parent<TagHierarchy>::value << std::endl;
      parse<TagHierarchy>(this->theStats, component_str, user_cmd_line);
   }
};

}



//MAKE_STAT_TAGS is a convenience wrapper for creating statistics.
//For example:
//
//     struct RxCountTag{
//        NAME = "RX_COUNTERS";
//     };
//     struct CorrScoreTag{
//        NAME = "CORR_SCORE";
//     };
//     struct LinkIndTag{
//        NAME = "LINK_IND";
//     };
//
//     using ChildTypes = vector<RxCountTag, CorrScoreTag, LinkIndTag>;
//
//Can be replaced by a single line:
//     MAKE_STAT_TAGS( (RxCount) (CorrScore) (LinkInd))
//
//TODO: it would be really cool if we can define stat hierarchies in this
// way. i.e.
//    MAKE_STAT_TAGS( (Level1Stats (RxCount) (CorrScore) (LinkInd))
//                    (Level2Stats (IpDown) (IpUp))
//                   )
//
//But it seems that boost's preprocessor library cannot handle
// nested sequences:
// http://stackoverflow.com/questions/26739178/are-two-dimensional-sequences-possible-with-boost-preprocessor
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/transform.hpp>

#define _MAKE_STAT_TAG(r, data, elem) BOOST_PP_CAT(elem,data)

#define _MAKE_STAT_TAG_STRUCT(r, data, elem) \
   struct _MAKE_STAT_TAG(r, data, elem) { \
         NAME = BOOST_PP_STRINGIZE(elem); \
            };

#if 1
#define MAKE_STAT_TAGS(args) \
      BOOST_PP_SEQ_FOR_EACH (_MAKE_STAT_TAG_STRUCT, Tag, args) \
   using ChildTypes = boost::fusion::vector<BOOST_PP_SEQ_ENUM(\
         BOOST_PP_SEQ_TRANSFORM(_MAKE_STAT_TAG, Tag, args))>;
#else
#endif

