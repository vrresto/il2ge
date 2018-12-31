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


#include "wgl_wrapper.h"
#include <sfs.h>
#include <misc.h>
#include <core.h>
#include <render_util/render_util.h>

#include <gl_wrapper.h>
#include <gl_wrapper/gl_wrapper.h>

#include <iostream>
#include <unordered_map>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <csignal>
#include <memory>

#include <windef.h>
#include <winbase.h>
#include <GL/gl.h>
#include <GL/glext.h>

#include <gl_wrapper/gl_interface.h>
#include <wgl_interface.h>
#include <loader_interface.h>
#include <il2ge/core_wrapper.h>


namespace
{


// typedef int __stdcall isCubeUpdated_T(void*, vo403id*);


bool g_initialized = false;
const LoaderInterface *g_loader = nullptr;
HMODULE g_core_module = 0;
// isCubeUpdated_T *is_cube_updated_func = nullptr;


FARPROC WINAPI wrap_JGL_GetProcAddress(HMODULE module, LPCSTR name)
{
  printf("jgl => GetProcAddress: %s\n", name);

  return (FARPROC) wgl_wrapper::getProcAddress(module, name);
}


HMODULE WINAPI wrap_JGL_LoadLibrary(LPCSTR libFileName)
{
  printf("jgl => LoadLibrary: %s\n", libFileName);

  HMODULE module = LoadLibraryA(libFileName);

  if (module != wgl_wrapper::getGLModule())
  {
    fatalError("DirectX mode is not supported.");
  }

  return module;
}


} // namespace


std::string getCoreWrapperFilePath()
{
  return g_loader->getCoreWrapperFilePath();
}


void *getOrigProcAddress(const char *name)
{
  void *func = reinterpret_cast<void*>(GetProcAddress(g_core_module, name));
  if (func)
    return func;
  else
    return 0;
}


void *il2ge::core_wrapper::getProcAddress(const char *name)
{
  void *addr = nullptr;

  if (g_initialized)
    addr = jni_wrapper::getExport(name);

  if (!addr)
    addr = getOrigProcAddress(name);

  return addr;
}


void il2ge::core_wrapper::init(HMODULE core_module_, const LoaderInterface *loader)
{
  printf("*** il2_core wrapper initialisation ***\n");

  g_core_module = core_module_;
  assert(g_core_module);

  g_loader = loader;
  assert(g_loader);

  sfs::init();
  wgl_wrapper::init();
  core_gl_wrapper::init();
  core::init();
  jni_wrapper::init();

  HMODULE jgl_module = GetModuleHandle("jgl.dll");
  assert(jgl_module);

  g_loader->patchIAT("LoadLibraryA", "kernel32.dll",
      (void*) &wrap_JGL_LoadLibrary, NULL, jgl_module);
  g_loader->patchIAT("GetProcAddress", "kernel32.dll",
      (void*) &wrap_JGL_GetProcAddress, NULL, jgl_module);

//   is_cube_updated_func = (isCubeUpdated_T*) GetProcAddress(g_core_module,
//                  "_Java_com_maddox_il2_engine_Landscape_cIsCubeUpdated@8");
//   assert(is_cube_updated_func);

  g_initialized = true;

  printf("*** il2_core wrapper initialisation finished ***\n");
}
