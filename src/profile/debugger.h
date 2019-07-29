#ifndef PROFILE_DEBUGGER_H
#define PROFILE_DEBUGGER_H

#include "profile/breakpoint.h"
#include "profile/performance_counter.h"

#include <deque>
#include <string>
#include <unordered_map>
#include <vector>

#include <cstdint>
#include <sys/types.h>

namespace profile
{
  class Debugger
  {
    public:

      explicit Debugger (std::string program_name, pid_t pid);

      void run ();

      void set_measure (const Event &event,
                        std::intptr_t begin_address,
                        std::intptr_t end_address,
                        const std::string &name);

    private:

      struct MeasuredBlock
      {
        std::intptr_t start = {};
        std::intptr_t end = {};
        PerformanceCounter counter;
        std::string name;
      };

      struct MeasurePoint
      {
        detail::Breakpoint breakpoint;
        std::vector<MeasuredBlock*> measured_blocks;
      };

      bool continue_execution ();
      std::intptr_t get_pc () const;
      void set_pc (std::intptr_t pc);
      void step_over_breakpoint ();
      bool wait_for_signal ();
      void insert_measure_point (std::intptr_t address,
                                 MeasuredBlock *block);

      std::string m_program_name;
      pid_t m_pid;
      std::intptr_t m_virtual_offset;

      std::unordered_map<std::intptr_t, MeasurePoint> m_measure_points;
      std::deque<MeasuredBlock> m_measured_blocks;
  };
}

#endif
