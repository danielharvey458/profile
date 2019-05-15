#ifndef PT_PROFILE_MEASURE_POINT_H
#define PT_PROFILE_MEASURE_POINT_H

#include "pt_profile/perf_event.h"

namespace pt_profile
{
  struct MeasurePoint
  {
    enum begin_end_t
    {
      BEGIN,
      END
    };

    Breakpoint breakpoint = {};
    begin_end_t begin_end = {};
    PerformanceCounter* counter = {};
  };
}

#endif
