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

#include <il2ge/exception_handler.h>
#include <log.h>

#include <mingw_crash_handler.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <cstdio>
#include <csignal>
#include <cassert>
#include <windows.h>
#include <ntstatus.h>


#ifndef STATUS_CPP_EH_EXCEPTION
#define STATUS_CPP_EH_EXCEPTION 0xE06D7363
#endif


using namespace std;


namespace
{

[[ noreturn ]] void die();

const char* const crash_handler_library_name =
  IL2GE_LIB_DIR "/mingw_crash_handler.dll";

const char* const drmingw_download_url =
  "https://github.com/jrfonseca/drmingw/releases/download/0.8.2/drmingw-0.8.2-win32.7z";


class Lock
{
  HANDLE m_mutex;

public:
  Lock(HANDLE mutex) : m_mutex(mutex)
  {
    SetLastError(ERROR_SUCCESS);
    auto wait_res = WAIT_FAILED;
    int tries = 0;

    while (wait_res != WAIT_OBJECT_0)
    {
      wait_res = WaitForSingleObject(m_mutex, INFINITE);
      if (wait_res != WAIT_OBJECT_0)
      {
        tries++;

        fprintf(stderr, "WaitForSingleObject() failed - wait_res: 0x%x - error: 0x%x\n",
                wait_res,
                GetLastError());

        if (tries > 5)
        {
          die();
        }

        Sleep(200);
      }
    }
  }
  ~Lock()
  {
    auto release_res = ReleaseMutex(m_mutex);
    assert(release_res);
  }
};


LONG g_handler_entered = 0;
MingwCrashHandlerInterface *g_crash_handler = nullptr;
HANDLE g_crash_handler_mutex = 0;
HANDLE g_target_thread_mutex = 0;
unordered_set<HMODULE> g_watched_modules;
std::string g_log_file_name;
std::function<void(const char*)> g_fatal_error_handler;


void clearLog()
{
  ofstream out(g_log_file_name);
  out << endl;
}


void printLog()
{
  ifstream in(g_log_file_name);
  string line;

  while (getline(in, line))
  {
#if USE_PLOG
    auto &logger = *plog::get<PLOG_DEFAULT_INSTANCE>();
    plog::Record record(plog::error);
    record << line << endl;
    logger += record;
#else
    LOG_ERROR << line << endl;
#endif
  }
}


void fatalError(const char *msg)
{
  if (g_fatal_error_handler)
    g_fatal_error_handler(msg);
}


[[ noreturn ]] void die()
{
  TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);

  while (true)
  {
    Sleep(1000);
  }
}


bool isModuleWatched(HMODULE module)
{
  return true;
}


bool loadCrashHandlerLibrary()
{
  Lock lock(g_crash_handler_mutex);

  static HMODULE g_crash_handler_module = 0;

  static bool g_failed = false;
  if (g_failed)
    return false;

  if (g_crash_handler_module)
    return true;

  auto old_error_mode = GetThreadErrorMode();
  SetThreadErrorMode(SEM_FAILCRITICALERRORS, nullptr);
  g_crash_handler_module = LoadLibraryA(crash_handler_library_name);
  SetThreadErrorMode(old_error_mode, nullptr);

  if (!g_crash_handler_module)
  {
    g_failed = true;

    LOG_SEPARATOR;
    LOG_ERROR << "Could not load " << crash_handler_library_name << " - backtrace disabled." << endl;
    LOG_ERROR << "To get a useful backtrace please download "
          << drmingw_download_url
          << " and copy the file bin/mgwhelp.dll to your IL-2 directory." << endl;
    LOG_FLUSH;

    return false;
  }

  g_crash_handler = (MingwCrashHandlerInterface*)
      GetProcAddress(g_crash_handler_module, "mingw_crash_handler_interface");
  assert(g_crash_handler);

  g_crash_handler->setLogFileName(g_log_file_name.c_str());

  return true;
}


DWORD WINAPI backtraceThreadMain(LPVOID lpParameter)
{
  auto wait_res = WaitForSingleObject(g_target_thread_mutex, INFINITE);
  assert(wait_res == WAIT_OBJECT_0);

  HANDLE thread = lpParameter;

  LOG_DEBUG<<"target thread: "<<thread<<endl;

  CONTEXT context;
  memset(&context, 0, sizeof(context));
  context.ContextFlags = CONTEXT_FULL;

  if (GetThreadContext(thread, &context))
  {
    Lock lock(g_crash_handler_mutex);

    clearLog();
    assert(g_crash_handler);
    g_crash_handler->dumpStack(&context);
    printLog();
  }
  else
  {
    fprintf(stderr, "ERROR: GetThreadContext() failed.\n");
  }

  return 0;
}


