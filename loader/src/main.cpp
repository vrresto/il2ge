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
#include <il2ge/log.h>
#include <il2ge/exception_handler.h>
#include <il2ge/core_wrapper.h>
#include <il2ge/version.h>
#include <util.h>

#include <INIReader.h>

#include <iostream>
#include <windows.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>

#ifndef _stricmp
#define _stricmp strcasecmp
#endif


extern "C" void WINAPI il2ge_init();


using namespace std;

extern "C"
{
  BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, void *reserved);
}


Logger g_log;


namespace
{


const char* const g_log_file_name = "il2ge.log";

void installIATPatches(HMODULE);

HMODULE g_loader_module = 0;
HMODULE g_core_wrapper_module = 0;
il2ge::CoreWrapperGetProcAddressFunc *g_core_wrapper_get_proc_address_f = 0;
std::ofstream g_logfile;


void initLog()
{
  g_log.m_outputs.push_back(&cerr);

  {
    // clear previous contents
    ofstream log(g_log_file_name);
  }

  freopen(g_log_file_name, "a", stdout);
  freopen(g_log_file_name, "a", stderr);

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

  il2ge::exception_handler::watchModule(wrapper_module);

  wrapper_init_f(core_module, &g_interface);
}


HMODULE WINAPI wrap_LoadLibraryA(LPCSTR libFileName)
{
  g_log << "LoadLibrary: " << libFileName << '\n';
  g_log.flush();

  string module_name = util::makeLowercase(util::basename(libFileName, true));

  if (module_name == "il2_core" ||
      module_name == "il2_corep4")
  {
    loadCoreWrapper(libFileName);
    return g_core_wrapper_module;
  }

  HMODULE module = LoadLibraryA(libFileName);

  if (module_name == "jvm" ||
      module_name == "dt" ||
      module_name == "jgl" ||
      module_name == "hpi" ||
      module_name == "wrapper")
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


void fatalError(const std::string &message)
{
  g_log << "ERROR: " << message << '\n';
  g_log.flush();
  _Exit(1);
}


extern "C"
{


void WINAPI il2ge_init()
{
  std::atexit(atexitHandler);

  initLog();

  g_log.printSeparator();
  g_log << "*** il2ge.dll initialization ***\n";
  g_log << "Build: " << il2ge::version::getBuildJobID() << '\n';
  g_log << "Debug: " << il2ge::version::isDebugBuild() << '\n';
  g_log << "Commit: " << il2ge::version::getCommitSHA() << '\n';
  g_log.flush();

  INIReader ini("il2ge.ini");
  if (!ini.ParseError())
  {
    if (!ini.GetBoolean("", "EnableGE", true))
    {
      g_log << "IL2GE is disabled in config.\n";
      g_log.flush();
      return;
    }
  }

  il2ge::exception_handler::install(g_log_file_name);
  il2ge::exception_handler::watchModule(g_loader_module);

  auto jvm_module = GetModuleHandle("jvm.dll");
  assert(jvm_module);

  installIATPatches(GetModuleHandle(0));
  installIATPatches(jvm_module);
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
