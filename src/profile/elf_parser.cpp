#include "profile/elf_parser.h"

#include <iostream>

namespace profile
{
  namespace
  {
    bool refers_to_function (const dwarf::die &die,
                             const std::string &function_name)
    {
      return (die.has (dwarf::DW_AT::name) &&
              at_name (die) == function_name) ||
             (die.has (dwarf::DW_AT::specification) &&
              refers_to_function (at_specification (die), function_name));
    }

    std::optional<std::pair<std::intptr_t, std::intptr_t>>
    get_function_range_impl (const dwarf::die &die,
                             const std::string &function_name)
    {
      for (const auto &child : die)
      {
        if (refers_to_function (child, function_name) &&
            child.has (dwarf::DW_AT::low_pc) &&
            child.has (dwarf::DW_AT::high_pc))
        {
          return std::make_pair (at_low_pc (child), at_high_pc (child) - 1);
        }
        get_function_range_impl (child, function_name);
      }

      return std::nullopt;
    }
  }

  std::optional<std::pair<std::intptr_t, std::intptr_t>>
  get_function_range (const dwarf::dwarf &dwarf_info,
                      const std::string &function_name)
  {
    for (auto &cu : dwarf_info.compilation_units ())
    {
      if (const auto r = get_function_range_impl (cu.root (), function_name))
      {
        return r;
      }
    }
    return std::nullopt;
  }
}


