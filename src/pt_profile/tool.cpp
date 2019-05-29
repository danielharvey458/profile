#include "pt_profile/breakpoint.h"
#include "pt_profile/debugger.h"
#include "pt_profile/elf_parser.h"
#include "pt_profile/perf_event.h"

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

namespace pt_profile
{
  std::vector<std::tuple<Event, std::string>>
  config_from_file (std::istream &&input)
  {
    std::vector<std::tuple<Event, std::string>> result;
    std::string temp;

    while (std::getline (input, temp))
    {
      std::string event;
      std::string function;
      std::stringstream string_stream (temp);

      string_stream >> event >> function;
      result.emplace_back (event_from_string (event), function);
    }

    return result;
  }
}

int main (int argc, char **argv)
{
  using namespace pt_profile;

  if (argc < 3)
  {
    std::cerr << "Usage: "
              << argv[0]
              << " CONFIG_FILE PROGRAM ARGS..."
              << std::endl;

    return EXIT_FAILURE;
  }

  const auto prog = argv[2];
  const auto pid = fork ();

  if (pid == 0)
  {
    ptrace (PTRACE_TRACEME, 0, nullptr, nullptr);
    execv (prog, argv + 2);
  }
  else if (pid >= 1)
  {
    auto dbg = Debugger (prog, pid);
    const auto fd = open (argv[2], O_RDONLY);
    const auto e = elf::elf (elf::create_mmap_loader (fd));
    const auto dw = dwarf::dwarf (dwarf::elf::create_loader (e));

    for (const auto &[event, function]
         : config_from_file (std::ifstream (argv[1])))
    {
      const auto [lo, hi] = get_function_range (dw, function);
      dbg.set_measure (event, lo, hi);
    }
    dbg.run ();
  }
}
