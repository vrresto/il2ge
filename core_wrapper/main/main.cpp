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
#include "configuration_p.h"
#include <sfs.h>
#include <wgl_wrapper.h>
#include <misc.h>
#include <il2ge/exception_handler.h>
#include <il2ge/version.h>
#include <util.h>
#include <jni.h>
#include <mutex_locker.h>
#include <config.h>
#include <log.h>
#include <log/file_appender.h>
#include <log/console_appender.h>
#include <log/txt_formatter.h>
#include <log/message_only_formatter.h>

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


extern "C"
{
  void WINAPI il2geInit();
  BOOL WINAPI DllMain(HINSTANCE instance, DWORD reason, void *reserved);
}


using namespace std;
using il2ge::core_wrapper::MutexLocker;


namespace
{


#if !USE_PLOG
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
#endif


void installIATPatches(HMODULE);
HMODULE loadCoreWrapper(const char*);


constexpr jint JNI_VERSION = JNI_VERSION_1_2;
constexpr auto LOG_FILE_NAME = "il2ge.log";
constexpr auto LOG_FULL_FILE_NAME = "il2ge_full.log";


HMODULE g_core_wrapper_module = 0;
bool g_core_wrapper_loaded = false;
decltype(JNI_GetCreatedJavaVMs) *g_get_created_java_vms = nullptr;
JavaVM *g_java_vm = nullptr;
DWORD g_main_thread = 0;

bool g_fatal_error = false;
CRITICAL_SECTION g_fatal_error_mutex;

#if !USE_PLOG
LogBuf g_cout_buf;
LogBuf g_cerr_buf;
#endif


void initLog()
{
#if USE_PLOG

  constexpr bool ADD_NEW_LINE = false;

  using namespace util::log;
  using FileSink = FileAppender<TxtFormatter<ADD_NEW_LINE>>;
  using ConsoleSink = ConsoleAppender<MessageOnlyFormatter<ADD_NEW_LINE>>;

  try
  {
    auto &logger_default = plog::init(plog::verbose);

    #if LOG_TO_CONSOLE
    {
      static ConsoleSink sink;
      auto &logger = plog::init<LOGGER_INFO>(plog::info, &sink);
      logger_default.addAppender(&logger);
    }
    #endif

    {
      static FileSink sink(LOG_FILE_NAME);
      auto &logger = plog::init<LOGGER_DEBUG>(plog::debug, &sink);
      logger_default.addAppender(&logger);
    }

    {
      static FileSink sink(LOG_FULL_FILE_NAME);
      auto &logger = plog::init<LOGGER_TRACE>(plog::verbose, &sink);
      logger_default.addAppender(&logger);
    }
  }
  catch (std::exception &e)
  {
    auto message = std::string(e.what()) + "\n\rTerminating program.";
    MessageBoxA(nullptr, message.c_str(), "IL-2 Graphics Extender", MB_OK | MB_ICONERROR);
    _Exit(EXIT_FAILURE);
  }

#else

  {
    ofstream log(LOG_FILE_NAME);
    log << endl;
  }

  freopen(LOG_FILE_NAME, "a", stdout);
  freopen(LOG_FILE_NAME, "a", stderr);

  auto out_fd = _fileno(stdout);

  auto res = _dup2(out_fd, 1);
  assert(res != -1);
  res = _dup2(out_fd, 2);
  assert(res != -1);

  cout.rdbuf(&g_cout_buf);
  cerr.rdbuf(&g_cerr_buf);

#endif
}


#if USE_PLOG
int wrap_write(int fd, const void *buffer, unsigned int count)
{
  if (fd <= 2)
  {
    string str((char*)buffer, count);

    MutexLocker lock(g_fatal_error_mutex);

    plog::Record record(g_fatal_error ? plog::error : plog::verbose);
    record << str;
    *plog::get<PLOG_DEFAULT_INSTANCE>() += record;

    return count;
  }
  else
  {
    return _write(fd, buffer, count);
  }
}
#endif


HMODULE WINAPI wrap_LoadLibraryA(LPCSTR libFileName)
{
  LOG_INFO << "LoadLibrary: " << libFileName << endl;
  LOG_FLUSH;

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
  LOG_TRACE << "jgl => GetProcAddress: " << name << endl;

  return (FARPROC) wgl_wrapper::getProcAddress(module, name);
}


HMODULE WINAPI wrap_JGL_LoadLibrary(LPCSTR libFileName)
{
  LOG_DEBUG << "jgl => LoadLibrary: " << libFileName << endl;

  HMODULE module = LoadLibraryA(libFileName);

  if (module != wgl_wrapper::getGLModule())
  {
    il2ge::core_wrapper::fatalError("DirectX mode is not supported.");
  }

  return module;
}


void installIATPatches(HMODULE module)
{
  patchIAT("LoadLibraryA", "kernel32.dll", (void*) &wrap_LoadLibraryA, nullptr, module);
  patchIAT("GetProcAddress", "kernel32.dll", (void*) &wrap_GetProcAddress, nullptr, module);
#if USE_PLOG
  patchIAT("_write", "msvcrt.dll", (void*) &wrap_write, nullptr, module);
#endif
}


HMODULE loadCoreWrapper(const char *core_library_filename)
{
  assert(!g_core_wrapper_loaded);

  {
    MutexLocker lock(g_fatal_error_mutex);
    jsize num = 0;
    g_get_created_java_vms(&g_java_vm, 1, &num);
    assert(num == 1);
    assert(g_java_vm);
  }

  HMODULE core_module = LoadLibraryA(core_library_filename);
  if (!core_module)
  {
    LOG_ERROR << "Loading " << core_library_filename << " failed with error " << GetLastError() << endl;
    LOG_FLUSH;
    abort();
  }

  il2ge::exception_handler::blacklistModule(core_module);

  installIATPatches(core_module);
  jni_wrapper::resolveImports((void*)core_module);

  il2ge::core_wrapper::init(core_module);

  HMODULE jgl_module = GetModuleHandle("jgl.dll");
  assert(jgl_module);

  patchIAT("LoadLibraryA", "kernel32.dll",
           (void*) &wrap_JGL_LoadLibrary, nullptr, jgl_module);
  patchIAT("GetProcAddress", "kernel32.dll",
           (void*) &wrap_JGL_GetProcAddress, nullptr, jgl_module);

  g_core_wrapper_loaded = true;

  return core_module;
}


void fatalErrorHandler(const char *msg)
{
  JNIEnv *env = nullptr;

  {
    MutexLocker lock(g_fatal_error_mutex);
    g_fatal_error = true;

    if (g_java_vm)
      g_java_vm->GetEnv((void**)&env, JNI_VERSION);
  }

  if (env)
    env->FatalError(msg);
}


void atexitHandler()
{
  LOG_FLUSH;
}


} // namespace


