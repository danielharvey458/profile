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

#include "profile/performance_counter.h"
#include "profile/exception.h"

#include <asm/unistd.h>
#include <linux/perf_event.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>

#include <sys/stat.h>

namespace profile
{
  namespace
  {
    long perf_event_open (struct perf_event_attr *hw_event, pid_t pid,
                          int cpu, int group_fd, unsigned long flags)
    {
      int ret;
      ret = syscall(__NR_perf_event_open, hw_event, pid, cpu,
                     group_fd, flags);
      return ret;
    }
  }

  int PerformanceCounter::m_group_leader = -1;

  PerformanceCounter::PerformanceCounter (const Event &event, pid_t pid)
  {
    memset(&m_pe, 0, sizeof (struct perf_event_attr));
    m_pe.type = event.type ();
    m_pe.size = sizeof (struct perf_event_attr);
    m_pe.config = event.event ();
    m_pe.disabled = 1;
    m_pe.exclude_kernel = 1;
    m_pe.exclude_hv = 1;
    m_pe.exclude_idle = 1;

    errno = 0; 

    m_fd = perf_event_open (&m_pe, pid, -1, m_group_leader, 0);

    if (m_group_leader == -1)
    {
      m_group_leader = m_fd;
    }

    ASSERT (m_fd > 0, std::string ("Failed to open perf event, ")
                      + strerror (errno));

    errno = 0;
    ASSERT (ioctl (m_fd, PERF_EVENT_IOC_ENABLE, 0) >= 0,
            std::string ("Failed to enable perf event, ")
            + strerror (errno));
  }

  Event PerformanceCounter::event () const
  {
    return Event (m_pe.type, m_pe.config);
  }

  PerformanceCounter::PerformanceCounter (PerformanceCounter &&other)
  {
    m_fd = other.m_fd;
    other.m_fd = -1;
    m_pe = other.m_pe;
  }

  PerformanceCounter &
  PerformanceCounter::operator = (PerformanceCounter && other)
  {
    m_fd = other.m_fd;
    other.m_fd = -1;
    m_pe = other.m_pe;
    return *this;
  }

  PerformanceCounter::~PerformanceCounter ()
  {
    close (m_fd);
  }

  void PerformanceCounter::start ()
  {
    m_last = read ();
  }

  void PerformanceCounter::stop ()
  {
    m_count += read () - m_last;
  }

  void PerformanceCounter::reset ()
  {
    m_count = 0;
    m_last = read ();
  }

  long PerformanceCounter::get () const
  {
    return m_count;
  }

  long PerformanceCounter::read () const
  {
    errno = 0;

    long long count = {};

    ASSERT (::read (m_fd, &count, sizeof (long long)) >= 0,
            std::string ("Failed to get counter value, ")
            + strerror (errno));

    return count;
  }
}
