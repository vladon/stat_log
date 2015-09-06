#pragma once
#include <stat_log/parsers/parser_common.h>
#include <stat_log/stats/stats_common.h>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/arg.hpp>
#include <boost/mpl/accumulate.hpp>
#include <boost/fusion/include/as_list.hpp>

#include <iostream>
#include <cstring>
#include <iomanip>
#include <ios>
#include <mutex>

namespace stat_log
{

template <typename AccumSet>
struct Accumulator {};

namespace accumulator
{

   //ACCUMULATOR TRAITS:
   //For each statistic type the user defines, they must also provide a mapping
   //so the library knowns how to serialize the stat into shared memory.
   //For example, suppose the _mean_ stat is to be used. In this case, the user
   //must provide something like (where AccumulatorSetType is their definition
   //of accumulator_set<..>):
   //
   //   template <>
   //   struct traits<AccumulatorSetType, boost::accumulator::tag::mean>
   //   {
   //       using SharedType = typename AccumulatorSetType::sample_type;
   //       static void serialize(AccumulatorSetType& acc, char* ptr)
   //       {
   //          *reinterpret_cast<SharedType*>(ptr) = boost::accumulator::mean(acc);
   //       }
   //
   //       static constexpr size_t size()
   //       {
   //          return sizeof(SharedType);
   //       }
   //
   //       static const char* const stat_name = "mean";
   //       static dumpStat(void* ptr)
   //       {
   //          std::cout << *reinterpret_cast<SharedType*>(ptr);
   //       }
   //   };
   template <typename AccumStatType, typename Tag>
   struct traits;
}

namespace detail
{
   template <typename AccumSet, typename Tag>
   struct accum_trait_holder
   {
      using type = accumulator::traits<AccumSet, Tag>;
   };

   struct plus_wrapper
   {
      template <typename T1, typename T2>
      struct apply
      {
         using type = typename boost::mpl::plus<T1,
                    boost::mpl::int_<T2::size()>>::type;
      };
   };

   template <typename AccumSet>
   struct AccumBase
   {
      //features_type is a MPL list of features.
      using features_type = typename AccumSet::features_type;

      //feature_handlers_t is a MPL-transformed list derived
      // from the features_type list --  accumulator::traits is
      // used in the transformation function.
      using feature_handlers_t = typename boost::mpl::transform
         <features_type, accum_trait_holder<AccumSet, boost::mpl::_1>>::type;

      //feature_handlers is simply a boost::fusion representation of
      // feature_handlers_t.
      using feature_handlers =
         typename boost::fusion::result_of::as_list<feature_handlers_t>::type;

      //control word is a flag shared between the control and operational
      // modes.  Its current purpose is to simply flag that the accumulator
      // should be reset.
      using control_word = int;

      //Define -- using MPL magic -- an array with a size equal to the
      // sum of the feature handlers' shared_types _plus_ the size of
      // the control word.
      using SharedType = std::array<char,
                              boost::mpl::accumulate<
                                   feature_handlers_t
                                   , boost::mpl::int_<0>
                                   , plus_wrapper
                                >::type::value
                                + sizeof(control_word)
                         >;
   };

   template <typename AccumSet>
   struct AccumOp : AccumBase<AccumSet>
   {
      using BaseClass = AccumBase<AccumSet>;
      using sample_type = typename AccumSet::sample_type;
      using SharedType = typename BaseClass::SharedType;
      using feature_handlers = typename BaseClass::feature_handlers;
      using control_word = typename BaseClass::control_word;

      //Called via the deferred processing thread
      void serialize(void* ptr)
      {
         std::unique_lock<std::mutex> lock(mtx);
         //Visit each of the features' serialize method.
         using namespace boost::fusion;
         auto shm_ptr = reinterpret_cast<char*>(ptr);
         for_each(feature_handlers{}, [&](auto& feature_handler)
         {
            using FeatureHandlerType = std::remove_reference_t<decltype(feature_handler)>;
            FeatureHandlerType::serialize(acc, shm_ptr);
            shm_ptr += FeatureHandlerType::size();
         });
      }

