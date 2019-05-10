#include "perf_event/perf_event.h"
#include "perf_event/exception.h"

#include <asm/unistd.h>
#include <linux/perf_event.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>

#include <iostream>
#include <sys/stat.h>

namespace perf_event
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

  PerformanceCounter::PerformanceCounter (unsigned type, long long unsigned config, pid_t pid)
  {
		memset(&m_pe, 0, sizeof(struct perf_event_attr));
		m_pe.type = type;
		m_pe.size = sizeof(struct perf_event_attr);
		m_pe.config = config;
		m_pe.disabled = 1;
		m_pe.exclude_kernel = 1;
		m_pe.exclude_hv = 1;
    errno = 0; 

		m_fd = perf_event_open (&m_pe, pid, -1, -1, 0);

    ASSERT (m_fd > 0, std::string ("Failed to open perf event, ")
                      + strerror (errno));
  }

  PerformanceCounter::PerformanceCounter (PerformanceCounter &&other)
  {
    m_fd = other.m_fd;
    other.m_fd = -1;
  }

  PerformanceCounter &PerformanceCounter::operator = (PerformanceCounter && other)
  {
    m_fd = other.m_fd;
    other.m_fd = -1;
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

  void PerformanceCounter::reset ()
  {
    errno = 0;
    ASSERT (ioctl (m_fd, PERF_EVENT_IOC_RESET, 0) >= 0,
            std::string ("Failed to reset perf event, ")
            + strerror (errno));
  }

  void PerformanceCounter::disable ()
  {
    errno = 0;
    ASSERT (ioctl (m_fd, PERF_EVENT_IOC_DISABLE, 0) >= 0,
            std::string ("Failed to disable perf event, ")
            + strerror (errno));
  }

  long PerformanceCounter::get () const
  {
    errno = 0;

    long long count = {};

    ASSERT (read (m_fd, &count, sizeof(long long)) >= 0,
            std::string ("Failed to get performance value, ")
            + strerror (errno));

    return count;
  }
}
