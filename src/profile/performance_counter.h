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

#ifndef PROFILE_PERFORMANCE_COUNTER_H
#define PROFILE_PERFORMANCE_COUNTER_H

#include "profile/event.h"

#include <linux/perf_event.h>

namespace profile
{
  /**
   * PerformanceCounter counts performance events measured by
   * perf_event_open
   */
  struct PerformanceCounter
  {
    public:

      /**
       * Construct a counter to measure @p event on process
       * @p pid. If @p pid is set to 0, events in the current process
       * are measured.
       */
      explicit PerformanceCounter (const Event &event, pid_t pid = 0);

      /**
       * Non-copyable.
       */
      PerformanceCounter (const PerformanceCounter &) = delete;
      PerformanceCounter &operator = (const PerformanceCounter &) = delete;

      /**
       * Move construction and assignment
       */
      PerformanceCounter (PerformanceCounter &&);
      PerformanceCounter &operator = (PerformanceCounter &&);

      /**
       * Custom destructor to release system resources used to
       * measure events
       */
      ~PerformanceCounter ();

      /**
       * Start measuring the performance event.
       */
      void start ();

      /**
       * Stop measuring the performance event.
       */
      void stop ();

      /**
       * Reset the internal count of the event to zero
       */
      void reset ();

      /**
       * Get the measurement for this event.
       */
      long get () const;

      /**
       * Get the event being measured.
       */
      Event event () const;

    private:
      long read () const;
      struct perf_event_attr m_pe;
      int m_fd = {};
      long m_last = {};
      long m_count = {};
      static int m_group_leader;
  };
}

#endif
