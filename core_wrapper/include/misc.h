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

#ifndef IL2_CORE_H
#define IL2_CORE_H

#include <stddef.h>
#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <memory>

struct JNIEnv_;

namespace jni_wrapper
{
  void init();
  void resolveImports(void *module);
  void *getExport(const std::string &full_name);
}


namespace il2ge::core_wrapper
{
  struct Config
  {
    bool enable_dump = false;
    bool enable_effects = false;
    bool enable_light_point = false;
    bool enable_base_map = false;
  };

  const Config &getConfig();
  JNIEnv_ *getJNIEnv();
  std::string getWrapperLibraryFilePath();
  [[ noreturn ]] void fatalError(const std::string &message);
}


#endif
