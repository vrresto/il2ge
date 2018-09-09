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


using namespace std;

extern "C"
{
  BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, void *reserved);

  HRESULT WINAPI DirectInputCreateA(HINSTANCE hinst,
                                    DWORD dwVersion,
                                    void *ppDI,
                                    LPUNKNOWN punkOuter);
}


Logger g_log;


namespace
{


const char* const g_log_file_name = "il2ge.log";
const bool g_was_loaded_by_selector = true;

typedef HRESULT __stdcall DirectInputCreateA_T(HINSTANCE, DWORD, void*, LPUNKNOWN);

void installIATPatches(HMODULE);

HMODULE g_loader_module = 0;
HMODULE g_core_wrapper_module = 0;
HMODULE g_dinput_module = 0;
il2ge::CoreWrapperGetProcAddressFunc *g_core_wrapper_get_proc_address_f = 0;
DirectInputCreateA_T *g_directInputCreateA_func = 0;
std::ofstream g_logfile;


void initLog()
{
  auto out_fd = _fileno(stdout);

  auto res = _dup2(out_fd, 1);
  assert(res != -1);
  res = _dup2(out_fd, 2);
  assert(res != -1);
}


std::string getCoreWrapperFilePath()
{
  assert(g_core_wrapper_module);

  char module_file_name[MAX_PATH];

  if (!GetModuleFileNameA(g_core_wrapper_module,
      module_file_name,
      sizeof(module_file_name)))
  {
    abort();
  }

  return module_file_name;
}


LoaderInterface g_interface =
{
  &patchIAT,
  &getCoreWrapperFilePath
};


void lookForSelector()
{
  char exe_path[MAX_PATH];

  auto res = GetModuleFileName(GetModuleHandle(0), exe_path, sizeof(exe_path));
  if (!res || res == sizeof(exe_path))
    abort();

  auto dir = util::getDirFromPath(exe_path);
  assert(!dir.empty());

  auto dummy_dinput_path = dir + "/dinput.dll";

  auto dummy_dinput_module = GetModuleHandle(dummy_dinput_path.c_str());
  if (!dummy_dinput_module)
    abort();

  bool was_loaded_by_selector = GetProcAddress(dummy_dinput_module,
      "Java_com_maddox_sas1946_il2_util_BaseGameVersion_getSelectorInfo") != nullptr;

  if (was_loaded_by_selector)
  {
    g_log << "IL2GE was loaded by IL-2 Selector.\n";
    g_dinput_module = dummy_dinput_module;
  }
  else
  {
    g_log << "ERROR: IL2GE was NOT loaded by IL-2 Selector.\n";
    g_log.flush();
    abort();
  }

}


HMODULE loadDinputLibrary()
{
  assert(g_dinput_module);
  return g_dinput_module;

#if 0
  if (g_dinput_module)
    return g_dinput_module;

  assert(!g_was_loaded_by_selector);

  g_log << "Loading bin\\selector\\basefiles\\dinput.dll ...\n";

  g_dinput_module = LoadLibraryA("bin\\selector\\basefiles\\dinput.dll");
  if (g_dinput_module)
  {
    g_log << "Success.\n";
    g_log.flush();
    return g_dinput_module;
  }

  g_log << "Failed.\n";

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

  g_log << "Loading " << dinput_path << " ...\n";

  g_dinput_module = LoadLibraryA(dinput_path);
  if (!g_dinput_module)
  {
    g_log << "Loading " << dinput_path << " failed with error " << GetLastError() << '\n';
    g_log.flush();
    abort();
  }

  g_log << "Success.\n";
  g_log.flush();

  return g_dinput_module;
#endif
}


void loadCoreWrapper(const char *core_library_filename)
{
  assert(!g_core_wrapper_module);

  HMODULE core_module = LoadLibraryA(core_library_filename);
  if (!core_module)
  {
    g_log << "Loading " << core_library_filename << " failed with error " << GetLastError() << '\n';
    g_log.flush();
    abort();
  }
  installIATPatches(core_module);

#ifdef STATIC_CORE_WRAPPER
  HMODULE wrapper_module = g_loader_module;
  g_core_wrapper_get_proc_address_f = &il2ge_coreWrapperGetProcAddress;
  il2ge::CoreWrapperInitFunc *wrapper_init_f = &il2ge_coreWrapperInit;
#else
  HMODULE wrapper_module = LoadLibraryA(IL2GE_LIB_DIR "/" CORE_WRAPPER_LIBRARY_NAME ".dll");
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
  g_log << "LoadLibrary: " << libFileName << '\n';
  g_log.flush();

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
    initLog();
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


void atexitHandler()
{
  g_log.flush();
  g_log.m_outputs.clear();
}


} // namespace


HMODULE getLoaderModule()
{
  return g_loader_module;
}


HMODULE getCoreWrapperModule()
{
  return g_core_wrapper_module;
}


const char *getLogFileName()
{
  return g_log_file_name;
}


void fatalError(const std::string &message)
{
  g_log << "ERROR: " << message << '\n';
  g_log.flush();
  _Exit(1);
}


extern "C"
{

#if 0
HRESULT WINAPI DirectInputCreateA(HINSTANCE hinst, DWORD dwVersion, void *ppDI, LPUNKNOWN punkOuter)
{
  if (!g_directInputCreateA_func)
  {
    HMODULE dinput_module = loadDinputLibrary();
    assert(!g_was_loaded_by_selector);
    g_directInputCreateA_func = (DirectInputCreateA_T*) GetProcAddress(dinput_module, "DirectInputCreateA");
    assert(g_directInputCreateA_func);
  }

  return g_directInputCreateA_func(hinst, dwVersion, ppDI, punkOuter);
}
#endif


void WINAPI il2ge_init()
{
  std::atexit(atexitHandler);

  {
    // clear previous contents
    ofstream all_log("il2ge_all.log");
  }

  freopen("il2ge_all.log", "a", stdout);
  freopen("il2ge_all.log", "a", stderr);

  g_log.m_outputs.push_back(&cerr);

  g_logfile.open(getLogFileName(), ios_base::app);
  if (g_logfile.good())
    g_log.m_outputs.push_back(&g_logfile);

  g_log.printSeparator();
  g_log << "*** il2ge.dll initialization ***\n";
  g_log.flush();

  initLog();
  installExceptionHandler();
  installIATPatches(GetModuleHandle(0));
  lookForSelector();
  installIATPatches(GetModuleHandle("jvm.dll"));
}


BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, void *reserved)
{
  switch (reason)
  {
    case DLL_PROCESS_ATTACH:
      g_loader_module = instance;
      break;
    case DLL_PROCESS_DETACH:
      break;
  }

  return true;
}


} // extern "C"
