#include "pt_profile/breakpoint.h"
#include "pt_profile/perf_event.h"
#include "pt_profile/debugger.h"

#include <iostream>
#include <stddef.h>
#include <sys/ptrace.h>
#include <unistd.h>

#include <vector>
#include <fstream>
#include <sstream>

namespace pt_profile
{
  std::vector<std::tuple<Event, std::intptr_t, std::intptr_t>>
  config_from_file (std::istream &&input)
  {
    std::vector<std::tuple<Event, std::intptr_t, std::intptr_t>> result;
    std::string temp;

    while (std::getline (input, temp))
    {
      std::string event;
      std::intptr_t start, stop;
      std::stringstream string_stream (temp);

      string_stream >> event >> std::hex >> start >> stop;
      result.emplace_back (event_from_string (event), start, stop);
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

    for (const auto &[event, start, stop]
         : config_from_file (std::ifstream (argv[1])))
    {
      dbg.set_measure (event, start, stop);
    }
    dbg.run ();
  }
}
