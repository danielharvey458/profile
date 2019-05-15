#ifndef PT_PROFILE_DEBUGGER_H
#define PT_PROFILE_DEBUGGER_H

#include "pt_profile/breakpoint.h"
#include "pt_profile/measure_point.h"
#include "pt_profile/perf_event.h"

#include <list>
#include <string>
#include <unordered_map>

#include <cstdint>
#include <sys/types.h>

namespace pt_profile
{
  class Debugger
  {
    public:

      explicit Debugger (std::string program_name, pid_t pid);

      void run ();

      void set_measure (std::intptr_t begin_address,
                        std::intptr_t end_address);

    private:

      bool continue_execution ();
      void handle_command (const std::string &command);
      void write_memory (uint64_t address, uint64_t value);
      uint64_t read_memory (uint64_t address) const;
      uint64_t get_pc () const;
      void set_pc (int64_t pc);
      void step_over_breakpoint ();
      bool wait_for_signal ();


      std::string m_program_name;
      pid_t m_pid;
      std::intptr_t m_virtual_offset;
      std::unordered_multimap<std::intptr_t, MeasurePoint> m_measure_points;
      std::list<PerformanceCounter> m_counters;
  };
}

#endif
