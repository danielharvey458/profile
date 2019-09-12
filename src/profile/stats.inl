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

#include <cmath>

namespace profile
{
  template<typename Iterator>
  std::pair<double, double>
  mean_stdev (Iterator begin, const Iterator end)
  {
    double mean = 0;
    double mn = 0;

    auto n = 1.0;

    const auto size = std::distance (begin, end);

    for (; begin != end; ++begin)
    {
      /*
       * TODO Make this more robust to large numbers
       */
      const auto old_mean = mean;
      mean += (1.0 / n) * (*begin - mean);
      mn += (*begin - old_mean) * (*begin - mean);
      n += 1.0;
    }

    return std::make_pair (mean, std::sqrt (mn / size));
  }

  template<typename Range>
  std::pair<double, double> mean_stdev (const Range &r)
  {
    using std::begin, std::end;
    return mean_stdev (begin (r), end (r));
  }
}
