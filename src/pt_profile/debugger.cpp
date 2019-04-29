#include "pt_profile/debugger.h"
#include "pt_profile/tokenizer.h"

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

    enum class Register
    {
      R15, R14, R13, R12, RBP, RBX,
      R11, R10,  R9,  R8, RAX, RCX,
      RDX, RSI, RDI, ORIG_RAX, RIP,
      CS,  RFLAGS,   RSP,  SS, FS_BASE,
      GS_BASE, DS, ES, FS, GS
    };

    constexpr std::size_t registers_count = 27;

    struct RegisterDescriptor
    {
      Register reg;
      int dwarf_register;
      std::string name;
    };

    const std::array<RegisterDescriptor, registers_count> register_descriptors
    {{
       {Register::R15, 15, "r15"},
       {Register::R14, 14, "r14"},
       {Register::R13, 13, "r13"},
       {Register::R12, 12, "r12"},
       {Register::RBP,  6, "rbp"},
       {Register::RBX,  3, "rbx"},
       {Register::R11, 11, "r11"},
       {Register::R10, 10, "r10"},
       {Register::R9,   9,  "r9"},
       {Register::R8,   8,  "r8"},
       {Register::RAX,  0, "rax"},
       {Register::RCX,  2, "rcx"},
       {Register::RDX,  1, "rdx"},
       {Register::RSI,  4, "rsi"},
       {Register::RDI,  5, "rdi"},
       {Register::ORIG_RAX,  -1, "orig_rax"},
       {Register::RIP,  -1, "rip"},
       {Register::CS,  51, "cs"},
       {Register::RFLAGS,  49, "eflags"},
       {Register::RSP,  7, "rsp"},
       {Register::SS,  52, "ss"},
       {Register::FS_BASE,  58, "fs_base"},
       {Register::GS_BASE,  59, "gs_base"},
       {Register::DS,  53, "ds"},
       {Register::ES,  50, "es"},
       {Register::FS,  54, "fs"},
       {Register::GS,  55, "gs"},
    }};

    uint64_t get_register (pid_t pid, Register reg)
    {
      user_regs_struct regs;
      ptrace (PTRACE_GETREGS, pid, nullptr, &regs);
      /*
       * TODO Move this into a class of its own
       */
      const auto it = std::find_if (register_descriptors.begin (),
                                    register_descriptors.end (),
                                    [&] (auto &&descriptor)
                                    {
                                      return descriptor.reg == reg;
                                    });

      return *(reinterpret_cast<uint64_t*> (&regs)
                 + std::distance (register_descriptors.begin (), it));
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

      if (op == "read")
      {
        const auto name = tokenizer.next ();

        const auto register_it
          = std::find_if (register_descriptors.begin (),
                          register_descriptors.end (),
                          [&] (auto &&descriptor)
                          {
                            return descriptor.name == name;
                          });

        if (register_it == register_descriptors.end ())
        {
          std::cerr << "No such register '" << name << "'" << std::endl;
          return;
        }
        else
        {
          std::cout << register_it->name << " " << std::hex << get_register (m_pid, register_it->reg) << std::endl;
        }
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
