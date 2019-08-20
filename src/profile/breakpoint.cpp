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

#include "profile/breakpoint.h"

#include <sys/ptrace.h>

namespace profile::detail
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
