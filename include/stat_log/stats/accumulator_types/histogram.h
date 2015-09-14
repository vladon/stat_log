#pragma once
#include <stat_log/stats/accumulator_types/boost_hist.hpp>
#include <stat_log/stats/accumulator.h>
#include <boost/accumulators/statistics.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/mpl/vector.hpp>
#include <tuple>
#include <array>

namespace stat_log
{
   namespace detail
   {
      template <typename Sample, typename tag_type, int min_range, int max_range, int num_bins>
      struct HistogramGeneric
      {
         HistogramGeneric()
            : acc(boost::accumulators::tag::histogram_count::num_bins = num_bins,
                  boost::accumulators::tag::histogram_count::min_range = min_range,
                  boost::accumulators::tag::histogram_count::max_range = max_range
                 )
         {}

         boost::accumulators::accumulator_set<
            Sample,
            boost::accumulators::stats<tag_type>
         > acc;

         void operator()(Sample sample)
         {
            acc(sample);
         }

         static constexpr std::size_t num_hist_bins = num_bins + 2;
         using sample_type = Sample;
         using features_type = boost::mpl::vector<tag_type>;
      };

   }

   template <typename Sample, int min_range, int max_range, int num_bins = 10>
   using HistogramCount = detail::HistogramGeneric<
      Sample,
      boost::accumulators::tag::histogram_count,
      min_range, max_range, num_bins>;

   template <typename Sample, int min_range, int max_range, int num_bins = 10>
   using HistogramDensity = detail::HistogramGeneric<
      Sample,
      boost::accumulators::tag::histogram_density,
      min_range, max_range, num_bins>;


   namespace accumulator
   {
      //WARNING: the AccumSet CANNOT be a normal boost accumulator_set.
      // It is required to be a specialization of the Histogram template above
      // (I did this because I needed a way to forward the histogram constructor
      // args from the user to the accumulator implementation).
      //
      //TODO: it would be nice to do some sorta static_assert here
      // if the user tries to use a histogram_count or histogram_density statistic tag within
      // a boost accumulator set.
      //
      //I could do something similar to the "is_instantiation_of" check here:
      //http://stackoverflow.com/questions/11251376/how-can-i-check-if-a-type-is-an-instantiation-of-a-given-class-template
      //But, this solution does not work for non-type template parameters so is
      // unusable in my case.

      template <typename AccumSet>
      struct traits<AccumSet, boost::accumulators::tag::histogram_count>
      {
         using shared_type = std::array<
            std::tuple<typename AccumSet::sample_type, std::size_t>, // (bin_start, count in bin)
            AccumSet::num_hist_bins>;
         static void serialize(AccumSet& acc, char* ptr)
         {
            const auto& hist = boost::accumulators::histogram_count(acc.acc);
            auto hist_shared_ptr = reinterpret_cast<shared_type*>(ptr);
            for(std::size_t i = 0; i < hist_shared_ptr->size(); ++i)
            {
               auto& tup = (*hist_shared_ptr)[i];
               std::get<0>(tup) = hist[i].first;
               std::get<1>(tup) = hist[i].second;
            }

         }

         static constexpr size_t size()
         {
            return sizeof(shared_type);
         }

         static constexpr const char* stat_name = "count histogram";

         static void dumpStat(void* ptr)
         {
            auto hist_ptr = reinterpret_cast<shared_type*>(ptr);
            for(std::size_t i = 0; i < hist_ptr->size(); ++i)
            {
               const auto& tup = (*hist_ptr)[i];
               std::cout << "Bin start = " << std::get<0>(tup)
                  << ", Count = " << std::get<1>(tup) << std::endl;
            }
         }
      };

      template <typename AccumSet>
      struct traits<AccumSet, boost::accumulators::tag::histogram_density>
      {
         using shared_type = std::array<
            std::tuple<typename AccumSet::sample_type, double>, // (bin_start, probability)
            AccumSet::num_hist_bins>;
         static void serialize(AccumSet& acc, char* ptr)
         {
            const auto& hist = boost::accumulators::histogram_density(acc.acc);
            auto hist_shared_ptr = reinterpret_cast<shared_type*>(ptr);
            for(std::size_t i = 0; i < hist_shared_ptr->size(); ++i)
            {
               auto& tup = (*hist_shared_ptr)[i];
               std::get<0>(tup) = hist[i].first;
               std::get<1>(tup) = hist[i].second;
            }
         }

         static constexpr size_t size()
         {
            return sizeof(shared_type);
         }

         static constexpr const char* stat_name = "density histogram";

         static void dumpStat(void* ptr)
         {
            auto hist_ptr = reinterpret_cast<shared_type*>(ptr);
            for(std::size_t i = 0; i < hist_ptr->size(); ++i)
            {
               const auto& tup = (*hist_ptr)[i];
               std::cout.precision(5);
               std::cout << "\nBin start = " << std::get<0>(tup)
                  << ", Prob = " << std::get<1>(tup);
            }
         }
      };
   }
}
