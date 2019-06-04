#ifndef PROFILE_REGISTERS_H
#define PROFILE_REGISTERS_H

#include <sys/user.h>

#include <array>

namespace profile
{
  class Registers
  {
    public:

      /**
       * Enumeration of available registers
       */
      enum Register
      {
        R15, R14, R13, R12, RBP, RBX,
        R11, R10,  R9,  R8, RAX, RCX,
        RDX, RSI, RDI, ORIG_RAX, RIP,
        CS, RFLAGS,   RSP,  SS, FS_BASE,
        GS_BASE, DS, ES, FS, GS
      };

      /**
       * Descriptor containing information about
       * registers
       */
      struct Descriptor
      {
        Register reg = {};
        int dwarf_register = {};
        std::string name = {};
      };

      static const constexpr std::size_t registers_count = 27;

      using DescriptorTable = std::array<Descriptor, registers_count>;

      using value_type = DescriptorTable::value_type;

      using const_iterator = DescriptorTable::const_iterator;

      static const_iterator begin ();

      static const_iterator end ();

      static const_iterator from_name (const std::string &name);

      static const_iterator from_dwarf_register (int dwarf_register);

      static const_iterator from_register (Register reg);

      static uint64_t *
      lookup_user_regs (user_regs_struct &regs, const_iterator it);

      static const uint64_t *
      lookup_user_regs (const user_regs_struct &regs, const_iterator it);
  };
}

#endif 
