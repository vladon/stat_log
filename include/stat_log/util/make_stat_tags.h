#pragma once
#include <stat_log/defs.h>

//SL_MAKE_TAGS is a convenience wrapper for creating tags.
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
//     using children = SL_MAKE_LIST(
//       children
//       (RxCountTag)
//       (CorrScoreTag)
//       (LinkIndTag)
//       );
//
//Can be replaced by a single line:
//     SL_MAKE_TAGS( (RxCount) (CorrScore) (LinkInd))
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
//     using children = SL_MAKE_LIST(
//       (RxCountTag)
//       (CorrScoreTag)
//       (LinkIndTag)
//       );
//
//Can be replaced by
//     using ChildTypes = SL_MAKE_TAGS_BASE(
//         children,
//         ((RxCount, Base1))
//         ((CorrScore, Base2))
//         ((LinkIndTag, Base3)))
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
//     using children = SL_MAKE_LIST(
//       (RxCountTag)
//       (CorrScoreTag)
//       (LinkIndTag)
//       );
//
// Can be replaced by:
//     SL_MAKE_TAGS_NAMED_BASE(
//        children
//        Base,
//        (RxCount)
//        (CorrScore)
//        (LinkInd));
//
//
//TODO: it would be really cool if we can define stat hierarchies in this
// way. i.e.
//    SL_MAKE_TAGS( (Level1Stats (RxCount) (CorrScore) (LinkInd))
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

#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/punctuation/comma.hpp>
#include <boost/preprocessor/seq/for_each_i.hpp>
#include <boost/preprocessor/control/if.hpp>

#include <boost/fusion/include/cons.hpp>
#include <boost/fusion/include/as_list.hpp>
#include <boost/fusion/include/push_back.hpp>


//// SL_MAKE_LIST ///////////
#define _CONS_SEP_START(r, token, i, e) token e BOOST_PP_COMMA()
#define _CONS_START(token, sequence) BOOST_PP_SEQ_FOR_EACH_I(_CONS_SEP_START, token, sequence)

#define _CONS_SEP_END(r, token, i, e) BOOST_PP_IF(i,, boost::fusion::nil) token
#define _CONS_END(token, sequence) BOOST_PP_SEQ_FOR_EACH_I(_CONS_SEP_END, token, sequence)

#define SL_MAKE_LIST(seq) \
   _CONS_START(boost::fusion::cons<, seq) \
   _CONS_END(>, seq)
//////////////////////////////////////////


//// SL_MAKE_TAGS ///////////
#define _SL_MAKE_TAG(r, data, elem) BOOST_PP_CAT(elem,data)

#define _SL_MAKE_TAG_STRUCT(r, data, elem) \
   struct _SL_MAKE_TAG(r, data, elem) { \
      SL_NAME = BOOST_PP_STRINGIZE(elem); \
   };

#define SL_MAKE_TAG_STRUCT(stat_name) \
   _SL_MAKE_TAG_STRUCT(~, _TAG, stat_name)

#define SL_MAKE_TAGS(ListName, args) \
   BOOST_PP_SEQ_FOR_EACH (_SL_MAKE_TAG_STRUCT, _TAG, args) \
   using ListName = SL_MAKE_LIST(\
         BOOST_PP_SEQ_TRANSFORM(_SL_MAKE_TAG, _TAG, args));
//////////////////////////////////////////



//// SL_MAKE_TAGS_BASE ///////////

#define _SL_MAKE_TAG_STRUCT_BASE(r, data, elem) \
   struct _SL_MAKE_TAG(r, data, BOOST_PP_TUPLE_ELEM(2,0, elem)) : BOOST_PP_TUPLE_ELEM(2,1,elem) { \
      SL_NAME = BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(2,0, elem)); \
   };

#define SL_MAKE_TAG_STRUCT_BASE(stat_name, base) \
   _SL_MAKE_TAG_STRUCT_BASE(~, _TAG, (stat_name, base))


#define _SL_MAKE_TAG_BASE(r, data, elem) BOOST_PP_CAT(BOOST_PP_TUPLE_ELEM(2,0,elem),data)

#define SL_MAKE_TAGS_BASE(ListName, args) \
   BOOST_PP_SEQ_FOR_EACH (_SL_MAKE_TAG_STRUCT_BASE, _TAG, args) \
   using ListName = SL_MAKE_LIST(\
         BOOST_PP_SEQ_TRANSFORM(_SL_MAKE_TAG_BASE, _TAG, args));
//////////////////////////////////////////



//// SL_MAKE_TAGS_NAMED_BASE ///////////
#define _MAKE_TUPLE(r, data, elem) (elem, data)

#define SL_MAKE_TAGS_NAMED_BASE(ListName, Base, args) \
   SL_MAKE_TAGS_BASE(ListName, BOOST_PP_SEQ_TRANSFORM(_MAKE_TUPLE, Base, args))
//////////////////////////////////////////


//// SL_APPEND_TO_TAG_LIST ///////////
#define SL_APPEND_TO_TAG_LIST(NewListName, OrigListName, elem) \
   using NewListName = boost::fusion::result_of::as_list<\
      boost::fusion::result_of::push_back<OrigListName, elem>::type\
   >::type
//////////////////////////////////////////

