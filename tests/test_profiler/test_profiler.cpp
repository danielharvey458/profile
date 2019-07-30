#define BOOST_TEST_MODULE test_profiler

#include "profile/performance_counter.h"

#include <boost/test/included/unit_test.hpp>

#include <numeric>
#include <iostream>

using namespace profile;

namespace
{
  template<typename T>
  void do_not_optimize (const T &t)
  {
    asm volatile ("" : : "r,m"(t) : "memory");
  }

  double count_random (int n)
  {
    auto x = 0.;

    for (auto i = 0; i < n; ++i)
    {
      x += rand ();
    }

    return x;
  }

  template<typename T>
  std::vector<std::vector<T>>
  transpose (const std::vector<std::vector<T>> &input)
  {
    if (input.empty ())
    {
      return {};
    }

    const auto nrows = input.size ();
    const auto ncols = input.front ().size ();

    std::vector<std::vector<T>> output (
        ncols, std::vector<T> (nrows, T {}));

    for (auto i = 0u; i < nrows; ++i)
    {
      for (auto j = 0u; j < ncols; ++j)
      {
        output[j][i] = input[i][j];
      }
    }

    return output;
  }

  struct MeanStdev
  {
    double mean = {};
    double stdev = {};
  };

  template<typename T>
  MeanStdev
  mean_stdev (const std::vector<T> &ts)
  {
    const auto n = static_cast<double> (ts.size ());

    if (!n)
    {
      return {{}, {}};
    }

    const auto mean
      = std::accumulate (ts.begin (), ts.end (), 0.) / n;

    const auto variance
      = std::accumulate (ts.begin (), ts.end (), 0.,
          [mean] (auto &lhs, auto &rhs)
          {
            return lhs + (rhs - mean) * (rhs - mean);
          }) / n;

    return {mean, std::sqrt (variance)};
  }

  std::vector<MeanStdev>
  mean_stdev (const std::vector<std::vector<long>> &input)
  {
    const auto transposed = transpose (input);

    std::vector<MeanStdev> output (transposed.size ());

    std::transform (
        transposed.begin (),
        transposed.end (),
        output.begin (),
        &mean_stdev<long>
    );

    return output;
  }

  struct CountRandomFixture
  {
    std::vector<PerformanceCounter> counters;

    std::vector<long> profile (int n)
    {
      std::for_each (counters.begin (), counters.end (),
                     std::mem_fn (&PerformanceCounter::reset));

      std::for_each (counters.begin (), counters.end (),
                     std::mem_fn (&PerformanceCounter::start));

      do_not_optimize (count_random (n));

      std::for_each (counters.begin (), counters.end (),
                     std::mem_fn (&PerformanceCounter::stop));

      std::vector<long> result (counters.size ());

      std::transform (counters.begin (), counters.end (),
                      result.begin (),
                      std::mem_fn (&PerformanceCounter::get));
      return result;
    }

    std::vector<MeanStdev>
    run_benchmark (int n)
    {
      static const int nruns = 100;

      std::for_each (counters.begin (), counters.end (),
                     std::mem_fn (&PerformanceCounter::reset));

      std::vector<std::vector<long>> counts;
      counts.reserve (nruns);

      for (auto i = 0; i < nruns; ++i)
      {
        counts.emplace_back (profile (n));
      }

      return mean_stdev (counts);
    }
  };
}

/*
 * Test we get consistent profiling results
 */
BOOST_FIXTURE_TEST_CASE (test_profiler_basic_consistency,
                         CountRandomFixture)
{
  counters.emplace_back (
      PerformanceCounter (
        Event::from_string ("perf_count_hw_instructions"), 0));

  /*
   * Check that the variance of the measured instructions is low
   */
  BOOST_REQUIRE_EQUAL (run_benchmark (1 << 10).size (), 1ul);
  BOOST_CHECK_LE (run_benchmark (1 << 10).front ().stdev, 1.0);
  BOOST_CHECK_LE (run_benchmark (1 << 11).front ().stdev, 1.0);

  /*
   * Check that the relationship between the means
   * is as expected for simple counting functions.
   */
  BOOST_CHECK_CLOSE (run_benchmark (1 << 11).front ().mean,
                     run_benchmark (1 << 10).front ().mean * 2.0,
                     10 /*pct*/);

  /*
   * Check two counters counting the same event give consistent
   * results.
   */
  counters.emplace_back (
      PerformanceCounter (
        Event::from_string ("perf_count_hw_instructions"), 0));

  const auto results = run_benchmark (1 << 11);

  BOOST_REQUIRE_EQUAL (results.size (), 2ul);
  BOOST_CHECK_CLOSE (results.at (0).mean, results.at (1).mean, 1.0);
}
