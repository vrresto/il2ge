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

#include <iostream>
#include <sstream>
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


typedef void CrashHandlerFunc(PEXCEPTION_POINTERS pExceptionInfo);
typedef void DumpStackFunc(const CONTEXT*);


const char* const crash_handler_library_name =
    IL2GE_DATA_DIR "/mingw_crash_handler.dll";

const char* const crash_handler_func_name = "crashHandler";
const char* const dump_stack_func_name = "dumpStack";


static LONG g_handler_entered = 0;


HMODULE loadCrashHandlerLibrary()
{
  HMODULE crash_handler_module = LoadLibraryA(crash_handler_library_name);

  if (!crash_handler_module)
  {
    fprintf(stderr, "\n**** could not load %s - backtrace disabled ****\n\n",
            crash_handler_library_name);

    stringstream message;
    message<<"Could not load "<<crash_handler_library_name<<" - backtrace disabled.";
    message<<endl<<endl;
    message<<"To get a useful backtrace please download https://github.com/jrfonseca/drmingw/releases/download/0.8.2/drmingw-0.8.2-win32.7z and copy the file bin/exchndl.dll to your IL-2 directory.";

    MessageBoxA(0, message.str().c_str(), nullptr, 0);
  }

  return crash_handler_module;
}


DWORD WINAPI backtraceThreadMain(LPVOID lpParameter)
{
  Sleep(1000); // workaround race condition

  HANDLE thread = lpParameter;

  cout<<"target thread: "<<thread<<endl;

  SuspendThread(thread);

  printf("target thread suspended.\n");

  CONTEXT context;
  memset(&context, 0, sizeof(context));
  context.ContextFlags = CONTEXT_FULL;

  if (GetThreadContext(thread, &context))
  {
    HMODULE crash_handler_module = loadCrashHandlerLibrary();

    if (crash_handler_module)
    {
      DumpStackFunc *dump_stack_func = (DumpStackFunc*)
          GetProcAddress(crash_handler_module, dump_stack_func_name);
      assert(dump_stack_func);
      dump_stack_func(&context);
    }
  }
  else
  {
    fprintf(stderr, "ERROR: GetThreadContext() failed.\n");
  }

  TerminateProcess(GetCurrentProcess(), EXIT_FAILURE);
  SuspendThread(GetCurrentThread());
  _Exit(EXIT_FAILURE);
}


void printBacktracePrivate()
{
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

  cout<<"current thread: "<<currend_thread<<endl;

  printf("waiting for backtrace thread to finish ...\n");
  fflush(stdout);
  fflush(stderr);
  SetLastError(0);

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
    return;
  }
  WaitForSingleObject(thread, INFINITE);

  fprintf(stderr, "backtrace thread returned\n");

  CloseHandle(currend_thread);
}


void abortHandler(int sig)
{
  if (InterlockedIncrement(&g_handler_entered) != 1)
  {
    _Exit(1);
  }

  fprintf(stderr, "caught SIGABRT\n");
  fflush(stderr);
  printBacktrace();

  _Exit(1);

  InterlockedDecrement(&g_handler_entered);
}


LONG WINAPI vectoredExceptionHandler(_EXCEPTION_POINTERS *info)
{
  if (info->ExceptionRecord->ExceptionCode != STATUS_CPP_EH_EXCEPTION)
  {
    if (InterlockedIncrement(&g_handler_entered) != 1)
    {
      _Exit(1);
    }

    fprintf(stderr, "\nException code: %x  Flags: %x\n",
            info->ExceptionRecord->ExceptionCode,
            info->ExceptionRecord->ExceptionFlags);

    HMODULE crash_handler_module = loadCrashHandlerLibrary();

    if (crash_handler_module)
    {
      CrashHandlerFunc *crash_handler = (CrashHandlerFunc*)
          GetProcAddress(crash_handler_module, crash_handler_func_name);
      assert(crash_handler);
      crash_handler(info);
    }

    InterlockedDecrement(&g_handler_entered);
  }

  return EXCEPTION_CONTINUE_SEARCH;
}


void terminateHandler()
{
  static int tried_throw = 0;
  stringstream message;

  try
  {
    // try once to re-throw currently active exception
    tried_throw++;
    if (tried_throw == 1)
      throw;
  }
  catch (const std::exception &e)
  {
    message << __FUNCTION__ << " caught unhandled exception. what(): "
              << e.what() << std::endl;
  }
  catch (...)
  {
    message << __FUNCTION__ << " caught unknown/unhandled exception."
              << std::endl;
  }

  string message_str = message.str();
  if (!message_str.empty())
  {
    cerr << message_str;
    MessageBoxA(0, message_str.c_str(), nullptr, MB_TASKMODAL | MB_SETFOREGROUND | MB_TOPMOST);
  }
  abort();
}


} // namespace


void printBacktrace()
{
  printBacktracePrivate();
}

void installExceptionHandler()
{
  AddVectoredExceptionHandler(true, &vectoredExceptionHandler);
  signal(SIGABRT, abortHandler);
  std::set_terminate(&terminateHandler);
}
