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


namespace jni_wrapper
{
  void init();
  void *getExport(const std::string &full_name);
}


class Module
{
  std::unordered_map<std::type_index, std::shared_ptr<Module>> sub_modules;

public:
  virtual ~Module() {}

  template <class T>
  T *getSubModule()
  {
    return dynamic_cast<T*>(sub_modules[std::type_index(typeid(T))].get());
  }

  template <class T>
  void setSubModule(T *obj)
  {
    sub_modules[std::type_index(typeid(T))].reset(obj);
  }
};


Module *getGLContext();

void *getOrigProcAddress(const char *name);


#endif