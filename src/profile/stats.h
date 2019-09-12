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

#ifndef PROFILE_STATS_H
#define PROFILE_STATS_H

namespace profile
{
  /**
   * Compute the mean and standard deviation of the range
   * bounded by the iterators using Welford's online method.
   */
  template<typename Iterator>
  std::pair<double, double>
  mean_stdev (Iterator begin, Iterator end);

  /**
   * Range overload of mean_stdev
   */
  template<typename Range>
  std::pair<double, double> mean_stdev (const Range &r);
}

#include "profile/stats.inl"

#endif

