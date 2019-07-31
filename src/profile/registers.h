#ifndef PROFILE_REGISTERS_H
#define PROFILE_REGISTERS_H

#include <sys/user.h>

#include <array>

namespace profile
{
  /**
   * Registers represents information about CPU registers
   */
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
       * Descriptor contains information about
       * the value of individual registers
       */
      struct Descriptor
      {
        Register reg = {};
        int dwarf_register = {};
        std::string name = {};
      };

      /**
       * Total number of registers available via this API
       */
      static const constexpr std::size_t registers_count = 27;

      using storage = std::array<Descriptor, registers_count>;
      using value_type = storage::value_type;
      using const_iterator = storage::const_iterator;

      /**
       * Iterator pointing at the beginning of the set of registers
       */
      static const_iterator begin ();

      /**
       * Iterator pointing at one-past-the-end of the set of registers
       */
      static const_iterator end ();

      /**
       * Iterator to a register from its name,
       * returning end () if no such register exists
       */
      static const_iterator from_name (const std::string &name);

      /**
       * Iterator to a register from its dwarf representation,
       * returning end () if no such register exists
       */
      static const_iterator from_dwarf_register (int dwarf_register);

      /**
       * Iterator to a register from its enumerator,
       * returning end () if no such register exists
       */
      static const_iterator from_register (Register reg);

      /**
       * Given a user_regs_struct, get the value of the register
       * corresponding to the Descriptor pointed at by @p it
       */
      static uint64_t *
      lookup_user_regs (user_regs_struct &regs, const_iterator it);

      /**
       * Const overload of lookup_user_regs
       */
      static const uint64_t *
      lookup_user_regs (const user_regs_struct &regs, const_iterator it);
  };
}

#endif
