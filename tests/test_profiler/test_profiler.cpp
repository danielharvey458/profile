#include "profile/performance_counter.h"

#include <iostream>

int main ()
{
  using namespace profile;

  PerformanceCounter count_instructions (
    event_from_string ("perf_count_hw_instructions"), -1);

  /*
   * Do some work.
   */
  auto x = 0.;
  for (auto i = 0; i < 1<<10; ++i)
  {
    x += rand ();
  }

  std::cout << count_instructions.get () << std::endl;
  return x;
}
