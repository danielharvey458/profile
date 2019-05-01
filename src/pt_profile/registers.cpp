#include "pt_profile/registers.h"

#include <algorithm>

namespace pt_profile
{
  namespace
  {
    const Registers::DescriptorTable register_descriptors
    {{
      {Registers::R15, 15, "r15"},
      {Registers::R14, 14, "r14"},
      {Registers::R13, 13, "r13"},
      {Registers::R12, 12, "r12"},
      {Registers::RBP,  6, "rbp"},
      {Registers::RBX,  3, "rbx"},
      {Registers::R11, 11, "r11"},
      {Registers::R10, 10, "r10"},
      {Registers::R9,   9,  "r9"},
      {Registers::R8,   8,  "r8"},
      {Registers::RAX,  0, "rax"},
      {Registers::RCX,  2, "rcx"},
      {Registers::RDX,  1, "rdx"},
      {Registers::RSI,  4, "rsi"},
      {Registers::RDI,  5, "rdi"},
      {Registers::ORIG_RAX,  -1, "orig_rax"},
      {Registers::RIP,  -1, "rip"},
      {Registers::CS,  51, "cs"},
      {Registers::RFLAGS,  49, "eflags"},
      {Registers::RSP,  7, "rsp"},
      {Registers::SS,  52, "ss"},
      {Registers::FS_BASE,  58, "fs_base"},
      {Registers::GS_BASE,  59, "gs_base"},
      {Registers::DS,  53, "ds"},
      {Registers::ES,  50, "es"},
      {Registers::FS,  54, "fs"},
      {Registers::GS,  55, "gs"},
    }}; 
  }

  Registers::const_iterator Registers::begin ()
  {
    return register_descriptors.begin ();
  }

  Registers::const_iterator Registers::end ()
  {
    return register_descriptors.end ();
  }

  Registers::const_iterator Registers::from_name (const std::string &name)
  {
    const auto pred = [&name] (const auto &d) {return d.name == name;};
    return std::find_if (begin (), end (), pred);
  }

  Registers::const_iterator Registers::from_dwarf_register (const int dwarf_register)
  {
    const auto pred = [dwarf_register] (const auto &d) {
                        return d.dwarf_register == dwarf_register;};
    return std::find_if (begin (), end (), pred);
  }

  Registers::const_iterator Registers::from_register (const Register reg)
  {
    const auto pred = [reg] (const auto &d) {return d.reg == reg;};
    return std::find_if (begin (), end (), pred);
  }

  uint64_t *
  Registers::lookup_user_regs (user_regs_struct &regs,
                               const_iterator it)
  {
    return reinterpret_cast<uint64_t*> (&regs) + std::distance (begin (), it);
  }

  const uint64_t *
  Registers::lookup_user_regs (const user_regs_struct &regs,
                               const_iterator it)
  {
    return reinterpret_cast<const uint64_t*> (&regs) + std::distance (begin (), it);
  }
}
