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

#include <algorithm>

namespace profile
{
  template<typename Expr>
  auto Profile::operator () (Expr &&expr) const
  {
    std::for_each (
      m_counters.begin (), m_counters.end (),
      [] (auto &c) {c.reset ();});

    std::for_each (
      m_counters.begin (), m_counters.end (),
      [] (auto &c) {c.start ();});

    std::forward<Expr> (expr) ();

    std::for_each (
      m_counters.begin (), m_counters.end (),
      [] (auto &c) {c.stop ();});

    std::map<Event, long> result;

    for (const auto &counter : m_counters)
    {
      result[counter.event ()] = counter.get ();
    }

    return result;
  }

  template<typename Expr>
  auto Sampler::operator () (Expr &&expr) const
  {
    Profile profile (m_counters);

    std::map<Event, std::vector<long>> samples;

    for (auto i = 0u; i < m_n; ++i)
    {
      for (const auto &[e, c]
           : profile (std::forward<Expr> (expr)))
      {
        samples[e].push_back (c);
      }
    }

    return samples;
  }
}
