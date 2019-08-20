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

#include "profile/registers.h"
#include "profile/remote_profiler.h"

#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>

#include <algorithm>
#include <fstream>
#include <iostream>

namespace profile
{
  namespace
  {
    /*
     * Compute the virtual offset for the address space of a given
     * process of /proc/PID/maps
     */
    std::intptr_t get_virtual_offset (pid_t pid)
    {
      std::ifstream stream ("/proc/" + std::to_string (pid) + "/maps");

      std::string line;

      if (!std::getline (stream, line, '-'))
      {
        throw std::runtime_error ("Couldn't find virtual address offset");
      }

      return std::stol (line.c_str (), nullptr, 16);
    }

    uint64_t get_register (pid_t pid, Registers::Register reg)
    {
      user_regs_struct regs;
      ptrace (PTRACE_GETREGS, pid, nullptr, &regs);

      const auto it = Registers::from_register (reg);

      return *Registers::lookup_user_regs (regs, it);
    }

    void set_register (pid_t pid, Registers::Register reg, uint64_t value)
    {
      user_regs_struct regs;
      ptrace (PTRACE_GETREGS, pid, nullptr, &regs);

      const auto it = Registers::from_register (reg);

      *Registers::lookup_user_regs (regs, it) = value;

      ptrace (PTRACE_SETREGS, pid, nullptr, &regs);
    }
  }

  RemoteProfiler::RemoteProfiler (const pid_t pid)
    : m_pid (pid),
      m_virtual_offset (get_virtual_offset (m_pid))
  {
  }

  void RemoteProfiler::set_measure (const Event &event,
                                    CodeBlock code_block)
  {
    const auto perf_counter_it
      = m_measured_blocks.emplace (
          m_measured_blocks.end (),
          MeasuredBlock
          {
            std::move (code_block),
            PerformanceCounter {event, m_pid}
          });

    insert_measure_point (perf_counter_it->code_block.start, &*perf_counter_it);
    insert_measure_point (perf_counter_it->code_block.end, &*perf_counter_it);
  }

  std::intptr_t RemoteProfiler::get_pc () const
  {
    return get_register (m_pid, Registers::RIP);
  }

  void RemoteProfiler::set_pc (const std::intptr_t pc)
  {
    set_register (m_pid, Registers::RIP, pc);
  }

  bool RemoteProfiler::wait_for_signal ()
  {
    auto wait_status = 0;
    auto options = 0;
    waitpid (m_pid, &wait_status, options);
    return !WIFEXITED (wait_status);
  }

  void RemoteProfiler::insert_measure_point (const std::intptr_t address,
                                             MeasuredBlock *block)
  {
    auto &measure_point
      = m_measure_points.emplace (
          address,
          MeasurePoint
          {
            detail::Breakpoint
            {
              m_pid, address + m_virtual_offset
            }
          }).first->second;

    measure_point.measured_blocks.push_back (block);
    measure_point.breakpoint.enable ();
  }

  bool RemoteProfiler::continue_execution ()
  {
    step_over_breakpoint ();
    ptrace (PTRACE_CONT, m_pid, nullptr, nullptr);
    return wait_for_signal ();
  }

  void RemoteProfiler::step_over_breakpoint ()
  {
    const auto breakpoint = get_pc () - 1;

    const auto virtual_address = breakpoint - m_virtual_offset;

    /*
     * Find the breakpoint that we have hit
     */
    const auto it = m_measure_points.find (virtual_address);

    if (it != m_measure_points.end () &&
        it->second.breakpoint.is_enabled ())
    {
      /*
       * Toggle all measure points.
       */
      for (const auto block_ptr : it->second.measured_blocks)
      {
        if (block_ptr->code_block.start == virtual_address)
        {
          block_ptr->counter.start ();
        }
        else if (block_ptr->code_block.end == virtual_address)
        {
          block_ptr->counter.stop ();
        }
      }

      /*
       * Step over the breakpoint
       */
      set_pc (breakpoint);
      it->second.breakpoint.disable ();
      ptrace (PTRACE_SINGLESTEP, m_pid, nullptr, nullptr);
      wait_for_signal ();
      it->second.breakpoint.enable ();
    }
  }

  void RemoteProfiler::run ()
  {
    while (continue_execution ()) {}

    /*
     * TODO - return information
     */
    for (const auto &block : m_measured_blocks)
    {
      std::cerr << std::hex;

      if (!block.code_block.name.empty ())
      {
        std::cerr << block.code_block.name;
      }
      else
      {
        std::cerr << "0x" << block.code_block.start
                  << "--"
                  << "0x" << block.code_block.end;
      }

      std::cerr << " "
                << block.counter.event ().str ()
                << ": "
                << std::dec
                << block.counter.get () << std::endl;
    }
  }
}
