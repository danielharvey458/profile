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

#include "profile/stats.h"

using namespace profile;

BOOST_AUTO_TEST_CASE (test_mean_stdev)
{
  std::vector<int> rng {1, 2, 3};

  auto [mean, std] = mean_stdev (rng);
  BOOST_CHECK_CLOSE (mean, 2.0, 1e-2);
  BOOST_CHECK_CLOSE (std, 0.8165, 1e-2);

  std::vector<unsigned> med {UINT_MAX - 2, UINT_MAX - 1, UINT_MAX};
  std::tie (mean, std) = mean_stdev (med);

  BOOST_CHECK_CLOSE (mean, UINT_MAX - 1, 1e-2);
  BOOST_CHECK_CLOSE (std, 0.8165, 1e-2);

  std::vector<size_t> lrg {ULLONG_MAX - 2, ULLONG_MAX - 1, ULLONG_MAX};
  std::tie (mean, std) = mean_stdev (lrg);

  /*
   * Error in standard deviation caused by large numbers not precisely
   * representable in doubles
   */
  BOOST_CHECK (static_cast<double> (ULLONG_MAX)
               == static_cast<double> (ULLONG_MAX - 1));
  BOOST_CHECK_CLOSE (mean, ULLONG_MAX - 1, 1e-2);
  BOOST_CHECK_CLOSE (std, 0, 1e-2);
}
