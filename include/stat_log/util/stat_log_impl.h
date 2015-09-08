#pragma once
//TODO: generalize backend types
#include <stat_log/defs.h>
#include <stat_log/backends/shared_mem_backend.h>
#include <stat_log/loggers/logger_common.h>
#include <stat_log/stats/stats_common.h>

#include <boost/fusion/view.hpp>
#include <boost/fusion/algorithm.hpp>
#include <boost/type_traits.hpp>
#include <boost/fusion/support/is_sequence.hpp>
#include <boost/fusion/include/deref.hpp>
#include <boost/fusion/include/begin.hpp>
#include <boost/fusion/include/for_each.hpp>

#include <boost/fusion/include/as_list.hpp>
#include <boost/fusion/include/list.hpp>

#include <boost/mpl/arg.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/fold.hpp>
#include <boost/mpl/contains.hpp>
#include <boost/mpl/find_if.hpp>

#include <type_traits>
#include <string>

namespace stat_log
{

namespace detail
{
   using namespace boost;
   using namespace boost::fusion;


   //++++ LOGGER CONTAINERS ++++++++++
   template <typename TagNode>
   struct GenericOpLogger
   {
      using tag_node = TagNode;
      using tag = typename tag_node::tag;
      void setSharedPtr(void* ptr)
      {
         theProxy.setSharedPtr(ptr);
      }
      static size_t getSharedSize()
      {
         return Proxy::getSharedSize();
      }

      auto getLevel(size_t log_idx)
      {
         return theProxy.getLevel(log_idx);
      }
   private:
      using Proxy = LogOpProxy;
      Proxy theProxy;
   };

   template <typename TagNode>
   struct GenericControlLogger
   {
      using tag_node = TagNode;
      using tag = typename tag_node::tag;
      void setSharedPtr(void* ptr)
      {
         theProxy.setSharedPtr(ptr);
      }

      static size_t getSharedSize()
      {
         return Proxy::getSharedSize();
      }

      template <typename... Args>
      void doCommand(Args&&... args)
      {
         theProxy.doCommand(std::forward<Args>(args)...);
      }

   private:
      using Proxy = LogControlProxy;
      Proxy theProxy;
   };
   //+++++++++++++++++++++++++++++++++

   //++++ STATISTIC CONTAINERS ++++++++++

   //"StatParent" is a basically just a placeholder node
   // just to tie together the statistic hierarchy.
   //Stat "parent" nodes -- unlike like log parent nodes --
   //aren't directly interacted with by either the control
   //or operational code.
   template <typename TagNode>
   struct StatParent
   {
      using tag_node = TagNode;
      using tag = typename tag_node::tag;

      void setSharedPtr(void* ptr) {}

      void doSerialize() {}
      static size_t getSharedSize() { return 0; }

      template <typename... Args>
      void doCommand(Args&&... args)
      {
         using NullProxy = ControlStatProxy<detail::ControlStatBase>;
         NullProxy{}.doCommand(std::forward<Args>(args)...);
      }
   };

   template <typename TagNode>
   struct GenericOpStat
   {
      using tag_node = TagNode;
      using tag = typename tag_node::tag;

      template <typename... Args>
      void writeVal(Args... args)
      {
         theProxy.writeVal(args...);
      }

      void setSharedPtr(void* ptr)
      {
         theProxy.setSharedPtr(ptr);
      }

      void doSerialize()
      {
         theProxy.serialize();
      }

      static size_t getSharedSize()
      {
         return Proxy::getSharedSize();
      }
   private:
      using Proxy = OperationalStatProxy<typename stat_tag_to_type<tag>::type>;
      Proxy theProxy;
   };


   template <typename TagNode>
   struct GenericControlStat
   {
      using tag_node = TagNode;
      using tag = typename tag_node::tag;

