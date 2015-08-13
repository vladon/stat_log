#pragma once
#include "fusion_includes.h"

#define NAME static constexpr const char* name
namespace stat_log
{

template <typename T, bool IsOperational>
struct stat_traits;

namespace detail
{
   using namespace boost;
   using namespace boost::fusion;

   template <typename U> struct AlwaysVoid {
      typedef void type;
   };

   template <typename Tag, typename MatchingTags, bool IsOperational>
   struct GenericStat
   {
      void setShmPtr(void* shm_ptr)
      {
         theProxy.setShmPtr(shm_ptr);
      }
      using SerialType = typename stat_traits<Tag, IsOperational>::SerialType;
      using Proxy = typename stat_traits<Tag, IsOperational>::ProxyType;
      Proxy theProxy;
      using matching_tags = MatchingTags;

      auto& getValue()
      {
         return this->theProxy.getValue();
      }
   };

   template <typename Tag, typename MatchingTags>
      struct GenericOpStat : GenericStat<Tag, MatchingTags, true>
   {
      template <typename... Args>
         void writeVal(Args... args)
         {
            this->theProxy.write(args...);
         }

      using BaseClass = GenericStat<Tag, MatchingTags, true>;
      using SerialType = typename BaseClass::SerialType;
   };


   template <typename Tag, typename MatchingTags>
      struct GenericControlStat : GenericStat<Tag, MatchingTags, false>
   {
      using BaseClass = GenericStat<Tag, MatchingTags, false>;
      using SerialType = typename BaseClass::SerialType;
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

         // _1 == GlobalTagVec
         // _2 == the iterator to the child TagNode
         using type = typename mpl::fold<
               ChildTagHierarchy,
               GlobalTagVec,
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

   template <typename UserStatH, bool IsOperational>
      struct LogStatBase
   {
      struct TopName
      {
         NAME = "";
      };
      using TopNode = detail::TagNode<TopName, void, 0>;

      using TagHierarchy = typename detail::GenTagHierarchy<UserStatH, TopNode,
            std::integral_constant<int, 0> >::type;
      using TheStats = typename boost::fusion::result_of::as_vector<
         typename detail::stat_creator<TagHierarchy, IsOperational>::type>::type;

      void assignShmPtr(char* shm_ptr)
      {
         using namespace boost::fusion;
         for_each(theStats, [&shm_ptr](auto& stat)
               {
               using StatType = std::remove_reference_t<decltype(stat)>;
               stat.setShmPtr(shm_ptr);
               std::cout << "assignSmPtr to " << TypeId<StatType>{}
               << std::hex << (long int)shm_ptr << std::endl;
               shm_ptr += StatType::SerialType::getSize();
               });

      }
      TheStats theStats;
   };
}
template<typename Tag, typename T>
auto getStatHandleView(T& stats)
{
   return boost::fusion::filter_view<T, detail::matches_tag<Tag>>(stats);
}

   template <typename Tag, typename T>
auto& getValue(T& stats)
{
   using namespace boost::fusion;
   auto statHdlView = getStatHandleView<Tag>(stats);
   static_assert(result_of::size<decltype(statHdlView)>::value == 1,
         "getValues requires a Leaf Tag!");
   return deref(begin(statHdlView)).getValue();
}
}
