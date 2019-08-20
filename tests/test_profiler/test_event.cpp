/*
 * Copyright 2019 Daniel Harvey
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
