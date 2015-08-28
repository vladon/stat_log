#pragma once
#include "stat_log/defs.h"

//MAKE_STAT_TAGS is a convenience wrapper for creating statistics.
//For example:
//
//     struct RxCountTag{
//        SL_NAME = "RX_COUNTERS";
//     };
//     struct CorrScoreTag{
//        SL_NAME = "CORR_SCORE";
//     };
//     struct LinkIndTag{
//        SL_NAME = "LINK_IND";
//     };
//
//     using ChildTypes = vector<RxCountTag, CorrScoreTag, LinkIndTag>;
//
//Can be replaced by a single line:
//     MAKE_STAT_TAGS( (RxCount) (CorrScore) (LinkInd))
//
//++++++++++++++++++++++++++++++++++++++++++
//Or
//
//     struct RxCountTag : Base1{
//        SL_NAME = "RX_COUNTERS";
//     };
//     struct CorrScoreTag : Base2{
//        SL_NAME = "CORR_SCORE";
//     };
//     struct LinkIndTag : Base3{
//        SL_NAME = "LINK_IND";
//     };
//
//     using ChildTypes = vector<RxCountTag, CorrScoreTag, LinkIndTag>;
//
//Can be replaced by
//     MAKE_STAT_TAGS_BASE( ((RxCount, Base1)) ((CorrScore, Base2)) ((LinkIndTag, Base3)))
//
//++++++++++++++++++++++++++++++++++++++++++
//Or
//
//     struct RxCountTag : Base{
//        SL_NAME = "RX_COUNTERS";
//     };
//     struct CorrScoreTag : Base{
//        SL_NAME = "CORR_SCORE";
//     };
//     struct LinkIndTag : Base{
//        SL_NAME = "LINK_IND";
//     };
//
//     using ChildTypes = vector<RxCountTag, CorrScoreTag, LinkIndTag>;
//
// Can be replaced by:
//     MAKE_STAT_TAGS_NAMED_BASE( Base, (RxCount) (CorrScore) (LinkInd))
//
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
#include <boost/preprocessor/tuple/elem.hpp>

#include <boost/fusion/include/vector.hpp>


#define _MAKE_STAT_TAG(r, data, elem) BOOST_PP_CAT(elem,data)

#define _MAKE_STAT_TAG_STRUCT(r, data, elem) \
 struct _MAKE_STAT_TAG(r, data, elem) { \
       SL_NAME = BOOST_PP_STRINGIZE(elem); \
          };

#define MAKE_STAT_TAG_STRUCT(stat_name) \
 _MAKE_STAT_TAG_STRUCT(~, _TAG, stat_name)

#define MAKE_STAT_TAGS(args) \
    BOOST_PP_SEQ_FOR_EACH (_MAKE_STAT_TAG_STRUCT, _TAG, args) \
 using ChildTypes = boost::fusion::vector<BOOST_PP_SEQ_ENUM(\
       BOOST_PP_SEQ_TRANSFORM(_MAKE_STAT_TAG, _TAG, args))>;

////////////////////////////

#define _MAKE_STAT_TAG_STRUCT_BASE(r, data, elem) \
 struct _MAKE_STAT_TAG(r, data, BOOST_PP_TUPLE_ELEM(2,0, elem)) : BOOST_PP_TUPLE_ELEM(2,1,elem) { \
       SL_NAME = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(2,0, elem)); \
          };

#define MAKE_STAT_TAG_STRUCT_BASE(stat_name, base) \
 _MAKE_STAT_TAG_STRUCT_BASE(~, _TAG, (stat_name, base))


#define _MAKE_STAT_TAG_BASE(r, data, elem) BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2,0,elem),data)

#define MAKE_STAT_TAGS_BASE(args) \
BOOST_PP_SEQ_FOR_EACH (_MAKE_STAT_TAG_STRUCT_BASE, _TAG, args) \
   using ChildTypes = boost::fusion::vector<BOOST_PP_SEQ_ENUM(\
          BOOST_PP_SEQ_TRANSFORM(_MAKE_STAT_TAG_BASE, _TAG, args))>;

////////////////////////////


#define _MAKE_TUPLE(r, data, elem) (elem, data)

#define MAKE_STAT_TAGS_NAMED_BASE(Base, args) \
       MAKE_STAT_TAGS_BASE(BOOST_PP_SEQ_TRANSFORM(_MAKE_TUPLE, Base, args))
