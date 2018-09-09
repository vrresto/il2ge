/**
 *    IL-2 Graphics Extender
 *    Copyright (C) 2018 Jan Lepper
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef LOADER_INTERFACE_H
#define LOADER_INTERFACE_H

#include <string>
#include <windef.h>


[[ noreturn ]] void fatalError(const std::string &message);

struct LoaderInterface
{
  void (*patchIAT)(const char *function_name,
      const char *import_module,
      void *new_function,
      void **orig_func_out,
      HMODULE module) = 0;
  std::string (*getCoreWrapperFilePath)() = 0;
};

#endif
