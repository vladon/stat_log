#pragma once
#include <stat_log/util/command.h>
#include <stat_log/stats/stats_common.h>
#include <stat_log/stats/accumulator_types/accumulator_common.h>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/arg.hpp>
#include <boost/mpl/plus.hpp>
#include <boost/mpl/accumulate.hpp>
#include <boost/fusion/include/as_list.hpp>

#include <iostream>
#include <cstring>
#include <iomanip>
#include <ios>
#include <mutex>
#include <array>

namespace stat_log
{

template <typename AccumSet>
struct Accumulator {};

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
      using shared_type = std::array<char,
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
      using shared_type = typename BaseClass::shared_type;
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
         ptr += sizeof(shared_type) - sizeof(control_word);
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
      using sample_type = typename AccumSet::sample_type;
      using shared_type = typename BaseClass::shared_type;
      using feature_handlers = typename BaseClass::feature_handlers;
      using control_word = typename BaseClass::control_word;

      static void doStatCommand(
            void* shared_ptr,
            StatCmd cmd,
            boost::any& arg,
            StatCmdOutput& stat_output)
      {
         using namespace boost::fusion;

         auto ptr = reinterpret_cast<char*>(shared_ptr);
         std::stringstream ss_title;
         std::stringstream ss_entry;
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

            size_t max_pad = max_width + 4;

            ss_title.flags(std::ios::left);
            ss_entry.flags(std::ios::left);

            for_each(feature_handlers{}, [&](auto feature_handler)
            {
               using FeatureHandlerType = decltype(feature_handler);
               ss_title.width(max_pad);
               FeatureHandlerType::getTitle(ptr, ss_title);
               ptr += FeatureHandlerType::size();
            });
            ptr = reinterpret_cast<char*>(shared_ptr);
            for_each(feature_handlers{}, [&](auto feature_handler)
            {
               using FeatureHandlerType = decltype(feature_handler);
               if(num_fields > 1)
               {
                  ss_entry.width(max_pad);
                  ss_entry.precision(max_width);
               }
               FeatureHandlerType::dumpStat(ptr, ss_entry);
               ptr += FeatureHandlerType::size();
            });
            stat_output.entryTitle = ss_title.str();
            stat_output.entries.push_back(ss_entry.str());
         }
         else if(cmd == StatCmd::PRINT_STAT_TYPE)
         {
            ss_title << "Accumulator:";
            for_each(feature_handlers{}, [&](auto feature_handler)
            {
                  using FeatureHandlerType = decltype(feature_handler);
                  ss_title << " " << FeatureHandlerType::stat_name;
            });
            stat_output.entryTitle = ss_title.str();
         }
         else if(cmd == StatCmd::CLEAR_STAT)
         {
            ptr += sizeof(shared_type) - sizeof(control_word);
            auto control_word_ptr = reinterpret_cast<control_word*>(ptr);
            *control_word_ptr = 1;
         }
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

}

