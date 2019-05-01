#ifndef PT_PROFILE_DEBUGGER_H
#define PT_PROFILE_DEBUGGER_H

#include "pt_profile/breakpoint.h"

#include <sys/types.h>
#include <cstdint>
#include <unordered_map>
#include <string>

namespace pt_profile
{
  class Debugger
  {
    public:

      explicit Debugger (std::string program_name, pid_t pid);

      void run ();

    private:

      void continue_execution ();
      void set_breakpoint (std::intptr_t address);
      void handle_command (const std::string &command);
      void write_memory (uint64_t address, uint64_t value);
      uint64_t read_memory (uint64_t address) const;
      uint64_t get_pc () const;
      void set_pc (int64_t pc);
      void step_over_breakpoint ();
      void wait_for_signal ();


      std::string m_program_name;
      pid_t m_pid;
      std::intptr_t m_virtual_offset;
      std::unordered_map<std::intptr_t, Breakpoint> m_breakpoints;
  };
}

#endif
