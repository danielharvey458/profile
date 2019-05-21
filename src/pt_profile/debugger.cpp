#include "pt_profile/debugger.h"
#include "pt_profile/tokenizer.h"
#include "pt_profile/registers.h"

#include <sys/ptrace.h>
#include <sys/user.h>
#include <sys/wait.h>

#include <algorithm>
#include <fstream>
#include <iostream>

namespace pt_profile
{
  namespace
  {
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

  Debugger::Debugger (std::string program_name, pid_t pid)
    : m_program_name (std::move (program_name)),
      m_pid (pid),
      m_virtual_offset (get_virtual_offset (m_pid))
  {
  }

  void Debugger::set_measure (const Event &event,
                              std::intptr_t begin_address,
                              std::intptr_t end_address)
  {
    const auto perf_counter_it
      = m_measured_blocks.emplace (
          m_measured_blocks.end (),
          MeasuredBlock
          {
            begin_address,
            end_address,
            PerformanceCounter {event, m_pid}
          });

    insert_measure_point (begin_address, &*perf_counter_it);
    insert_measure_point (end_address, &*perf_counter_it);
  }

  std::intptr_t Debugger::get_pc () const
  {
    return get_register (m_pid, Registers::RIP);
  }

  void Debugger::set_pc (const std::intptr_t pc)
  {
    set_register (m_pid, Registers::RIP, pc);
  }

  bool Debugger::wait_for_signal ()
  {
    auto wait_status = 0;
    auto options = 0;
    waitpid (m_pid, &wait_status, options);
    return !WIFEXITED (wait_status);
  }

  void Debugger::insert_measure_point (const std::intptr_t address,
                                       MeasuredBlock *block)
  {
    auto &measure_point
      = m_measure_points.emplace (
          address,
          MeasurePoint
          {
            Breakpoint
            {
              m_pid, address + m_virtual_offset
            }
          }).first->second;

    measure_point.measured_blocks.push_back (block);
    measure_point.breakpoint.enable ();
  }

  bool Debugger::continue_execution ()
  {
    step_over_breakpoint ();
    ptrace (PTRACE_CONT, m_pid, nullptr, nullptr);
    return wait_for_signal ();
  }

  void Debugger::step_over_breakpoint ()
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
        if (block_ptr->start == virtual_address)
        {
          block_ptr->counter.start ();
        }
        else if (block_ptr->end == virtual_address)
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

  void Debugger::run ()
  {
    while (continue_execution ()) {}

    for (const auto &block : m_measured_blocks)
    {
      std::cerr << std::hex
                << "0x" << block.start
                << "--"
                << "0x" << block.end
                << " "
                << event_to_string (block.counter.event ())
                << ": "
                << std::dec
                << block.counter.get () << std::endl;
    }
  }
}
