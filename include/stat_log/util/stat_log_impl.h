#pragma once
//TODO: generalize backend types
#include "stat_log/defs.h"
#include "stat_log/backends/shared_mem_backend.h"
#include "stat_log/loggers/logger_common.h"
#include "stat_log/stats/stats_common.h"

#include <boost/fusion/view.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/type_traits.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/include/deref.hpp>
#include <boost/fusion/include/begin.hpp>
#include <boost/fusion/include/for_each.hpp>
#include <boost/fusion/include/vector.hpp>

#include <boost/fusion/include/make_vector.hpp>
#include <boost/fusion/include/mpl.hpp>

#include <boost/mpl/arg.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/contains.hpp>

#include <memory>
#include <type_traits>
#include <string>

namespace stat_log
{

namespace detail
{
   using namespace boost;
   using namespace boost::fusion;

   template <typename Tag, typename MatchingTags>
   struct GenericLogger
   {
      void setSharedPtr(void* ptr)
      {
         theProxy.setSharedPtr(ptr);
      }

      void doSerialize()
      {
      }

      using Proxy = LogProxy<LogControlWord>;
      //theProxy is use to both
      // 1. Set the log level (control) AND
      // 2. Check the current log level (operational)
      Proxy theProxy;
      using matching_tags = MatchingTags;
      static constexpr bool IsParent = true;
   };

   template <typename Tag, typename MatchingTags>
   struct GenericStat
   {
      using matching_tags = MatchingTags;
      static constexpr bool IsParent = false;
   };

   template <typename Tag, typename MatchingTags>
      struct GenericOpStat : GenericStat<Tag, MatchingTags>
   {
      template <typename... Args>
      void writeVal(Args... args)
      {
         this->theProxy.write(args...);
      }

      void setSharedPtr(void* ptr)
      {
         theProxy.setSharedPtr(ptr);
      }
      using Proxy = OperationalStatProxy<typename stat_tag_to_type<Tag>::type>;
      Proxy theProxy;

      void doSerialize()
      {
         theProxy.serialize();
      }
   };


   template <typename Tag, typename MatchingTags>
      struct GenericControlStat : GenericStat<Tag, MatchingTags>
   {
      void setSharedPtr(void* ptr)
      {
         theProxy.setSharedPtr(ptr);
      }
      using Proxy = ControlStatProxy<typename stat_tag_to_type<Tag>::type>;
      Proxy theProxy;

   };

   ///////////////

   template <typename T>
      struct is_parent
      {
         //T is of type "struct TagNode"
         //Then T::child_list should be either another sequence (if parent) or void
         using type = typename mpl::is_sequence<typename T::child_list>::type;
         static const bool value = mpl::is_sequence<typename T::child_list>::value;
      };

   template <typename GlobalTagVec, typename ParentVec,
             typename ThisStat ,typename IsOpType
             >
      struct stat_inserter
      {
         //ThisStat is of type "struct TagNode"
         using ThisTag = typename ThisStat::tag;
         //Add the entire lineage of parents to the matching tags for
         // this statistic
         using matching_tags =
            typename mpl::push_back<ParentVec,ThisTag>::type;
         using this_stat = typename std::conditional_t<
               IsOpType::value,
               GenericOpStat<ThisTag, matching_tags>,
               GenericControlStat<ThisTag, matching_tags>
            >;
         //Finally add this statistic to the global tag vec
         using type = typename mpl::push_back<GlobalTagVec, this_stat>::type;
      };

   template <typename GlobalTagVec, typename ParentVec,
             typename TagHierarchy , typename IsOpType
             >
      struct stat_creator_helper
      {

         using ThisLineage = typename mpl::push_back<ParentVec,
               typename TagHierarchy::tag>::type;
         using ChildTagHierarchy = typename TagHierarchy::child_list;
         using ThisTag = typename TagHierarchy::tag;
         using this_logger = GenericLogger<ThisTag, ThisLineage>;

         using UpdatedGlobalTagVec = typename mpl::push_back<
            GlobalTagVec, this_logger>::type;

         // _1 == UpdatedGlobalTagVec
         // _2 == the iterator to the child TagNode
         using type = typename mpl::fold<
               ChildTagHierarchy,
               UpdatedGlobalTagVec,
               mpl::eval_if<
                  is_parent<mpl::_2>,
                  stat_creator_helper<
                     mpl::_1, ThisLineage, mpl::_2, IsOpType
                  >,
                  stat_inserter<mpl::_1, ThisLineage, mpl::_2, IsOpType
                  >
               >
            >::type;
      };

