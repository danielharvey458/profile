#include <boost/test/unit_test.hpp>

#include "profile/performance_counter.h"

using namespace profile;

BOOST_AUTO_TEST_CASE (test_parse_event_from_string)
{
#define TEST_PARSE(str, exp_type, exp_event)   \
  {                                            \
    const auto e = Event::from_string (str);   \
    BOOST_CHECK_EQUAL (e.type (), exp_type);   \
    BOOST_CHECK_EQUAL (e.event (), exp_event); \
  }

  TEST_PARSE ("perf_count_hw_CPU_CYCLES",
              PERF_TYPE_HARDWARE,
              PERF_COUNT_HW_CPU_CYCLES);

  TEST_PARSE ("perf_count_hw_instructions",
              PERF_TYPE_HARDWARE,
              PERF_COUNT_HW_INSTRUCTIONS);

#undef TEST_PARSE
}
