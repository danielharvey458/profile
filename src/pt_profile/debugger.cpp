#include "pt_profile/debugger.h"
#include "pt_profile/tokenizer.h"
#include "pt_profile/registers.h"

#include "linenoise.h"

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

  void Debugger::set_measure (std::intptr_t begin_address,
                              std::intptr_t end_address)
  {
    const auto perf_counter_it
      = m_counters.emplace (
          m_counters.end (),
          AddressedCounter
          {
            begin_address,
            end_address,
            PerformanceCounter
            {
              PERF_TYPE_HARDWARE,
              PERF_COUNT_HW_INSTRUCTIONS,
              m_pid
            }
          });

    m_breakpoints.emplace (
        begin_address,
        Breakpoint (m_pid, begin_address + m_virtual_offset))
          .first->second.enable ();

    m_measure_points.emplace (
          begin_address,
          CounterHandle {&perf_counter_it->counter, CounterHandle::START});

    m_breakpoints.emplace (
        end_address,
        Breakpoint (m_pid, end_address + m_virtual_offset))
          .first->second.enable ();

    m_measure_points.emplace (
          end_address,
          CounterHandle {&perf_counter_it->counter, CounterHandle::STOP});
  }

  void Debugger::write_memory (uint64_t address, uint64_t value)
  {
    ptrace (PTRACE_POKEDATA, m_pid, address, &value);
  }

  uint64_t Debugger::read_memory (uint64_t address) const
  {
    return ptrace (PTRACE_PEEKDATA, m_pid, address, nullptr);
  }

  uint64_t Debugger::get_pc () const
  {
    return get_register (m_pid, Registers::RIP);
  }

  void Debugger::set_pc (const int64_t pc)
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
    const auto breakpoint_it = m_breakpoints.find (virtual_address);

    if (breakpoint_it != m_breakpoints.end () &&
        breakpoint_it->second.is_enabled ())
    {
      /*
       * Toggle all measure points.
       */
      const auto [begin, end] = m_measure_points.equal_range (virtual_address);
      for (auto it = begin; it != end; ++it)
      {
        if (it->second.start_stop == CounterHandle::START)
        {
          it->second.counter->start ();
        }
        else
        {
          it->second.counter->stop ();
        }
      }

      /*
       * Step over the breakpoint
       */
      set_pc (breakpoint);
      breakpoint_it->second.disable ();
      ptrace (PTRACE_SINGLESTEP, m_pid, nullptr, nullptr);
      wait_for_signal ();
      breakpoint_it->second.enable ();
    }
  }

  void Debugger::run ()
  {
    while (continue_execution ()) {}

    for (const auto &counter : m_counters)
    {
      std::cerr << std::hex
                << "0x" << counter.start
                << "--"
                << "0x" << counter.end
                << ": "
                << std::dec
                << counter.counter.get () << std::endl;
    }
  }
}
