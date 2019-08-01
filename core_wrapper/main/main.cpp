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
#include <configuration.h>

#include <INIReader.h>

#include <iostream>
#include <sstream>
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


class LogBuf : public std::stringbuf
{
protected:
  int sync() override
  {
    fwrite(str().data(), sizeof(char), str().size(), stdout);
    fflush(stdout);

    str({});

    return 0;
  }
};


typedef jint JNICALL JNI_GetCreatedJavaVMs_t(JavaVM **, jsize, jsize *);

void installIATPatches(HMODULE);
HMODULE loadCoreWrapper(const char*);


constexpr jint g_jni_version = JNI_VERSION_1_2;
constexpr const char* const LOG_FILE_NAME = "il2ge.log";
constexpr const char* const CONFIG_FILE_NAME = "il2ge.ini";


il2ge::core_wrapper::Configuration g_config;
HMODULE g_core_wrapper_module = 0;
std::ofstream g_logfile;
bool g_core_wrapper_loaded = false;
JNI_GetCreatedJavaVMs_t *p_JNI_GetCreatedJavaVMs = nullptr;
JavaVM *g_java_vm = nullptr;
DWORD g_main_thread = 0;
LogBuf g_cout_buf;
LogBuf g_cerr_buf;


bool readConfig()
{
  bool success = false;

  INIReader ini(CONFIG_FILE_NAME);

  if (!ini.ParseError())
  {
    auto read_value = [&ini] (std::string name)
    {
      return ini.Get("", name, "");
    };

    g_config.read(read_value);

    success = true;
  }
  else if (ini.ParseError() != -1)
  {
    g_log << "Error reading configuration file " << CONFIG_FILE_NAME
      << " at line " << ini.ParseError() << "\n"
      << "Using default configuration.\n";
  }
  else
  {
    success = true;
  }

  g_log << "\n*** IL2GE Configuration ***\n";

  for (auto &setting : g_config.getSettings())
  {
    g_log << setting->getName() << ": " << setting->getValueStr() << "\n";
  }

  g_log << "\n";
  g_log.flush();

  return success;
}


void writeConfig()
{
  ofstream config_out(CONFIG_FILE_NAME);
  assert(config_out.good());
  g_config.write(config_out);
}


void redirectOutput()
{
  freopen(LOG_FILE_NAME, "a", stdout);
  freopen(LOG_FILE_NAME, "a", stderr);

  auto out_fd = _fileno(stdout);

  auto res = _dup2(out_fd, 1);
  assert(res != -1);
  res = _dup2(out_fd, 2);
  assert(res != -1);

  cout.rdbuf(&g_cout_buf);
  cerr.rdbuf(&g_cerr_buf);
}


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

  if (util::isPrefix("Java_", lpProcName))
  {
    auto addr = GetProcAddress(g_core_wrapper_module, lpProcName);
    if (addr)
      return addr;
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
    return (FARPROC) sfs::get_openf_wrapper();
  }


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


const il2ge::core_wrapper::Configuration &il2ge::core_wrapper::getConfig()
{
  return g_config;
}


JNIEnv_ *il2ge::core_wrapper::getJNIEnv()
{
  JNIEnv_ *env = nullptr;
  if (g_java_vm)
    g_java_vm->GetEnv((void**)&env, g_jni_version);
  return env;
}


bool il2ge::core_wrapper::isMainThread()
{
  assert(g_main_thread);
  return GetCurrentThreadId() == g_main_thread;
}


extern "C"
{


void WINAPI il2ge_init()
{
  std::atexit(atexitHandler);

  ofstream log(LOG_FILE_NAME);

  g_log.m_outputs.push_back(&cout);
  g_log.m_outputs.push_back(&log);

  g_log.printSeparator();
  g_log << "*** il2ge.dll initialization ***\n";
  g_log << "Build: " << il2ge::version::getBuildJobID() << '\n';
  g_log << "Debug: " << il2ge::version::isDebugBuild() << '\n';
  g_log << "Commit: " << il2ge::version::getCommitSHA() << '\n';
  g_log.flush();

  if (readConfig())
    writeConfig();

  if (!g_config.enable_graphics_extender)
  {
    g_log << "IL2GE is disabled in config.\n";
    g_log.flush();
    g_log.m_outputs.clear();
    return;
  }

  g_log.m_outputs.clear();
  log.close();

  g_log.m_outputs.push_back(&cout);

  redirectOutput();

  g_main_thread = GetCurrentThreadId();

  il2ge::exception_handler::install(LOG_FILE_NAME, &fatalErrorHandler);
  il2ge::exception_handler::watchModule(g_core_wrapper_module);

  auto jvm_module = GetModuleHandle("jvm.dll");
  assert(jvm_module);

  p_JNI_GetCreatedJavaVMs = (JNI_GetCreatedJavaVMs_t*)
      GetProcAddress(jvm_module, "JNI_GetCreatedJavaVMs");
  assert(p_JNI_GetCreatedJavaVMs);

  installIATPatches(GetModuleHandle(0));
  installIATPatches(jvm_module);

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
