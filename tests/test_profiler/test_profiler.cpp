#define BOOST_TEST_MODULE test_profiler

#include "profile/performance_counter.h"

#include <boost/test/included/unit_test.hpp>

#include <iostream>

BOOST_AUTO_TEST_CASE (test_profiler_basic)
{
  using namespace profile;

  PerformanceCounter count_instructions (
    event_from_string ("perf_count_hw_instructions"), 0);

  auto x = 0.;
  for (auto i = 0; i < 1<<10; ++i)
  {
    x += rand ();
  }

  count_instructions.stop ();

  /*
   * Check that we count some instructions
   */
  BOOST_CHECK_GE (count_instructions.get (), 0);
}
