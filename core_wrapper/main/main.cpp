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
#include "core_wrapper.h"
#include <sfs.h>
#include <wgl_wrapper.h>
#include <misc.h>
#include <il2ge/log.h>
#include <il2ge/exception_handler.h>
#include <il2ge/version.h>
#include <util.h>
#include <jni.h>
#include <config.h>

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


typedef jint JNICALL JNI_GetCreatedJavaVMs_t(JavaVM **, jsize, jsize *);

void installIATPatches(HMODULE);
HMODULE loadCoreWrapper(const char*);


constexpr jint g_jni_version = JNI_VERSION_1_2;
constexpr const char* const g_log_file_name = "il2ge.log";


il2ge::core_wrapper::Config g_config;
HMODULE g_core_wrapper_module = 0;
std::ofstream g_logfile;
bool g_core_wrapper_loaded = false;
JNI_GetCreatedJavaVMs_t *p_JNI_GetCreatedJavaVMs = nullptr;
JavaVM *g_java_vm = nullptr;


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


#if ENABLE_SHORTCUTS
BOOL WINAPI wrap_PeekMessageA(
  LPMSG lpMsg,
  HWND  hWnd,
  UINT  wMsgFilterMin,
  UINT  wMsgFilterMax,
  UINT  wRemoveMsg)
{
  auto ret = PeekMessageA(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
  if (ret)
  {
    if (lpMsg->message == WM_KEYDOWN)
    {
      cout<<"wrap_PeekMessageA\n";
      cout<<"WM_KEYDOWN arrived\n";
      exit(0);
    }
  }

  return ret;
}


BOOL WINAPI wrap_PeekMessageW(
  LPMSG lpMsg,
  HWND  hWnd,
  UINT  wMsgFilterMin,
  UINT  wMsgFilterMax,
  UINT  wRemoveMsg)
{
  auto ret = PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, PM_NOREMOVE);

  if (ret)
  {
    if (lpMsg->message == WM_KEYDOWN)
    {
      switch(lpMsg->wParam)
      {
        case 'E':
          PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, PM_REMOVE);
          core_gl_wrapper::toggleEnable();
          return false;
        case 'O':
          PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, PM_REMOVE);
          core_gl_wrapper::toggleObjectShaders();
          return false;
        case 'T':
          PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, PM_REMOVE);
          core_gl_wrapper::toggleTerrain();
          return false;
      }
    }

    return PeekMessageW(lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg);
  }

  return ret;
}
#endif


HMODULE WINAPI wrap_LoadLibraryA(LPCSTR libFileName)
{
  g_log << "LoadLibrary: " << libFileName << '\n';
  g_log.flush();

  string module_name = util::makeLowercase(util::basename(libFileName, true));

  if (module_name == "il2_core" ||
      module_name == "il2_corep4")
  {
    return loadCoreWrapper(libFileName);
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

  if (module_name == "dt")
  {
    jni_wrapper::resolveImports((void*)module);
  }

  return module;
}


FARPROC WINAPI wrap_GetProcAddress(HMODULE hModule, LPCSTR lpProcName)
{
  void *addr = jni_wrapper::getExport(lpProcName);
  if (addr)
    return (FARPROC) addr;

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
    return (FARPROC) sfs::get_openf_wrapper();
  }
#if ENABLE_SHORTCUTS
  else if (_stricmp(lpProcName, "PeekMessageA") == 0)
  {
    return (FARPROC) &wrap_PeekMessageA;
  }
  else if (_stricmp(lpProcName, "PeekMessageW") == 0)
  {
    return (FARPROC) &wrap_PeekMessageW;
  }
#endif

  return GetProcAddress(hModule, lpProcName);
}


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
    il2ge::core_wrapper::fatalError("DirectX mode is not supported.");
  }

  return module;
}


