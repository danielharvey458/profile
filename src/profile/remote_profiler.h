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
  /**
   * RemoteProfiler measure performance counters of a remote process
   * by setting pairs of breakpoints at the beginning and end of
   * sequences of code that are to be profiled, and using the
   * PerformanceCounter class to measure the counters during
   * execution between the breakpoints
   */
  class RemoteProfiler
  {
    public:

      /**
       * CodeBlock represents a block of (assembly) code
       * to be profiled, with a start and end address and a
       * human-readable name
       */
      struct CodeBlock
      {
        std::string name;
        std::intptr_t start = {};
        std::intptr_t end = {};
      };

      /**
       * Construct a RemoteProfiler profiling the profilee process @p pid
       */
      explicit RemoteProfiler (pid_t pid);

      /**
       * Set a measure point to measure the @p event between the addresses
       * @p begin_address and @p end_address.
       *
       * @p name is an arbitrary identifier for the address range.
       *
       * The addresses @p begin_address and @p end_address are automatically
       * adjusted by the virtual memory offset of the profilee
       */
      void set_measure (const Event &event, CodeBlock code_block);

      /**
       * Run the profilee to completion, stopping at each measure
       * point to measure performance counters
       */
      void run ();

    private:

      struct MeasuredBlock
      {
        CodeBlock code_block;
        PerformanceCounter counter;
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

      pid_t m_pid;
      std::intptr_t m_virtual_offset;

      std::unordered_map<std::intptr_t, MeasurePoint> m_measure_points;
      std::deque<MeasuredBlock> m_measured_blocks;
  };
}

#endif