      void write(void* shared_ptr, sample_type sample)
      {
         std::unique_lock<std::mutex> lock(mtx);
         auto ptr = reinterpret_cast<char*>(shared_ptr);
         ptr += sizeof(SharedType) - sizeof(control_word);
         auto control_word_ptr = reinterpret_cast<control_word*>(ptr);

         //Check the control word to see if we need to reset the
         // accumulator.
         if(*control_word_ptr)
         {
            acc = AccumSet{};
            *control_word_ptr = 0;
         }
         //Add this sample to the accumulator
         acc(sample);
      }

   private:
      AccumSet acc;
      std::mutex mtx;
   };

   template <typename AccumSet>
   struct AccumControl : AccumBase<AccumSet>
   {
      using BaseClass = AccumBase<AccumSet>;
      using SharedType = typename BaseClass::SharedType;
      using feature_handlers = typename BaseClass::feature_handlers;
      using control_word = typename BaseClass::control_word;

      static void doStatCommand(
            void* shared_ptr,
            StatCmd cmd,
            boost::any& arg,
            const TagInfo& tag_info,
            bool is_substat,
            const std::vector<std::string>& enumNames,
            const std::vector<std::string>& dimensionNames,
            int dimension_idx)
      {
         using namespace boost::fusion;

         auto ptr = reinterpret_cast<char*>(shared_ptr);
         if(!is_substat)
            printHeader(cmd, tag_info);
         //TODO: handle all commands
         if(cmd == StatCmd::DUMP_STAT)
         {
            size_t max_width = 0;
            size_t num_fields = 0;
            for_each(feature_handlers{}, [&](auto feature_handler)
            {
               using FeatureHandlerType = decltype(feature_handler);
               auto len = std::strlen(FeatureHandlerType::stat_name);
               num_fields += 1;
               max_width = std::max(max_width, len);
            });

            size_t max_pad = max_width + 2;
            size_t total_width = num_fields * max_pad;

            std::cout << '\n';
            const auto orig_flags = std::cout.flags();
            std::cout.flags(std::ios::left);
            for_each(feature_handlers{}, [&](auto feature_handler)
            {
               using FeatureHandlerType = decltype(feature_handler);
               std::cout << std::setw(max_pad) << FeatureHandlerType::stat_name;
            });
            std::cout << std::setfill('-') << std::setw(total_width) << '\n' << std::endl;
            std::cout << std::setfill(' ') ;
            for_each(feature_handlers{}, [&](auto feature_handler)
            {
               using FeatureHandlerType = decltype(feature_handler);
               std::cout.width(max_pad);
               std::cout.precision(max_width);
               FeatureHandlerType::dumpStat(ptr);
               ptr += FeatureHandlerType::size();
            });
            std::cout.flags(orig_flags);
            std::cout << std::endl;
         }
         else if(cmd == StatCmd::PRINT_STAT_TYPE)
         {
            std::cout << "Accumulator:";
            for_each(feature_handlers{}, [&](auto feature_handler)
            {
                  using FeatureHandlerType = decltype(feature_handler);
                  std::cout << " " << FeatureHandlerType::stat_name;
            });
            std::cout << std::endl;
         }
         else if(cmd == StatCmd::CLEAR_STAT)
         {
            ptr += sizeof(SharedType) - sizeof(control_word);
            auto control_word_ptr = reinterpret_cast<control_word*>(ptr);
            *control_word_ptr = 1;
         }
         if(!is_substat)
            printHeader(cmd, tag_info);
      }
   };

   template <typename AccumSet>
   struct stat_type_to_impl<Accumulator<AccumSet>, true>
   {
      using type = AccumOp<AccumSet>;
   };

   template <typename AccumSet>
   struct stat_type_to_impl<Accumulator<AccumSet>, false>
   {
      using type = AccumControl<AccumSet>;
   };
}

template <typename AccumSet>
struct is_serialization_deferred<Accumulator<AccumSet>>
{
   static constexpr bool value = true;
};

template <typename AccumSet>
struct is_serialization_deferred<detail::AccumOp<AccumSet>>
{
   static constexpr bool value = true;
};

}

