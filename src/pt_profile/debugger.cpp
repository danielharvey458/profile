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
      /*
       * TODO Move this into a class of its own
       */
      const auto it = Registers::from_register (reg);

      return *(reinterpret_cast<uint64_t*> (&regs)
                 + std::distance (Registers::begin (), it));
    }

    void set_register (pid_t pid, Registers::Register reg, uint64_t value)
    {
      user_regs_struct regs;
      ptrace (PTRACE_GETREGS, pid, nullptr, &regs);

      const auto it = Registers::from_register (reg);

      const auto offset = std::distance (Registers::begin (), it);

      *(reinterpret_cast<uint64_t*> (&regs) + offset) = value;

      ptrace (PTRACE_SETREGS, pid, nullptr, &regs);
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
    auto tokenizer = Tokenizer (command, ' ');

    const auto leader = tokenizer.next ();

    if (leader == "continue")
    {
      continue_execution ();
    }
    else if (leader == "break")
    {
      const auto address
       = std::strtol (tokenizer.next ().c_str (),
                      nullptr,
                      16);

      set_breakpoint (address);
    }
    else if (leader == "register")
    {
      const auto op = tokenizer.next ();

      const auto print_register = [&] (const Registers::Descriptor &desc)
      {
        std::cout << desc.name << " 0x"
                  << std::hex << get_register (m_pid, desc.reg)
                  << '\n';
      };

      if (op == "dump")
      {
        std::for_each (Registers::begin (), Registers::end (), print_register);
      }
      else if (op == "read")
      {
        const auto name = tokenizer.next ();

        const auto register_it = Registers::from_name (name);

        if (register_it == Registers::end ())
        {
          std::cerr << "No such register '" << name << "'" << std::endl;
          return;
        }
        else
        {
          print_register (*register_it);
        }
      }
      else if (op == "write")
      {
        const auto name = tokenizer.next ();
        const auto value = std::stoull (tokenizer.next (), nullptr, 16);
        const auto reg = Registers::from_name (name);
        set_register (m_pid, reg->reg, value);
      }
      else
      {
        std::cerr << "Unrecognised operation '" << op << "'\n";
      }
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
