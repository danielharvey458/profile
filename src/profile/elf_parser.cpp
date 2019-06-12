#include "profile/elf_parser.h"

namespace profile
{
  std::optional<std::pair<std::intptr_t, std::intptr_t>>
  get_function_range (const dwarf::dwarf &dwarf_info,
                      const std::string &function_name)
  {
    for (auto &cu : dwarf_info.compilation_units ())
    {
      for (const auto &die : cu.root ())
      {
        if (die.has (dwarf::DW_AT::name) &&
            at_name (die) == function_name)
        {
          return std::make_pair (at_low_pc (die), at_high_pc (die) - 1);
        }
      }
    }
    return std::nullopt;
  }
}


