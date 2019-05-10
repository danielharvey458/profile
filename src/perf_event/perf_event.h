#ifndef PERF_EVENT_PERF_EVENT_H
#define PERF_EVENT_PERF_EVENT_H

#include <linux/perf_event.h>
#include <unistd.h>

namespace perf_event
{
  struct PerformanceCounter
  {
    public:

      PerformanceCounter (unsigned type, long long unsigned config, pid_t pid);

      PerformanceCounter (const PerformanceCounter &) = delete;
      PerformanceCounter &operator = (const PerformanceCounter &) = delete;

      PerformanceCounter (PerformanceCounter &&);
      PerformanceCounter &operator = (PerformanceCounter &&);

      ~PerformanceCounter ();

      void enable ();

      void reset ();

      void disable ();

      long get () const;

    private:
      struct perf_event_attr m_pe;
      int m_fd = {};
  };
}

#endif
