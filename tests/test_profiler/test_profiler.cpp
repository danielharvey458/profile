#define BOOST_TEST_MODULE test_profiler

#include "profile/performance_counter.h"

#include <boost/test/included/unit_test.hpp>

#include <numeric>
#include <iostream>

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
  std::pair<double, double> mean_stdev (const std::vector<T> &ts)
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
}

/*
 * Test we get consistent profiling results
 */
BOOST_AUTO_TEST_CASE (test_profiler_basic_consistency)
{
  using namespace profile;

  PerformanceCounter counter (
    Event::from_string ("perf_count_hw_instructions"), 0);

  const auto profile = [&] (int n)
  {
    counter.start ();
    do_not_optimize (count_random (n));
    counter.stop ();
    return counter.get ();
  };

  const auto run_benchmark = [&] (int n)
  {
    counter.reset ();
    std::vector<long> counts;
    for (auto i = 0; i < 100; ++i)
    {
      counts.push_back (profile (n));
    }
    std::adjacent_difference (
      counts.begin (), counts.end (), counts.begin ());

    return mean_stdev (counts);
  };

  /*
   * Check that the variance of the measured instructions is low
   */
  BOOST_CHECK_LE (run_benchmark (1 << 10).second, 1.0);
  BOOST_CHECK_LE (run_benchmark (1 << 11).second, 1.0);

  /*
   * Check that the relationship between the means
   * is as expected for simple counting functions.
   */
  BOOST_CHECK_CLOSE (run_benchmark (1 << 11).first,
                     run_benchmark (1 << 10).first * 2.0,
                     1e-1);
}
