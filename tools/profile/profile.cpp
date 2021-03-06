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

#include "profile/elf_parser.h"
#include "profile/performance_counter.h"
#include "profile/remote_profiler.h"

#include "elf++.hh"
#include "dwarf++.hh"

#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stddef.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

namespace profile
{
  struct Args
  {
    std::vector<std::pair<Event, std::string>> profile_config;
    size_t executable_index = 0;
  };

  Args parse_args (const std::vector<std::string> &cmd_args)
  {
    Args args;

    for (auto idx = 0u; idx < cmd_args.size (); ++idx)
    {
      if (cmd_args[idx] == "-e")
      {
        const auto spec = cmd_args.at (idx + 1);
        const auto it = spec.find (':');
        const auto event = std::string (spec.begin (), spec.begin () + it);
        const auto function = std::string (spec.begin () + it + 1, spec.end ());

        args.profile_config.emplace_back (
          Event::from_string (event), function);
      }
      else if (cmd_args[idx] == "--")
      {
        /*
         * Note don't add 1 here as 'args' has already
         * removed the program name
         */
        args.executable_index = idx + 2;
        return args;
      }
    }

    return args;
  }
}

int main (int argc, char **argv)
{
  using namespace profile;

  if (argc < 3)
  {
    std::cerr << "Usage: "
              << argv[0]
              << " [-e event:function_spec] -- PROGRAM ARGS..."
              << std::endl;

    return EXIT_FAILURE;
  }

  const auto args = parse_args ({argv + 1, argv + argc});
  const auto executable = argv[args.executable_index];
  const auto pid = fork ();

  if (pid == 0)
  {
    ptrace (PTRACE_TRACEME, 0, nullptr, nullptr);
    execv (executable, argv + args.executable_index + 1);
  }
  else if (pid >= 1)
  {
    auto remote = RemoteProfiler (pid);
    const auto fd = open (executable, O_RDONLY);
    const auto e = elf::elf (elf::create_mmap_loader (fd));
    const auto dw = dwarf::dwarf (dwarf::elf::create_loader (e));

    for (const auto &[event, function] : args.profile_config)
    {
      const auto opt = get_function_range (dw, function);

      if (!opt)
      {
        fprintf (stderr,
                 "Couldn't find symbol for '%s'\n",
                 function.c_str ());

        return EXIT_FAILURE;
      }

      auto code_block
        = RemoteProfiler::CodeBlock {function, opt->first, opt->second};

      remote.set_measure (event, std::move (code_block));
    }
    remote.run ();
  }
}