void printBacktracePrivate()
{
  if (!loadCrashHandlerLibrary())
    return;

  HANDLE currend_thread = 0;
  bool res = DuplicateHandle(GetCurrentProcess(),
                             GetCurrentThread(),
                             GetCurrentProcess(),
                             &currend_thread,
                             THREAD_ALL_ACCESS,
                             false,
                             0);

  if(!res)
  {
    fprintf(stderr, "ERROR: DuplicateHandle() failed.!\n");
    return;
  }

  LOG_DEBUG<<"current thread: "<<currend_thread<<endl;

  printf("waiting for backtrace thread to finish ...\n");
  fflush(stdout);
  fflush(stderr);
  SetLastError(0);

  auto wait_res = WaitForSingleObject(g_target_thread_mutex, INFINITE);
  assert(wait_res == WAIT_OBJECT_0);

  auto thread = CreateThread(nullptr,
                             0,
                             &backtraceThreadMain,
                             currend_thread,
                             0,
                             nullptr);
  if (!thread)
  {
    fprintf(stderr, "ERROR: can't create backtrace thread - error code: 0x%x\n", GetLastError());
    CloseHandle(currend_thread);
    ReleaseMutex(g_target_thread_mutex);
    return;
  }

  SignalObjectAndWait(g_target_thread_mutex, thread, INFINITE, false);

  fprintf(stderr, "backtrace thread returned\n");

  ReleaseMutex(g_target_thread_mutex);
  CloseHandle(currend_thread);
}


LONG WINAPI vectoredExceptionHandler(_EXCEPTION_POINTERS *info)
{
  if (info->ExceptionRecord->ExceptionCode != STATUS_CPP_EH_EXCEPTION)
  {
    if (InterlockedIncrement(&g_handler_entered) != 1)
    {
      die();
    }

    if (loadCrashHandlerLibrary())
    {
      HMODULE module = 0;

      {
        Lock lock(g_crash_handler_mutex);
        module = (HMODULE)
          g_crash_handler->getModuleBase(info->ExceptionRecord->ExceptionAddress);
      }

      if (isModuleWatched(module))
      {
        char module_file_name[MAX_PATH];

        if (GetModuleFileNameA(module, module_file_name, sizeof(module_file_name)))
        {
          LOG_SEPARATOR;
          LOG_ERROR << "Exception code: " << std::hex << info->ExceptionRecord->ExceptionCode << "  Flags: "
                    << info->ExceptionRecord->ExceptionFlags << '\n' << std::dec;
          LOG_ERROR << "Module: " << module_file_name << endl;
          LOG_FLUSH;

          {
            Lock lock(g_crash_handler_mutex);

            clearLog();
            g_crash_handler->crashHandler(info);
            printLog();
          }

          fatalError("Unhandled exception");
          die();
        }
      }
    }

    InterlockedDecrement(&g_handler_entered);
  }

  return EXCEPTION_CONTINUE_SEARCH;
}


void terminateHandler()
{
  static int tried_throw = 0;

  try
  {
    // try once to re-throw currently active exception
    tried_throw++;
    if (tried_throw == 1)
      throw;
  }
  catch (const std::exception &e)
  {
    LOG_SEPARATOR;
    LOG_ERROR << __FUNCTION__ << " caught unhandled exception. what(): " << e.what() << '\n';
  }
  catch (...)
  {
    LOG_SEPARATOR;
    LOG_ERROR << __FUNCTION__ << " caught unknown/unhandled exception.\n";
  }

  LOG_FLUSH;

  abort();
}


} // namespace


static void printBacktrace()
{
  printBacktracePrivate();
}


void il2ge::exception_handler::install(const std::string &log_file_name,
                                       std::function<void(const char*)> fatal_error_handler)
{
  static bool installed = false;
  assert(!installed);
  installed = true;

  g_log_file_name = log_file_name;
  g_fatal_error_handler = fatal_error_handler;

  g_target_thread_mutex = CreateMutexA(nullptr, false, nullptr);
  assert(g_target_thread_mutex);
  g_crash_handler_mutex = CreateMutexA(nullptr, false, nullptr);
  assert(g_crash_handler_mutex);

  AddVectoredExceptionHandler(true, &vectoredExceptionHandler);
  std::set_terminate(&terminateHandler);
}


void il2ge::exception_handler::watchModule(HMODULE module)
{
  g_watched_modules.insert(module);
}


extern "C"
{

#ifdef IL2GE_USE_CUSTOM_ASSERT
void _assert(const char *_Message, const char *_File, unsigned _Line)
{
  LOG_SEPARATOR;
  LOG_ERROR << "Assertion failed: " << _Message << '\n';
  LOG_ERROR << "File: " << _File << ":" << _Line << '\n';
  LOG_FLUSH;

  abort();
}
#endif


void abort()
{
  static long handler_entered = 0;

  if (InterlockedIncrement(&handler_entered) < 3)
  {
    LOG_SEPARATOR;
    LOG_ERROR << "Aborted.\n";
    LOG_FLUSH;
    printBacktrace();
    LOG_FLUSH;

    fatalError("Aborted");
  }

  die();
}


} // extern "C"
