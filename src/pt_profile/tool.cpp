#include "pt_profile/breakpoint.h"
#include "pt_profile/debugger.h"

#include <iostream>
#include <stddef.h>
#include <sys/ptrace.h>
#include <unistd.h>

#include <vector>
#include <fstream>
#include <sstream>

std::vector<std::pair<std::intptr_t, std::intptr_t>>
config_from_file (std::istream &&input)
{
  std::vector<std::pair<std::intptr_t, std::intptr_t>> result;
  std::string temp;

  while (std::getline (input, temp))
  {
    std::intptr_t start, stop;
    std::stringstream string_stream (temp);

    string_stream >> std::hex >> start >> stop;
    result.emplace_back (start, stop);
  }

  return result;
}

int main (int argc, char **argv)
{
  using namespace pt_profile;

  if (argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << " CONFIG_FILE PROGRAM" << std::endl;
    return EXIT_FAILURE;
  }

  const auto prog = argv[2];
  const auto pid = fork ();

  if (pid == 0)
  {
    ptrace (PTRACE_TRACEME, 0, nullptr, nullptr);
    execl (prog, prog, nullptr);
  }
  else if (pid >= 1)
  {
    auto dbg = Debugger (prog, pid);

    for (const auto &[start, stop] : config_from_file (std::ifstream (argv[1])))
    {
      dbg.set_measure (start, stop);
    }
    dbg.run ();
  }
}
