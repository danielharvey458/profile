/*
 * Copyright 2019 Daniel Harvey
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PROFILE_ELF_PARSER_H
#define PROFILE_ELF_PARSER_H

#include "elf++.hh"
#include "dwarf++.hh"

#include <optional>

namespace profile
{
  /*
   * Try to find the address range of the function called
   * @p function_name in the provided @p dwarf_info,
   * returning an empty optional if no such function
   * can be found.
   *
   * This function is highly incomplete: it doesn't
   * handle overloads at all, for example, and will
   * choose the first function that it finds.
   */
  std::optional<std::pair<std::intptr_t, std::intptr_t>>
  get_function_range (const dwarf::dwarf &dwarf_info,
                      const std::string &function_name);
}

#endif