void il2ge::core_wrapper::fatalError(const std::string &message)
{
  LOG_ERROR << "ERROR: " << message << endl;
  LOG_FLUSH;
  _Exit(EXIT_FAILURE);
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


JNIEnv_ *il2ge::core_wrapper::getJNIEnv()
{
  JNIEnv_ *env = nullptr;
  if (g_java_vm)
    g_java_vm->GetEnv((void**)&env, JNI_VERSION);
  return env;
}


bool il2ge::core_wrapper::isMainThread()
{
  assert(g_main_thread);
  return GetCurrentThreadId() == g_main_thread;
}


extern "C"
{


void WINAPI il2geInit()
{
  InitializeCriticalSection(&g_fatal_error_mutex);

  std::atexit(atexitHandler);

  initLog();

  LOG_SEPARATOR;
  LOG_INFO << "*** il2ge.dll initialization ***" << endl;
  LOG_INFO << "Build: " << il2ge::version::getBuildJobID() << endl;
  LOG_INFO << "Debug: " << il2ge::version::isDebugBuild() << endl;
  LOG_INFO << "Commit: " << il2ge::version::getCommitSHA() << endl;
  LOG_FLUSH;

  il2ge::core_wrapper::readConfig();
  il2ge::core_wrapper::writeConfig();

  if (!il2ge::core_wrapper::getConfig().enable_graphics_extender)
  {
    LOG_WARNING << "IL2GE is disabled in config." << endl;
    LOG_FLUSH;
    return;
  }

  g_main_thread = GetCurrentThreadId();

  il2ge::exception_handler::install("il2ge_crash.log", &fatalErrorHandler);
  il2ge::exception_handler::watchModule(g_core_wrapper_module);

  auto jvm_module = GetModuleHandle("jvm.dll");
  assert(jvm_module);

  g_get_created_java_vms = (decltype(JNI_GetCreatedJavaVMs)*)
      GetProcAddress(jvm_module, "JNI_GetCreatedJavaVMs");
  assert(g_get_created_java_vms);

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
