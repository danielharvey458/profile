#ifndef PROFILE_PERF_EVENT_H
#define PROFILE_PERF_EVENT_H

#include <linux/perf_event.h>
#include <unistd.h>

#include <string>
#include <tuple>
#include <vector>

namespace profile
{
  using Event = std::pair<unsigned, long long unsigned>;

  struct PerformanceCounter
  {
    public:

      explicit PerformanceCounter (const Event &event, pid_t pid);

      PerformanceCounter (const PerformanceCounter &) = delete;
      PerformanceCounter &operator = (const PerformanceCounter &) = delete;

      PerformanceCounter (PerformanceCounter &&);
      PerformanceCounter &operator = (PerformanceCounter &&);

      ~PerformanceCounter ();

      void start ();

      void stop ();

      void reset ();

      long get () const;

      Event event () const;

    private:
      struct perf_event_attr m_pe;
      int m_fd = {};
      static int m_group_leader;
  };


  Event event_from_string (const std::string &event_name);

  const std::string &event_to_string (const Event &event);

  const std::vector<std::string> &all_event_names ();
}

#endif
