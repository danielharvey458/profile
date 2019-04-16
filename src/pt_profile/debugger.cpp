#include "pt_profile/debugger.h"

#include "linenoise.h"

#include <sys/ptrace.h>
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
  }

  Debugger::Debugger (std::string program_name, pid_t pid)
    : m_program_name (std::move (program_name)),
      m_pid (pid),
      m_virtual_offset (get_virtual_offset (m_pid))
  {
  }

  void Debugger::continue_execution ()
  {
    ptrace (PTRACE_CONT, m_pid, nullptr, nullptr);
    int wait_status;
    auto options = 0;
    waitpid (m_pid, &wait_status, options);

    for (auto &kv : m_breakpoints)
    {
      kv.second.disable ();
    }
  }

  void Debugger::set_breakpoint (std::intptr_t address)
  {
    const auto [it, set]
      = m_breakpoints.emplace (address, Breakpoint (m_pid, m_virtual_offset + address));

    if (!set)
    {
      /*
       * FIXME no cout/cerr in library.
       */
      std::cerr << "Breakpoint already set at 0x" << std::hex
                << address << "\n";
    }
    else
    {
      it->second.enable ();
    }
  }

  void Debugger::handle_command (const std::string &command)
  {
    const auto it = std::find (command.begin (), command.end (), ' ');
    const auto leader = std::string (command.begin (), it);
    
    if (leader == "continue")
    {
      continue_execution ();
    }
    else if (leader == "break")
    {
      const auto address
       = std::strtol (std::string (it, command.end ()).c_str (),
                      nullptr,
                      16);

      set_breakpoint (address);
    }
    else
    {
      std::cerr << "Unrecognised command '" << command << "'\n";
    }     
  }

  void Debugger::run ()
  {
    int wait_status = 0;
    auto options = 0;

    waitpid (m_pid, &wait_status, options);

    char *line = nullptr;

    while ((line = linenoise ("mindbg> ")) != nullptr)
    {
      handle_command (line);
      linenoiseHistoryAdd (line);
      linenoiseFree (line);
    }
  }
}
