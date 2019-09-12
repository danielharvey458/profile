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

#ifndef PROFILE_PROFILER_H
#define PROFILE_PROFILER_H

#include "profile/performance_counter.h"

#include <map>

namespace profile
{
  /**
   * Profile executes an expression whilst counting
   * a single performance counter, returning
   * the elapsed value of the counter while the
   * expression was being invoked
   */
  class Profile
  {
    public:

    /**
     * Construct a Profile for a given range of
     * PerformanceCounters
     */
    explicit Profile (std::vector<PerformanceCounter> &counters);

    /**
     * Count the PerformanceCounters while executing
     * @p expr
     */
    template<typename Expr>
    auto operator () (Expr &&expr) const;

    private:

    std::vector<PerformanceCounter> &m_counters;
  };

  /**
   * Sampler runs N 'profiles' of a given expression
   * and collects the elapsed counts in a vector.
   */
  class Sampler
  {
    public:

    /**
     * Construct a Sampler for a given range of counters
     * and number of samples
     */
    explicit Sampler (std::vector<PerformanceCounter> &counters, size_t n);

    /**
     * Sample the profile of @p expr
     */
    template<typename Expr>
    auto operator () (Expr &&expr) const;

    private:

    std::vector<PerformanceCounter> &m_counters;
    size_t m_n;
  };
}

#include "profile/profiler.inl"

#endif
