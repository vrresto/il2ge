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

#include "loader.h"
#include "iat.h"
#include <loader_interface.h>
#include <il2ge/core_wrapper.h>
#include <util.h>

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>

#ifndef _stricmp
#define _stricmp strcasecmp
#endif


extern "C"
{
  BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, void *reserved);

  HRESULT WINAPI DirectInputCreateA(HINSTANCE hinst,
                                    DWORD dwVersion,
                                    void *ppDI,
                                    LPUNKNOWN punkOuter);
}


namespace
{


using namespace std;

typedef HRESULT __stdcall DirectInputCreateA_T(HINSTANCE, DWORD, void*, LPUNKNOWN);

void installIATPatches(HMODULE);

HMODULE g_loader_module = 0;
HMODULE g_core_wrapper_module = 0;
il2ge::CoreWrapperGetProcAddressFunc *g_core_wrapper_get_proc_address_f = 0;
DirectInputCreateA_T *g_directInputCreateA_func = 0;


std::string getCoreWrapperFilePath()
{
  assert(g_core_wrapper_module);

  char module_file_name[MAX_PATH];

  if (!GetModuleFileNameA(g_core_wrapper_module,
      module_file_name,
      sizeof(module_file_name)))
  {
    assert(false);
  }

  return module_file_name;
}


LoaderInterface g_interface =
{
  &patchIAT,
  &getCoreWrapperFilePath
};


HMODULE loadDinputLibrary()
{
  HMODULE module = LoadLibraryA("bin\\selector\\basefiles\\dinput.dll");
  if (module)
    return module;

  static char dinput_path[MAX_PATH];

  const char *dinput_dir = getenv("DINPUT_DIR");
  if (dinput_dir)
  {
    snprintf(dinput_path, sizeof(dinput_path), "%s\\dinput.dll", dinput_dir);
  }
  else
  {
    const char *system_root = getenv("SystemRoot");
    assert(system_root);
    snprintf(dinput_path, sizeof(dinput_path), "%s\\system32\\dinput.dll", system_root);
  }

  module = LoadLibraryA(dinput_path);
  if (!module)
  {
    printf("LoadLibraryA() failed with error %u.\n", GetLastError());
    exit(1);
  }
  return module;
}


void loadCoreWrapper(const char *core_library_filename)
{
  assert(!g_core_wrapper_module);

  HMODULE core_module = LoadLibraryA(core_library_filename);
  if (!core_module)
  {
    printf("LoadLibraryA() failed with error %u.\n", GetLastError());
    exit(1);
  }
  installIATPatches(core_module);

#ifdef STATIC_CORE_WRAPPER
  HMODULE wrapper_module = g_loader_module;
  g_core_wrapper_get_proc_address_f = &il2ge_coreWrapperGetProcAddress;
  il2ge::CoreWrapperInitFunc *wrapper_init_f = &il2ge_coreWrapperInit;
#else
  HMODULE wrapper_module = LoadLibraryA(IL2GE_DATA_DIR "/" CORE_WRAPPER_LIBRARY_NAME ".dll");
  if (!wrapper_module)
  {
    printf("LoadLibraryA() failed with error %u.\n", GetLastError());
    exit(1);
  }

  g_core_wrapper_get_proc_address_f =
    (il2ge::CoreWrapperGetProcAddressFunc*) GetProcAddress(wrapper_module, "il2ge_coreWrapperGetProcAddress");
  assert(g_core_wrapper_get_proc_address_f);

  il2ge::CoreWrapperInitFunc *wrapper_init_f =
    (il2ge::CoreWrapperInitFunc*) GetProcAddress(wrapper_module, "il2ge_coreWrapperInit");
  assert(wrapper_init_f);
#endif

  g_core_wrapper_module = wrapper_module;

  wrapper_init_f(core_module, &g_interface);
}


HMODULE WINAPI wrap_LoadLibraryA(LPCSTR libFileName)
{
  printf("LoadLibrary: %s\n", libFileName);

  string module_name = util::makeLowercase(util::basename(libFileName, true));

  if (module_name.compare("il2_core") == 0 ||
      module_name.compare("il2_corep4") == 0)
  {
    loadCoreWrapper(libFileName);
    return g_core_wrapper_module;
  }
  else if (module_name.compare("dinput") == 0)
  {
    return loadDinputLibrary();
  }
  else if (module_name.compare("jvm") == 0)
  {
    loadDinputLibrary();
  }

  HMODULE module = LoadLibraryA(libFileName);

  if (module_name.compare("jvm") == 0 ||
      module_name.compare("dt") == 0 ||
      module_name.compare("jgl") == 0 ||
      module_name.compare("hpi") == 0 ||
      module_name.compare("wrapper") == 0)
  {
    installIATPatches(module);
  }

  return module;
}


FARPROC WINAPI wrap_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
  if (g_core_wrapper_module && hModule == g_core_wrapper_module)
  {
    assert(g_core_wrapper_get_proc_address_f);
    return (FARPROC) g_core_wrapper_get_proc_address_f(lpProcName);
  }

  if (_stricmp(lpProcName, "LoadLibraryA") == 0)
  {
    return (FARPROC)&wrap_LoadLibraryA;
  }
  else if (_stricmp(lpProcName, "GetProcAddress") == 0)
  {
    return (FARPROC)&wrap_GetProcAddress;
  }
  else if (_stricmp(lpProcName, "__SFS_openf") == 0)
  {
    return (FARPROC) il2ge::get_SFS_openf_wrapper();
  }

  return GetProcAddress(hModule, lpProcName);
}


void installIATPatches(HMODULE module)
{
  patchIAT("LoadLibraryA", "kernel32.dll", (void*) &wrap_LoadLibraryA, 0, module);
  patchIAT("GetProcAddress", "kernel32.dll", (void*) &wrap_GetProcAddress, 0, module);
}


} // namespace


extern "C"
{


void abort()
{
  static long handler_entered = 0;

  if (InterlockedIncrement(&handler_entered) < 3)
  {
    fprintf(stderr, "\naborted.\n");
    printBacktrace();
  }

  TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
  SuspendThread(GetCurrentThread());
  _Exit(EXIT_FAILURE);
}


HRESULT WINAPI DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, void *ppDI, LPUNKNOWN punkOuter)
{
  if (!g_directInputCreateA_func)
  {
    HMODULE dinput_module = loadDinputLibrary();
    g_directInputCreateA_func = (DirectInputCreateA_T*) GetProcAddress(dinput_module, "DirectInputCreateA");
    assert(g_directInputCreateA_func);
  }

  return g_directInputCreateA_func(hinst, dwVersion, ppDI, punkOuter);
}


BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, void *reserved)
{
  switch (reason)
  {
    case DLL_PROCESS_ATTACH:
      printf("*** loader dll process attach ***\n");

      g_loader_module = instance;
      installExceptionHandler();
      installIATPatches(GetModuleHandle(0));

      break;
    case DLL_PROCESS_DETACH:
      break;
  }

  return true;
}


} // extern "C"
