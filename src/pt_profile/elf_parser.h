#ifndef PT_PROFILE_ELF_PARSER_H
#define PT_PROFILE_ELF_PARSER_H

#include "elf++.hh"
#include "dwarf++.hh"

namespace pt_profile
{
  std::pair<std::intptr_t, std::intptr_t>
  get_function_range (const dwarf::dwarf &dwarf_info,
                      const std::string &function_name);
}

#endif