void installIATPatches(HMODULE module)
{
  patchIAT("LoadLibraryA", "kernel32.dll", (void*) &wrap_LoadLibraryA, 0, module);
  patchIAT("GetProcAddress", "kernel32.dll", (void*) &wrap_GetProcAddress, 0, module);
}


HMODULE loadCoreWrapper(const char *core_library_filename)
{
  assert(!g_core_wrapper_loaded);

  jsize num = 0;
  p_JNI_GetCreatedJavaVMs(&g_java_vm, 1, &num);
  assert(num == 1);
  assert(g_java_vm);

  HMODULE core_module = LoadLibraryA(core_library_filename);
  if (!core_module)
  {
    g_log << "Loading " << core_library_filename << " failed with error " << GetLastError() << '\n';
    g_log.flush();
    abort();
  }

  installIATPatches(core_module);
  jni_wrapper::resolveImports((void*)core_module);

  il2ge::core_wrapper::init(core_module);

  HMODULE jgl_module = GetModuleHandle("jgl.dll");
  assert(jgl_module);

  patchIAT("LoadLibraryA", "kernel32.dll",
           (void*) &wrap_JGL_LoadLibrary, NULL, jgl_module);
  patchIAT("GetProcAddress", "kernel32.dll",
           (void*) &wrap_JGL_GetProcAddress, NULL, jgl_module);

  g_core_wrapper_loaded = true;

  return core_module;
}


void fatalErrorHandler(const char *msg)
{
  if (g_java_vm)
  {
    JNIEnv *env = nullptr;
    g_java_vm->GetEnv((void**)&env, g_jni_version);

    if (env)
      env->FatalError(msg);
  }
}


void atexitHandler()
{
  g_log.flush();
  g_log.m_outputs.clear();
}


} // namespace


void il2ge::core_wrapper::fatalError(const std::string &message)
{
  g_log << "ERROR: " << message << '\n';
  g_log.flush();
  _Exit(1);
}


std::string il2ge::core_wrapper::getWrapperLibraryFilePath()
{
  char module_file_name[MAX_PATH];

  if (!GetModuleFileNameA(g_core_wrapper_module,
      module_file_name,
      sizeof(module_file_name)))
  {
    abort();
  }

  return module_file_name;
}


const il2ge::core_wrapper::Config &il2ge::core_wrapper::getConfig()
{
  return g_config;
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

    g_config.enable_dump = ini.GetBoolean("", "EnableDump", g_config.enable_dump);
    g_config.enable_base_map = ini.GetBoolean("", "EnableBaseMap", g_config.enable_base_map);
    g_config.enable_light_point = ini.GetBoolean("", "EnableLightPoint", g_config.enable_light_point);
  }

  il2ge::exception_handler::install(g_log_file_name, &fatalErrorHandler);
  il2ge::exception_handler::watchModule(g_core_wrapper_module);

  auto jvm_module = GetModuleHandle("jvm.dll");
  assert(jvm_module);

  p_JNI_GetCreatedJavaVMs = (JNI_GetCreatedJavaVMs_t*)
      GetProcAddress(jvm_module, "JNI_GetCreatedJavaVMs");
  assert(p_JNI_GetCreatedJavaVMs);

  installIATPatches(GetModuleHandle(0));
  installIATPatches(jvm_module);

#if ENABLE_SHORTCUTS
  patchIAT("PeekMessageA", "user32.dll",
           (void*) &wrap_PeekMessageA, NULL, GetModuleHandle(0));
  patchIAT("PeekMessageW", "user32.dll",
           (void*) &wrap_PeekMessageW, NULL, GetModuleHandle(0));
#endif

  jni_wrapper::init();
  jni_wrapper::resolveImports((void*)GetModuleHandle(0));
}


BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, void *reserved)
{
  switch (reason)
  {
    case DLL_PROCESS_ATTACH:
      g_core_wrapper_module = instance;
      break;
    case DLL_PROCESS_DETACH:
      break;
  }

  return true;
}


} // extern "C"
