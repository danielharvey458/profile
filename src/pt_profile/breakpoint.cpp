#include "pt_profile/breakpoint.h"

#include <sys/ptrace.h>

namespace pt_profile
{
  Breakpoint::Breakpoint (pid_t pid, std::intptr_t address)
    : m_pid {pid},
      m_address {address},
      m_enabled {false},
      m_saved_data {}
  {
  }

  void Breakpoint::enable ()
  {
    if (!m_enabled)
    {
      auto data = ptrace (PTRACE_PEEKDATA, m_pid, m_address, nullptr);

      m_saved_data = static_cast<uint8_t> (data & 0xff);

      ptrace (PTRACE_POKEDATA,
              m_pid,
              m_address,
              ((data & ~0xff) | 0xcc));

      m_enabled = true;
    }
  }

  void Breakpoint::disable ()
  {
    auto data = ptrace (PTRACE_PEEKDATA, m_pid, m_address, nullptr);

    ptrace (PTRACE_POKEDATA,
            m_pid,
            m_address,
            ((data & ~0xff) | m_saved_data));

    m_enabled = false;
  }
}
