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
    m_counters.push_back (PerformanceCounter (PERF_TYPE_HARDWARE, PERF_COUNT_HW_REF_CPU_CYCLES, m_pid));

    m_measure_points.emplace (
          begin_address,
          MeasurePoint {Breakpoint (m_pid, m_virtual_offset + begin_address), MeasurePoint::BEGIN, &m_counters.back ()})->second.breakpoint.enable ();

    m_measure_points.emplace (
          end_address,
          MeasurePoint {Breakpoint (m_pid, m_virtual_offset + end_address), MeasurePoint::END, &m_counters.back ()})->second.breakpoint.enable ();
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

    const auto [begin, end] = m_measure_points.equal_range (virtual_address);

    for (auto it = begin; it != end; ++it)
    {
      if (it->second.breakpoint.is_enabled ())
      {
        if (it->second.begin_end == MeasurePoint::BEGIN)
        {
          it->second.counter->reset ();
          it->second.counter->enable ();
        }
        else
        {
          it->second.counter->disable ();
        }

        set_pc (breakpoint);
        it->second.breakpoint.disable ();
        ptrace (PTRACE_SINGLESTEP, m_pid, nullptr, nullptr);
        wait_for_signal ();
        it->second.breakpoint.enable ();
      }
    }
  }

  void Debugger::run ()
  {
    while (continue_execution ()) {}

    for (const auto &counter : m_counters)
    {
      std::cout << "Instructions " << counter.get () << std::endl;
    }
  }
}
