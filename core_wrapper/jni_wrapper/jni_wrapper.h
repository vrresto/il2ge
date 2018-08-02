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

#ifndef CORE_JNI_WRAPPER_H
#define CORE_JNI_WRAPPER_H

#include <string>
#include <vector>
#include <jni.h>

namespace jni_wrapper
{


template <typename ... Types>
struct MethodSpec
{
  typedef __stdcall int Signature (JNIEnv*, jobject, Types...);

  enum { N_ARGS = sizeof...(Types) };
};


struct MetaMethod
{
  std::string name;
  unsigned int num_args = 0;
  void **import_addr = nullptr;
  void *export_addr = nullptr;

  MetaMethod(const std::string &name, unsigned int num_args, void **import_addr, void *export_addr) :
      name(name), num_args(num_args), import_addr(import_addr), export_addr(export_addr) {}
};


struct MetaClass
{
  std::string package;
  std::string name;
  std::vector<MetaMethod> methods;

  template <class T>
  void addMethod(const std::string &name,
      typename T::Signature **import_addr,
      typename T::Signature *export_addr)
  {
    methods.push_back(MetaMethod(name, T::N_ARGS, (void**)import_addr, (void*)export_addr));
  }
};


class MetaClassRegistrator
{
  typedef void initializeMetaClass_t(jni_wrapper::MetaClass&);
  initializeMetaClass_t *m_init_func = 0;

public:
  MetaClassRegistrator(initializeMetaClass_t *init_func);
  initializeMetaClass_t *getInitFunc() const { return m_init_func; }
};


} // namespace jni_wrapper


#endif
