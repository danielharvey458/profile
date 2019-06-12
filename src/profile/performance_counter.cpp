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

#include <algorithm>

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
    memset(&m_pe, 0, sizeof(struct perf_event_attr));
    m_pe.type = event.first;
    m_pe.size = sizeof(struct perf_event_attr);
    m_pe.config = event.second;
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

    enable ();
  }

  Event PerformanceCounter::event () const
  {
    return {m_pe.type, m_pe.config};
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

  void PerformanceCounter::enable ()
  {
    errno = 0;
    ASSERT (ioctl (m_fd, PERF_EVENT_IOC_ENABLE, 0) >= 0,
            std::string ("Failed to enable perf event, ")
            + strerror (errno));
  }

  void PerformanceCounter::disable ()
  {
    errno = 0;
    ASSERT (ioctl (m_fd, PERF_EVENT_IOC_DISABLE, 0) >= 0,
            std::string ("Failed to disable perf event, ")
            + strerror (errno));
  }

  void PerformanceCounter::start ()
  {
    m_last = read_file ();
  }

  void PerformanceCounter::stop ()
  {
    m_count += read_file () - m_last;
  }

  void PerformanceCounter::reset ()
  {
    errno = 0;
    ASSERT (ioctl (m_fd, PERF_EVENT_IOC_RESET, 0) >= 0,
            std::string ("Failed to reset perf event, ")
            + strerror (errno));
  }

  long PerformanceCounter::get () const
  {
    return m_count;
  }

  long PerformanceCounter::read_file () const
  {
    errno = 0;

    long long count = {};

    ASSERT (read (m_fd, &count, sizeof(long long)) >= 0,
            std::string ("Failed to get performance value, ")
            + strerror (errno));

    return count;
  }

  namespace
  {
    using EventLookup = std::pair<std::vector<Event>, std::vector<std::string>>;

    const EventLookup &event_lookup ()
    {
      static const auto lookup = []
      {
        EventLookup lookup;

#define ADD_EVENT(type, name)                   \
        lookup.first.emplace_back (type, name); \
        lookup.second.emplace_back (#name);

        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_INSTRUCTIONS);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BRANCH_MISSES);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_BUS_CYCLES);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_FRONTEND);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_STALLED_CYCLES_BACKEND);
        ADD_EVENT(PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES);
#undef ADD_EVENT

        return lookup;
      } ();

      return lookup;
    }
  }

  Event event_from_string (const std::string &event_name)
  {
    const auto &lookup = event_lookup ();

    const auto icompare = [&] (const auto &name)
    {
      return std::equal (name.begin (), name.end (),
                         event_name.begin (), event_name.end (),
                         [] (char a, char b)
                         {
                           return tolower (a) == tolower (b);
                         });
    };

    const auto it = std::find_if (lookup.second.begin (),
                                  lookup.second.end (),
                                  icompare);

    ASSERT (it != lookup.second.end (), "No such event");

    return lookup.first[std::distance (lookup.second.begin (), it)];
  }

  const std::string &event_to_string (const Event &event)
  {
    const auto &lookup = event_lookup ();

    const auto it = std::find (lookup.first.begin (),
                               lookup.first.end (),
                               event);
    ASSERT (it != lookup.first.end (), "No such event");

    return lookup.second[std::distance (lookup.first.begin (), it)];
  }

  const std::vector<std::string> &all_event_names ()
  {
    return event_lookup ().second;
  }

}