      void setSharedPtr(void* ptr)
      {
         theProxy.setSharedPtr(ptr);
      }
      static size_t getSharedSize()
      {
         return Proxy::getSharedSize();
      }
      template <typename... Args>
      void doCommand(Args&&... args)
      {
         theProxy.doCommand(std::forward<Args>(args)...);
      }

   private:
      using Proxy = ControlStatProxy<typename stat_tag_to_type<tag>::type>;
      Proxy theProxy;
   };
   //+++++++++++++++++++++++++++++++++

   ///////////////

   template <typename T>
   struct is_parent
   {
      //T is of type "struct TagNode"
      //Then T::child_list should be either another sequence (if parent) or void
      using type = typename mpl::is_sequence<typename T::child_list>::type;
      static const bool value = mpl::is_sequence<typename T::child_list>::value;
   };

   template <typename GlobalTagVec, typename ThisTagNode,
             typename IsOpType, typename IsStatType>
   struct stat_log_inserter
   {
      //ThisTagNode is of type "struct TagNode"
      using this_node = typename std::conditional_t<
            IsStatType::value,
            typename std::conditional_t<
               IsOpType::value,
               GenericOpStat<ThisTagNode>,
               GenericControlStat<ThisTagNode>
            >,
            typename std::conditional_t<
               IsOpType::value,
               GenericOpLogger<ThisTagNode>,
               GenericControlLogger<ThisTagNode>
            >
         >;
      //Finally add this stat (or log) to the global tag vec
      using type = typename mpl::push_front<GlobalTagVec, this_node>::type;
   };

   template <typename GlobalTagVec, typename TagHierarchy,
             typename IsOpType, typename IsStatType>
   struct tag_list_creator_helper
   {
      using ChildTagHierarchy = typename TagHierarchy::child_list;
      using this_node = typename std::conditional_t<
            IsStatType::value,
            StatParent<TagHierarchy>,
            typename std::conditional_t<
               IsOpType::value,
               GenericOpLogger<TagHierarchy>,
               GenericControlLogger<TagHierarchy>
            >
         >;

      using UpdatedGlobalTagVec = typename mpl::push_front<
         GlobalTagVec, this_node>::type;

      // _1 == UpdatedGlobalTagVec
      // _2 == the iterator to the child TagNode
      using type = typename mpl::fold<
            ChildTagHierarchy,
            UpdatedGlobalTagVec,
            mpl::eval_if<
               is_parent<mpl::_2>,
               tag_list_creator_helper<
                  mpl::_1, mpl::_2, IsOpType, IsStatType
               >,
               stat_log_inserter<mpl::_1, mpl::_2,
                  IsOpType, IsStatType
               >
            >
         >::type;
   };

   template <typename TagHierarchy,
            bool IsOperational, bool IsStatistic>
   struct tag_list_creator
   {
      using type = typename tag_list_creator_helper<
         mpl::list<>, //Global Tag/Stat list
         TagHierarchy,
         //Need to wrap the booleans to be nice to the MPL algorithms
         typename std::integral_constant<bool, IsOperational>,
         typename std::integral_constant<bool, IsStatistic>
         >::type;
   };

   //This template is used in conjunction with an MPL algorithm
   // with the same semantics as mpl::find_if.
   //BoolFunc is the "condition" metafunction.
   //StatTagFunc is a metafunction that transforms the given
   //   stat_tag into something the algorithm requires.
   //   For example the "Identity" metafunction would work here.
   //StatTagArgs is extra arguments to the BoolFunc
   template <template<typename...> class BoolFunc,
             template<typename...> class StatTagFunc,
             class... StatTagArgs>
   struct tag_node_query
   {
      template<typename TagNode>
      struct apply
      {
         using stat_tag = typename TagNode::tag;
         using type = std::integral_constant
            <
               bool,
               BoolFunc<
                  typename StatTagFunc<stat_tag>::type,
                  StatTagArgs...
               >::value
            >;
      };
   };

   template <typename Tag> using matches_tag
      = tag_node_query<std::is_same, Identity, Tag>;

