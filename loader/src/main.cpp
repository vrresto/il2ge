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

#include "iat.h"
#include <loader_interface.h>
#include <il2ge/core_wrapper.h>
#include <util.h>

#include <iostream>
#include <unordered_map>
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

const char* const crash_handler_library_name =
    IL2GE_DATA_DIR "/mingw_crash_handler.dll";

const char* const crash_handler_func_name = "crashHandler";

typedef void CrashHandlerFunc(PEXCEPTION_POINTERS pExceptionInfo);
typedef int __stdcall SFS_openf_T (unsigned __int64 hash, int flags);
typedef HRESULT __stdcall DirectInputCreateA_T(HINSTANCE, DWORD, void*, LPUNKNOWN);

void installIATPatches(HMODULE);

unordered_map<__int64, __int64> g_sfs_redirections;
HMODULE g_loader_module = 0;
HMODULE g_core_wrapper_module = 0;
SFS_openf_T *g_sfs_openf_f = 0;
il2ge::CoreWrapperGetProcAddressFunc *g_core_wrapper_get_proc_address_f = 0;
DirectInputCreateA_T *g_directInputCreateA_func = 0;


LONG WINAPI vectoredExceptionHandler(_EXCEPTION_POINTERS *info)
{
  static LONG num_entered_handlers = 0;

  if (InterlockedIncrement(&num_entered_handlers) != 1)
  {
    abort();
  }

  fprintf(stderr, "\nException code: %u  Flags: %u\n",
          info->ExceptionRecord->ExceptionCode,
          info->ExceptionRecord->ExceptionFlags);

  HMODULE crash_handler_module = LoadLibraryA(crash_handler_library_name);

  if (crash_handler_module)
  {
    CrashHandlerFunc *crash_handler = (CrashHandlerFunc*)
        GetProcAddress(crash_handler_module, crash_handler_func_name);
    assert(crash_handler);
    crash_handler(info);
  }
  else
  {
    fprintf(stderr, "\n**** could not load %s - backtrace disabled ****\n\n",
            crash_handler_library_name);
  }

  InterlockedDecrement(&num_entered_handlers);

  return EXCEPTION_CONTINUE_SEARCH;
}


void installExceptionHandler()
{
  AddVectoredExceptionHandler(true, &vectoredExceptionHandler);
}


void sfsRedirect(__int64 hash, __int64 hash_redirection)
{
  g_sfs_redirections[hash] = hash_redirection;
}


void sfsClearRedirections()
{
  g_sfs_redirections.clear();
}


LoaderInterface g_interface = { &sfsRedirect, &sfsClearRedirections, &patchIAT };


int WINAPI wrap_SFS_openf(const unsigned __int64 hash_, const int flags)
{
  unsigned __int64 hash = hash_;

  auto it = g_sfs_redirections.find(hash);
  if (it != g_sfs_redirections.end())
    hash = it->second;
  return g_sfs_openf_f(hash, flags);
}


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

  wrapper_init_f(core_module, &g_interface);

  g_core_wrapper_module = wrapper_module;
}


HMODULE WINAPI wrap_LoadLibraryA(LPCSTR libFileName)
{
  printf("LoadLibrary: %s\n", libFileName);

  string module_name = util::makeLowercase(util::basename(libFileName, true));

  if (module_name.compare("il2_core") == 0 ||
      module_name.compare("il2_corep4") == 0 )
  {
    loadCoreWrapper(libFileName);
    return g_core_wrapper_module;
  }

  if (module_name.compare("dinput") == 0)
  {
    return loadDinputLibrary();
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

  if (module_name.compare("wrapper") == 0)
  {
    g_sfs_openf_f = (SFS_openf_T*) GetProcAddress(module, "__SFS_openf");
    assert(g_sfs_openf_f);
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
    return (FARPROC)&wrap_SFS_openf;
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
      installExceptionHandler();
      g_loader_module = instance;
      installIATPatches(GetModuleHandle(0));
      loadDinputLibrary();
      break;
    case DLL_PROCESS_DETACH:
      break;
  }

  return true;
}


} // extern "C"
