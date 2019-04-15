#include <stddef.h>
#include "linenoise.h"

#include <iostream>
#include <unistd.h>

#include <sys/ptrace.h>
#include <sys/wait.h>

#include <errno.h>

#include <algorithm>
#include <unordered_map>
#include <fstream>

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

struct breakpoint
{
  public:

    breakpoint (pid_t pid, std::intptr_t address)
      : m_pid {pid},
        m_address {address},
        m_enabled {false},
        m_saved_data {}
    { 
    }

    void enable ()
    {
      auto data = ptrace (PTRACE_PEEKDATA, m_pid, m_address, nullptr);

      m_saved_data = static_cast<uint8_t> (data & 0xff);

      ptrace (PTRACE_POKEDATA,
              m_pid,
              m_address,
              ((data & ~0xff) | 0xcc));

      m_enabled = true;
    }

    void disable ()
    {
      auto data = ptrace (PTRACE_PEEKDATA, m_pid, m_address, nullptr);

      ptrace (PTRACE_POKEDATA,
              m_pid,
              m_address,
              ((data & ~0xff) | m_saved_data));

      m_enabled = false;
    }

    bool is_enabled () const
    {
      return m_enabled;
    }

    std::intptr_t address () const
    {
      return m_address;
    }

  private:
    pid_t m_pid;
    std::intptr_t m_address;
    bool m_enabled;
    uint8_t m_saved_data;
};

struct debugger
{
  public:

    debugger (std::string program_name, pid_t pid)
      : m_program_name (std::move (program_name)),
        m_pid (pid),
        m_virtual_offset (get_virtual_offset (m_pid))
    {
      std::cout << "Tracing " << m_pid << std::hex
                << " from 0x" << m_virtual_offset << std::endl;
    }

    void continue_execution ()
    {
      ptrace (PTRACE_CONT, m_pid, nullptr, nullptr);
      int wait_status;
      auto options = 0;
      waitpid (m_pid, &wait_status, options);
    }

    void set_breakpoint (std::intptr_t address)
    {
      const auto [it, set]
        = m_breakpoints.emplace (address, breakpoint (m_pid, m_virtual_offset + address));

      if (!set)
      {
        std::cerr << "Breakpoint already set at 0x" << std::hex
                  << address << "\n";
      }
      else
      {
        it->second.enable ();
      }
    }

    void handle_command (const std::string &command)
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

    void run ()
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

  private:
    std::string m_program_name;
    pid_t m_pid;
    std::intptr_t m_virtual_offset;
    std::unordered_map<std::intptr_t, breakpoint> m_breakpoints;
};

int main (int argc, char **argv)
{
  if (argc < 1)
  {
    std::cerr << "Program not specified" << std::endl;
    return EXIT_FAILURE;
  }

  const auto prog = argv[1]; 
  const auto pid = fork ();

  if (pid == 0)
  {
    ptrace (PTRACE_TRACEME, 0, nullptr, nullptr);
    execl (prog, prog, nullptr);
  }
  else if (pid >= 1)
  {
    auto dbg = debugger (prog, pid);
    dbg.run ();
  }
}