   template <typename TheList, typename QueryMetaFunc>
   struct is_found_in_list
   {
     static constexpr bool value =
     !std::is_same
        <
           typename boost::mpl::end<TheList>::type,
           typename boost::mpl::find_if
           <
              TheList,
              QueryMetaFunc
           >::type
        >::value;
   };

   template <typename T, typename P, int Depth, typename C = void>
   struct TagNode
   {
      SL_NAME = T::name;
      using tag = T;
      using parent = P;
      static const int depth = Depth;
      using child_list = C;
   };

   template <typename T, typename Parent, typename Depth, class Dummy = void>
   struct GenTagHierarchy
   {
      using type = TagNode<T, Parent, Depth::value>;
   };

   template <typename T, typename Parent, typename Depth>
   struct GenTagHierarchy<T, Parent, Depth,
      typename AlwaysVoid<typename T::children>::type>
   {
      using children = typename T::children;
      using ChildDepth = std::integral_constant<int, Depth::value + 1>;
      using child_type_vec = typename boost::mpl::transform<
         children, GenTagHierarchy<boost::mpl::_, T, ChildDepth>>::type;
      using type = TagNode<T, Parent, Depth::value, child_type_vec>;
   };

   template <typename StatTagTree, typename LogTagTree,
             bool IsOperational, template<typename...> class Derived>
   struct LogStatBase
   {
      using TheDerived = Derived<StatTagTree, LogTagTree>;
      struct TopName
      {
         SL_NAME = "";
      };
      using TopNode = detail::TagNode<TopName, void, 0>;

      using StatTagHierarchy = typename detail::GenTagHierarchy<StatTagTree, TopNode,
            std::integral_constant<int, 0> >::type;
      using LogTagHierarchy = typename detail::GenTagHierarchy<LogTagTree, TopNode,
            std::integral_constant<int, 0> >::type;

      using TheStats = typename boost::fusion::result_of::as_list
      <
         typename detail::tag_list_creator
         <
            StatTagHierarchy,
            IsOperational,
            true
         >::type
      >::type;

      using TheLogs = typename boost::fusion::result_of::as_list
      <
         typename detail::tag_list_creator
         <
            LogTagHierarchy,
            IsOperational,
            false
         >::type
      >::type;

      void init(const std::string& shm_name)
      {
         size_t total_shm_size = 0;
         using namespace boost::fusion;

         auto get_shm_size_lambda = [&total_shm_size](auto& node)
         {
            using NodeType = std::remove_reference_t<decltype(node)>;
            total_shm_size += NodeType::getSharedSize();
         };
         for_each(theStats, get_shm_size_lambda);
         for_each(theLogs, get_shm_size_lambda);
         shm_backend.setParams(shm_name, total_shm_size, IsOperational);

         auto shm_start = shm_backend.getMemoryPtr();
#if 0
         std::cout << std::dec <<  "SHM size = " << total_shm_size
            <<" , shm_start = " << std::hex << (long int)shm_start << std::endl;
#endif
         auto shm_ptr = shm_start;

         //Next, need to inform each node theStats about its location
         // in shared memory
         auto set_shm_pointers = [&shm_ptr](auto& node)
         {
            using NodeType = std::remove_reference_t<decltype(node)>;
            node.setSharedPtr(shm_ptr);
            shm_ptr += NodeType::getSharedSize();
         };
         for_each(theStats, set_shm_pointers);
         for_each(theLogs, set_shm_pointers);

         static_cast<TheDerived*>(this)->doInit();
      }

      void stop()
      {
         static_cast<TheDerived*>(this)->doStop();
      }

      TheStats theStats;
      TheLogs theLogs;
      shared_mem_backend shm_backend;
   };

   template<typename Tag, typename T>
   auto getView(T& tag_node_list)
   {
      return boost::fusion::filter_view<T, detail::matches_tag<Tag>>(tag_node_list);
   }
}
}
