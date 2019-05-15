#ifndef PERF_EVENT_EXCEPTION_H
#define PERF_EVENT_EXCEPTION_H

#include <stdexcept>

namespace perf_event
{
  #define ASSERT(pred, what)             \
  {                                      \
    if (!(pred))                         \
    {                                    \
      throw std::runtime_error (what);   \
    }                                    \
  }
}

#endif
