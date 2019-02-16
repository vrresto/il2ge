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

#include "jni_wrapper.h"
#include "meta_class_registrators.h"
#include <misc.h>

#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <cassert>
#include <jni.h>

#include <windows.h>

using namespace std;
using namespace jni_wrapper;


namespace
{


bool g_initialized  = false;
map<string, MetaMethod&> g_exports;
vector<MetaClass> g_meta_classes;


void resolveImports(HMODULE module)
{
  for (auto &meta_class : g_meta_classes)
  {
    for (auto &method : meta_class.methods)
    {
      if (*method.import_addr)
        continue;

      for (auto &alias : method.name_aliases)
      {
        void *proc = (void*) GetProcAddress(module, alias.c_str());
        if (proc)
        {
          *method.import_addr = proc;
          break;
        }
      }
    }
  }
}


void addExport(MetaMethod &method)
{
  for (auto &alias : method.name_aliases)
    g_exports.insert_or_assign(alias, method);
}


} // namespace


namespace jni_wrapper
{


void init()
{
  if (g_initialized)
    return;


  vector<MetaClassInitFunc*> registrators
  {
    registrator::rts::Time,
    registrator::il2::engine::Renders,
    registrator::il2::engine::Render,
    registrator::il2::engine::Landscape,
    registrator::il2::engine::Camera,
    registrator::il2::engine::GObj,
    registrator::il2::fm::Wind,
  };

  if (il2ge::core_wrapper::getConfig().enable_light_point)
    registrators.push_back(registrator::il2::engine::LightPoint);

  for (auto registrator : registrators)
  {
    g_meta_classes.resize(g_meta_classes.size() + 1);

    MetaClass &meta_class = g_meta_classes.back();
    registrator(meta_class);

    cout<<"registerMetaClass: "<<meta_class.package<<'.'<<meta_class.name<<endl;

    string package = meta_class.package;
    for (char &c : package)
    {
      if (c == '.')
        c = '_';
    }

    for (auto &method : meta_class.methods)
    {
      cout<<"\tmethod: "<<method.name<<endl;

      string full_name = "Java_" + package + '_' + meta_class.name + '_' + method.name;
      string full_name_with_params;

      const size_t size_args = method.size_args + 2 * sizeof(void*);

      char buf[1024];
      int written = snprintf(buf, sizeof(buf), "%s@%d", full_name.c_str(), size_args);
      assert(written >= 0);
      assert((size_t)written < sizeof(buf));

      full_name_with_params = buf;

      method.name_aliases.push_back(full_name);
      method.name_aliases.push_back(full_name_with_params);
      method.name_aliases.push_back('_' + full_name);
      method.name_aliases.push_back('_' + full_name_with_params);

      addExport(method);
    }
  }

  g_initialized = true;
}


void *getExport(const string &full_name)
{
  assert(g_initialized);

  cout<<"getExport: "<<full_name<<endl;

  try
  {
    auto &method = g_exports.at(full_name);
    cout<<"found export: "<<full_name<<endl;
    assert(*method.import_addr);
    return method.export_addr;
  }
  catch(...)
  {
    return nullptr;
  }
}


void resolveImports(void *module)
{
  assert(g_initialized);

  ::resolveImports((HMODULE)module);
}


} // namespace jni_wrapper
