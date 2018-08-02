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

#include <iostream>
#include <unordered_map>
#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>

/*
#undef assert
#define assert(c) \
{ \
  if (!(c)) { \
    printf("%s:%d: assertion %s failed.\n", __FILE__, __LINE__,  #c); \
    exit(1); \
  } \
}
*/

#ifndef _stricmp
#define _stricmp strcasecmp
#endif


extern "C"
{

  int CDECL _splitpath_s(
    const char * path,  
    char * drive,  
    size_t driveNumberOfElements,  
    char * dir,  
    size_t dirNumberOfElements,  
    char * fname,  
    size_t nameNumberOfElements,  
    char * ext,   
    size_t extNumberOfElements  
  );  

}


namespace
{


using namespace std;

typedef void CoreWrapperInitFunc(HMODULE core_module, const LoaderInterface *loader);
typedef void* CoreWrapperGetProcAddressFunc(const char* name);
typedef int __stdcall SFS_openf_T (unsigned __int64 hash, int flags);
typedef HRESULT DirectInputCreateA_T(HINSTANCE, DWORD, void*, LPUNKNOWN);

void installIATPatches(HMODULE);

unordered_map<__int64, __int64> g_sfs_redirections;
bool g_is_core_wrapper_loaded = false;
HMODULE g_core_wrapper_module = 0;
SFS_openf_T *g_sfs_openf_f = 0;
CoreWrapperGetProcAddressFunc *g_core_wrapper_get_proc_address_f = 0;
DirectInputCreateA_T *g_directInputCreateA_func = 0;


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


HMODULE WINAPI wrap_LoadLibraryA(LPCSTR libFileName)
{
  printf("LoadLibrary: %s\n", libFileName);

  char module_name[100];

  int err = _splitpath_s(libFileName, NULL, 0, NULL, 0, module_name, sizeof(module_name), NULL, 0);
  assert(!err);

  if (_stricmp(module_name, "il2_core") == 0 || _stricmp(module_name, "il2_corep4") == 0 )
  {
    if (!g_is_core_wrapper_loaded)
    {
      HMODULE core_module = LoadLibraryA(libFileName);
      if (!core_module)
      {
        printf("LoadLibraryA() failed with error %u.\n", GetLastError());
        exit(1);
      }
      installIATPatches(core_module);

      HMODULE wrapper_module = LoadLibraryA("il2_core_wrapper.dll");
      if (!wrapper_module)
      {
        printf("LoadLibraryA() failed with error %u.\n", GetLastError());
        exit(1);
      }

      g_core_wrapper_get_proc_address_f =
        (CoreWrapperGetProcAddressFunc*) GetProcAddress(wrapper_module, "coreWrapperGetProcAddress");
      assert(g_core_wrapper_get_proc_address_f);

      CoreWrapperInitFunc *wrapper_init_f = (CoreWrapperInitFunc*) GetProcAddress(wrapper_module, "coreWrapperInit");
      assert(wrapper_init_f);
      wrapper_init_f(core_module, &g_interface);

      g_is_core_wrapper_loaded = true;
      g_core_wrapper_module = wrapper_module;

      return wrapper_module;
    }
    else
    {
      assert(0);
    }
  }

  if (_stricmp(module_name, "dinput") == 0)
  {
    return loadDinputLibrary();
  }

  HMODULE module = LoadLibraryA(libFileName);

  if (_stricmp(module_name, "jvm") == 0 ||
      _stricmp(module_name, "dt") == 0 ||
      _stricmp(module_name, "jgl") == 0 ||
      _stricmp(module_name, "hpi") == 0 ||
      _stricmp(module_name, "wrapper") == 0)
  {
    installIATPatches(module);
  }

  if (_stricmp(module_name, "wrapper") == 0)
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
      installIATPatches(GetModuleHandle(0));
      loadDinputLibrary();
      break;
    case DLL_PROCESS_DETACH:
      break;
  }

  return true;
}


} // extern "C"
