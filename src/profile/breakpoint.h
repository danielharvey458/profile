#ifndef PROFILE_DETAIL_BREAKPOINT_H
#define PROFILE_DETAIL_BREAKPOINT_H

#include <cstdint>
#include <sys/types.h>

namespace profile::detail
{
  /**
   * Breakpoint sets and removes breakpoints in remote
   * processes
   */
  struct Breakpoint
  {
    public:

      /**
       * Construct a breakpoint at @p address in process @p pid.
       *
       * Note that no runtime virtual offsets are added to the
       * @p address.
       *
       * Note that the breakpoint is not enabled on construction.
       */
      explicit Breakpoint (pid_t pid, std::intptr_t address);

      /**
       * Enable the breakpoint so that it will fire when the
       * remote process reaches the specified address
       */
      void enable ();

      /**
       * Disable the breakpoint.
       */
      void disable ();

      /**
       * Check whether the breakpoint is enabled.
       */
      bool is_enabled () const;

      /**
       * Get the address at which this breakpoint is set.
       */
      std::intptr_t address () const;

    private:
      pid_t m_pid = {};
      std::intptr_t m_address = {};
      bool m_enabled = {};
      uint8_t m_saved_data = {};
  };
}

#include "profile/breakpoint.inl"

#endif
