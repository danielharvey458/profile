#ifndef PT_PROFILE_BREAKPOINT_H
#define PT_PROFILE_BREAKPOINT_H

#include <cstdint>
#include <sys/types.h>

namespace pt_profile
{
  struct Breakpoint
  {
    public:

      Breakpoint () = default;

      explicit Breakpoint (pid_t pid, std::intptr_t address);

      void enable ();

      void disable ();

      bool is_enabled () const { return m_enabled; }

      std::intptr_t address () const { return m_address; }

    private:
      pid_t m_pid = {};
      std::intptr_t m_address = {};
      bool m_enabled = {};
      uint8_t m_saved_data = {};
  };
}

#endif
