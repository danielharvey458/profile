#include "pt_profile/breakpoint.h"
#include "pt_profile/debugger.h"

#include <iostream>
#include <stddef.h>
#include <sys/ptrace.h>
#include <unistd.h>

int main (int argc, char **argv)
{
  using namespace pt_profile;

  if (argc < 1)
  {
    std::cerr << "Program not specified" << std::endl;
    return EXIT_FAILURE;
  }

  const auto prog = argv[1]; 
  const auto pid = fork ();

  if (pid == 0)
  {
    ptrace (PTRACE_TRACEME, 0, nullptr, nullptr);
    execl (prog, prog, nullptr);
  }
  else if (pid >= 1)
  {
    auto dbg = Debugger (prog, pid);
    dbg.run ();
  }
}
