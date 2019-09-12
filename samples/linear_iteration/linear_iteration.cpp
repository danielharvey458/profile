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

#include "profile/do_not_optimize.h"
#include "profile/performance_counter.h"
#include "profile/profiler.h"
#include "profile/stats.h"

#include <algorithm>
#include <cmath>
#include <deque>
#include <iostream>
#include <list>
#include <map>
#include <numeric>
#include <set>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace profile
{
  template<typename Container>
  void iterate_read (Container &c)
  {
    for (auto &x : c)
    {
      do_not_optimize (x);
    }
  }

  auto make_work_factory ()
  {
    return std::make_tuple
    (
      [] (size_t n)
      {
        return std::tuple ("std::vector", std::vector<int64_t> (n));
      },
      [] (size_t n)
      {
        return std::tuple ("std::deque", std::deque<int64_t> (n));
      },
      [] (size_t n)
      {
        return std::tuple ("std::list", std::list<int64_t> (n));
      },
      [] (size_t n)
      {
        std::map<int32_t, int32_t> m;
        for (auto i = 0u; i < n; ++i)
        {
          m[i];
        }
        return std::tuple ("std::map", std::move (m));
      },
      [] (size_t n)
      {
        std::set<int64_t> s;
        for (auto i = 0u; i < n; ++i)
        {
          s.insert (i);
        }
        return std::tuple ("std::set", std::move (s));
      },
      [] (size_t n)
      {
        std::unordered_set<int64_t> s;
        for (auto i = 0u; i < n; ++i)
        {
          s.insert (i);
        }
        return std::tuple ("std::unordered_set", std::move (s));
      },
      [] (size_t n)
      {
        std::multiset<int64_t> s;
        for (auto i = 0u; i < n; ++i)
        {
          s.insert (i);
        }
        return std::tuple ("std::multiset", std::move (s));
      },
      [] (size_t n)
      {
        std::unordered_map<int32_t, int32_t> m;
        for (auto i = 0u; i < n; ++i)
        {
          m[i];
        }
        return std::tuple ("std::unordered_map", std::move (m));
      },
      [] (size_t n)
      {
        std::multimap<int32_t, int32_t> m;
        for (auto i = 0u; i < n; ++i)
        {
          m.insert (std::make_pair (i, 0));
        }
        return std::tuple ("std::multimap", std::move (m));
      }
    );
  }
}

using namespace profile;

int main ()
{

  std::vector<PerformanceCounter> counters;
  counters.emplace_back (
      Event (PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES));
  counters.emplace_back (
      Event (PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES));
  counters.emplace_back (
      Event (PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES));
  counters.emplace_back (
      Event (PERF_TYPE_SOFTWARE, PERF_COUNT_SW_CPU_CLOCK));

  Sampler sampler (counters, 100);

  auto && work = make_work_factory ();

  std::cout << "name,size";

  for (const auto &counter : counters)
  {
    auto name = counter.event ().str ();
    name = std::string (name.begin () + 11, name.end ());
    std::transform (
          name.begin (), name.end (), name.begin (), &tolower);

    std::cout << "," << name << "/mu," << name << "/sigma";
  }
  std::cout << std::endl;

  const auto evaluate = [&] (auto && f)
  {
    for (auto exp = 15; exp < 18; ++exp)
    {
      const auto tuple = f (1 << exp);
      const auto &[name, container] = tuple;
      const auto result = sampler ([&] {iterate_read (container);});

      std::cout << name << "," << (1 << exp);

      for (const auto &[event, samples] : result)
      {
        (void)event;
        const auto [m,s] = mean_stdev (samples);

        std::cout << "," << m << "," << s;
      }
      std::cout << std::endl;
    }

    return 1;
  };

  std::apply ([&] (auto &&...x) {std::tuple (evaluate (x)...);}, work);
}