   template <typename TagHierarchy, bool IsOperational>
      struct stat_creator
      {
         using type = typename stat_creator_helper<
            mpl::vector<>, //Global Tag/Stat Vector
            mpl::vector<>, //Parent vector
            TagHierarchy,
            //Need to wrap the boolean to be nice to the MPL algorithms
            typename std::integral_constant<bool, IsOperational>
            >::type;
      };

   template <typename TagVec, typename Tag>
      struct contains_tag
      {
         using type = typename mpl::contains<TagVec, Tag>::type;
      };

   template <typename Tag>
      struct matches_tag
      {
         template <typename Repr>
            struct apply
            : contains_tag<typename Repr::matching_tags, Tag>::type
            {};
      };

   template <typename T, typename P, int Depth, typename C = void>
      struct TagNode
      {
         NAME = T::name;
         using tag = T;
         using parent = P;
         static const int depth = Depth;
         using child_list = C;
      };

   template <typename T, typename Parent, typename Depth, typename Dummy = void>
      struct GenTagHierarchy
      {
         using type = TagNode<T, Parent, Depth::value>;
      };

   template <typename T, typename Parent, typename Depth>
      struct GenTagHierarchy<T, Parent, Depth,
         typename AlwaysVoid<typename T::ChildTypes>::type>
      {
         using ChildTypes = typename T::ChildTypes;
         using ChildDepth = std::integral_constant<int, Depth::value + 1>;
         using child_type_vec = typename boost::mpl::transform<
            ChildTypes, GenTagHierarchy<boost::mpl::_, T, ChildDepth>>::type;
         using type = TagNode<T, Parent, Depth::value, child_type_vec>;
      };

   template <typename UserStatDefs, bool IsOperational,
            typename Logger, typename Derived>
      struct LogStatBase
   {
      struct TopName
      {
         NAME = "";
      };
      using TopNode = detail::TagNode<TopName, void, 0>;

      using TagHierarchy = typename detail::GenTagHierarchy<UserStatDefs, TopNode,
            std::integral_constant<int, 0> >::type;
      using TheStats = typename boost::fusion::result_of::as_vector<
         typename detail::stat_creator<TagHierarchy, IsOperational>::type>::type;

      //Creates a single shared memory block that will store:
      //  -- The serialization for each stat.
      //  -- The control block for each stat (this will be
      //     used to signal per-stat behavior -- e.g., clear
      //     disable, etc.).
      //  -- The control block for each parent (this will be
      //     used to set the log level for both the normal logging
      //     AND the hex dumps).
      //1. Control Block
      //2. Stat Block
      void init(const std::string& shm_name)
      {
         size_t total_shm_size = 0;
         using namespace boost::fusion;
         for_each(theStats, [&total_shm_size](auto& stat)
         {
            using StatType = std::remove_reference_t<decltype(stat)>;
            total_shm_size += StatType::Proxy::getSharedSize();
         });
         shm_backend.setParams(shm_name, total_shm_size, IsOperational);
         auto shm_start = shm_backend.getMemoryPtr();
         std::cout << std::dec <<  "SHM size = " << total_shm_size
            <<" , shm_start = " << std::hex << (long int)shm_start << std::endl;
         auto shm_ptr = shm_start;

         //Next, need to inform each node theStats about its location
         // in shared memory
         for_each(theStats, [&shm_ptr](auto& stat)
         {
            using StatType = std::remove_reference_t<decltype(stat)>;
            stat.setSharedPtr(shm_ptr);
            shm_ptr += StatType::Proxy::getSharedSize();
         });

         static_cast<Derived*>(this)->doInit();
      }

      void stop()
      {
         static_cast<Derived*>(this)->doStop();
      }


      std::shared_ptr<Logger> theLogger;
      TheStats theStats;
      shared_mem_backend shm_backend;
   };

   template<typename Tag, typename T>
      auto getStatHandleView(T& stats)
      {
         return boost::fusion::filter_view<T, detail::matches_tag<Tag>>(stats);
      }

   template <typename Tag, typename T>
      auto& getValue(T& stats)
      {
         auto statHdlView = getStatHandleView<Tag>(stats);
         static_assert(
               boost::fusion::result_of::size<decltype(statHdlView)>::value == 1,
               "getValues requires a Leaf Tag!");
         return deref(begin(statHdlView)).getValue();
      }
}
}
