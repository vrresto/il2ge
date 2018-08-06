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

#include <sfs.h>
#include <render_util/render_util.h>
#include <misc.h>
#include <core.h>

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


extern "C"
{
  void coreWrapperInit(HMODULE core_module, const LoaderInterface *loader);
  void* coreWrapperGetProcAddress(const char *name);
  BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, void *reserved);
}


namespace
{


struct ContextData : public Module
{
  gl_wrapper::GL_Interface *interface = nullptr;
  WGL_Interface *wgl_interface = nullptr;
};


bool g_initialized = false;
const LoaderInterface *g_loader = nullptr;
HMODULE g_core_module = 0;
HMODULE g_gl_module = 0;
ContextData *current_context = nullptr;

std::unordered_map<HGLRC, ContextData*> g_data_for_context;

typedef PROC WINAPI GetProcAddressFunc(LPCSTR);

GetProcAddressFunc *getProcAddressFunc = nullptr;

typedef BOOL WINAPI wglMakeCurrent_t(HDC, HGLRC);

wglMakeCurrent_t *real_wglMakeCurrent = nullptr;

typedef int __stdcall isCubeUpdated_T(void*, void*);
isCubeUpdated_T *is_cube_updated_func = nullptr;


void signalHandler (int sig)
{
  if (sig == SIGABRT)
  {
    fprintf(stderr, "aborted.\n");
    fflush(stderr);
    quick_exit(1);
  }
  if (sig == SIGSEGV)
  {
    fprintf(stderr, "SIGSEGV\n");
    fflush(stderr);
    quick_exit(1);
  }
}


BOOL WINAPI wrap_wglMakeContextCurrentARB(HDC hDrawDC, HDC hReadDC, HGLRC hglrc)
{
  printf("wglMakeContextCurrentARB\n");
//   if (current_WGL_Interface()->wglMakeContextCurrentARB(hDrawDC, hReadDC, hglrc)) {
    exit(0);
//   }
//   else
//     return false;
}


void *getProcAddress_ext(const char *name)
{
  void *func = (void*) GetProcAddress(g_gl_module, name);
  if (!func) {
    func = (void*) getProcAddressFunc(name);
  }

  return func;
}


PROC WINAPI wrap_wglGetProcAddress(LPCSTR name)
{
  printf("wglGetProcAddress: %s\n", name);

  if (strcmp(name, "wglMakeContextCurrentARB") == 0)
  {
    return (PROC) &wrap_wglMakeContextCurrentARB;
  }

  assert(getProcAddressFunc);

  PROC proc = getProcAddressFunc(name);

  void *wrap_proc = core_gl_wrapper::getProc(name);
  if (wrap_proc && proc)
    return (PROC) wrap_proc;

  return proc;
}


BOOL WINAPI wrap_wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
  if (real_wglMakeCurrent(hdc, hglrc))
  {
    if (!hglrc)
    {
      current_context = nullptr;
      gl_wrapper::setCurrent_GL_interface(nullptr);
      return true;
    }

    current_context = g_data_for_context[hglrc];
    if (!current_context)
    {
      ContextData *d = new ContextData;

      d->interface = new gl_wrapper::GL_Interface(&getProcAddress_ext);

      d->wgl_interface = new WGL_Interface;
      d->wgl_interface->init(&getProcAddress_ext);

      g_data_for_context[hglrc] = d;
      current_context = d;
    }

    gl_wrapper::setCurrent_GL_interface(current_context->interface);

    return true;
  }
  else
    return false;
}


FARPROC WINAPI wrap_JGL_GetProcAddress(HMODULE module, LPCSTR name)
{
  printf("jgl => GetProcAddress: %s\n", name);

  FARPROC proc = GetProcAddress(module, name);

  if (strcmp(name, "wglGetProcAddress") == 0)
  {
    getProcAddressFunc = (GetProcAddressFunc*) proc;
    return (FARPROC) &wrap_wglGetProcAddress;
  }

  if (strcmp(name, "wglMakeCurrent") == 0)
  {
    real_wglMakeCurrent = (wglMakeCurrent_t*) proc;
    return (FARPROC) &wrap_wglMakeCurrent;
  }

  FARPROC wrap_proc = (FARPROC) core_gl_wrapper::getProc(name);
  if (wrap_proc && proc)
    return wrap_proc;

  return proc;
}


HMODULE WINAPI wrap_JGL_LoadLibrary(LPCSTR libFileName)
{
  printf("jgl => LoadLibrary: %s\n", libFileName);

  HMODULE module = LoadLibraryA(libFileName);

  if (strcasecmp(libFileName, "opengl32.dll") == 0)
    g_gl_module = module;
  else
     assert(0);

  return g_gl_module;
}


} // namespace


// const WGL_Interface *current_WGL_Interface() {
//   if (current_context)
//     return current_context->wgl_interface;
//   else
//     return 0;
// }


Module *getGLContext()
{
  assert(current_context);
  return current_context;
}


void *getOrigProcAddress(const char *name)
{
  void *func = reinterpret_cast<void*>(GetProcAddress(g_core_module, name));
  if (func)
    return func;
  else
    return 0;
}


namespace render_util
{
  const std::string &getDataPath()
  {
    static std::string path = "ge";
    return path;
  }

  const std::string &getResourcePath()
  {
    return getDataPath();
  }
}


namespace SFS
{
  void redirect(__int64 hash, __int64 hash_redirection)
  {
    g_loader->sfsRedirect(hash, hash_redirection);
  }

  void clearRedirections()
  {
    g_loader->sfsClearRedirections();
  }
}


namespace core
{
  bool isCubeUpdated()
  {
    return is_cube_updated_func(0, 0);
  }
}


extern "C"
{


void *coreWrapperGetProcAddress(const char *name)
{
  void *addr = nullptr;

  if (g_initialized)
    addr = jni_wrapper::getExport(name);

  if (!addr)
    addr = getOrigProcAddress(name);

  return addr;
}


void coreWrapperInit(HMODULE core_module_, const LoaderInterface *loader)
{
  printf("*** il2_core wrapper initialisation ***\n");

  g_core_module = core_module_;
  assert(g_core_module);

  g_loader = loader;
  assert(g_loader);

  signal(SIGABRT, signalHandler);
//   signal(SIGSEGV, signalHandler);

  SFS::init();
  core_gl_wrapper::init();
  jni_wrapper::init();

  HMODULE jgl_module = GetModuleHandle("jgl.dll");
  assert(jgl_module);

  g_loader->patchIAT("LoadLibraryA", "kernel32.dll",
      (void*) &wrap_JGL_LoadLibrary, NULL, jgl_module);
  g_loader->patchIAT("GetProcAddress", "kernel32.dll",
      (void*) &wrap_JGL_GetProcAddress, NULL, jgl_module);

  is_cube_updated_func = (isCubeUpdated_T*) GetProcAddress(g_core_module,
                 "_Java_com_maddox_il2_engine_Landscape_cIsCubeUpdated@8");
  assert(is_cube_updated_func);

  g_initialized = true;

  printf("*** il2_core wrapper initialisation finished ***\n");
}


BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, void *reserved)
{
  switch (reason)
  {
    case DLL_PROCESS_ATTACH:
        printf("*** il2_core wrapper process attach ***\n");
      break;
    case DLL_PROCESS_DETACH:
      break;
  }

  return TRUE;
}


} // extern "C"