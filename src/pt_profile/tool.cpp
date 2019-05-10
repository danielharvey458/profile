#include "pt_profile/breakpoint.h"
#include "pt_profile/debugger.h"

#include <iostream>
#include <stddef.h>
#include <sys/ptrace.h>
#include <unistd.h>

int main (int argc, char **argv)
{
  using namespace pt_profile;

  if (argc != 4)
  {
    std::cerr << "Usage: " << argv[0] << " START END PROGRAM" << std::endl;
    return EXIT_FAILURE;
  }

  const auto prog = argv[3];
  const auto pid = fork ();

  if (pid == 0)
  {
    ptrace (PTRACE_TRACEME, 0, nullptr, nullptr);
    execl (prog, prog, nullptr);
  }
  else if (pid >= 1)
  {
    auto dbg = Debugger (prog, pid);
    dbg.set_measure (std::strtol (argv[1], nullptr, 16),
                     std::strtol (argv[2], nullptr, 16));
    dbg.run ();
  }
}
