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

using namespace std;
using namespace jni_wrapper;


namespace
{


map<string, void*> g_exports;
vector<MetaClassInitFunc*> g_registrator_table =
{
  #include <_generated/jni_wrapper/registrator_table>
};


void registerMetaClass(const MetaClass &meta_class)
{
  cout<<"registerMetaClass: "<<meta_class.package<<'.'<<meta_class.name<<endl;

  string package = meta_class.package;
  for (char &c : package)
  {
    if (c == '.')
      c = '_';
  }

  for (const MetaMethod &method : meta_class.methods)
  {
    cout<<"\tmethod: "<<method.name<<endl;

    string full_name = "_Java_" + package + '_' + meta_class.name + '_' + method.name;

    char buf[1024];
    int written = snprintf(buf, sizeof(buf), "%s@%d", full_name.c_str(), (method.num_args + 2) * 4);
    assert(written >= 0);
    assert((size_t)written < sizeof(buf));

    *method.import_addr = il2ge::core_wrapper::getOrigProcAddress(buf);

    g_exports[buf] = method.export_addr;
  }
}


} // namespace


namespace jni_wrapper
{


void init()
{
  for (auto registrator : g_registrator_table)
  {
    MetaClass meta_class;
    registrator(meta_class);
    registerMetaClass(meta_class);
  }

}

void *getExport(const string &full_name)
{
  cout<<"getExport: "<<full_name<<endl;
  return g_exports[full_name];
}


} // namespace jni_wrapper

