/*
 * Copyright 2019 Daniel Harvey
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
