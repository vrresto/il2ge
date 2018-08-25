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

#include "wgl_wrapper.h"
#include <misc.h>
#include <gl_wrapper.h>

#include <gl_wrapper/gl_interface.h>

#include <cassert>
#include <cstdio>
#include <iostream>
#include <windows.h>

using namespace std;
using namespace wgl_wrapper;


namespace
{


typedef PROC WINAPI wglGetProcAddress_t(LPCSTR);
typedef BOOL WINAPI wglMakeCurrent_t(HDC, HGLRC);
typedef BOOL WINAPI wglDeleteContext_t(HGLRC);

wglMakeCurrent_t *real_wglMakeCurrent = nullptr;
wglDeleteContext_t *real_wglDeleteContext = nullptr;
wglGetProcAddress_t *real_wglGetProcAddress = nullptr;


struct ContextData : public Module
{
  ContextData() : Module("ContextData") {}

  std::shared_ptr<gl_wrapper::GL_Interface> iface;
};


struct GlobalData
{
  std::unordered_map<HGLRC, ContextData*> m_data_for_context;
  std::unordered_map<DWORD, ContextData*> m_current_context_for_thread;
  ContextData *m_main_context = nullptr;
  DWORD m_main_thread = 0;
  bool m_in_shutdown = false;
  HANDLE m_mutex = 0;
};


#if 0
class Lock
{
  HANDLE m_mutex;

public:
  Lock(HANDLE mutex) : m_mutex(mutex)
  {
    SetLastError(ERROR_SUCCESS);
    auto wait_res = WAIT_FAILED;

    while (wait_res != WAIT_OBJECT_0)
    {
      wait_res = WaitForSingleObject(m_mutex, INFINITE);
      if (wait_res != WAIT_OBJECT_0)
        printf("WaitForSingleObject() failed - wait_res: 0x%x - error: 0x%x\n", wait_res, GetLastError());
    }
  }
  ~Lock()
  {
    auto release_res = ReleaseMutex(m_mutex);
    assert(release_res);
  }
};
#else
class Lock
{
public:
  Lock(HANDLE mutex) {}
};
#endif


GlobalData g_data;


void setCurrentContext(ContextData *c)
{
  gl_wrapper::GL_Interface *iface = c ? c->iface.get() : nullptr;
  g_data.m_current_context_for_thread[GetCurrentThreadId()] = c;
  if (!g_data.m_main_context)
    g_data.m_main_context = c;
  gl_wrapper::GL_Interface::setCurrent(iface);
}


BOOL WINAPI wrap_wglMakeContextCurrentARB(HDC hDrawDC, HDC hReadDC, HGLRC hglrc)
{
  printf("wglMakeContextCurrentARB\n");
  _Exit(0);
}


void *getProcAddress_ext(const char *name)
{
  void *func = (void*) GetProcAddress(GetModuleHandle("opengl32.dll"), name);
  if (!func)
    func = (void*) real_wglGetProcAddress(name);

  return func;
}


PROC WINAPI wrap_wglGetProcAddress(LPCSTR name)
{
  cout<<"wrap_wglGetProcAddress: "<<name<<endl;

  if (isMainContextCurrent())
  {
    Lock lock(g_data.m_mutex);

    if (strcmp(name, "wglMakeContextCurrentARB") == 0)
    {
      return (PROC) &wrap_wglMakeContextCurrentARB;
    }

    PROC proc = real_wglGetProcAddress(name);

    void *wrap_proc = core_gl_wrapper::getProc(name);
    if (wrap_proc && proc)
      return (PROC) wrap_proc;

    return proc;
  }
  else
  {
    return real_wglGetProcAddress(name);
  }
}


BOOL WINAPI wrap_wglMakeCurrent(HDC hdc, HGLRC hglrc)
{
  Lock lock(g_data.m_mutex);

  assert(!g_data.m_in_shutdown);
  assert(GetCurrentThreadId() == g_data.m_main_thread);
  assert(g_data.m_current_context_for_thread.size() <= 1);

  if (!hglrc)
  {
    // assumtion: this is called by the game prior to deleting the main context (and only then)
    // so we use this last chance to free our ressources while the the context is still current

    g_data.m_in_shutdown = true;

    auto current_context = g_data.m_current_context_for_thread[GetCurrentThreadId()];

    assert(g_data.m_main_context);
    assert(current_context);
    assert(current_context == g_data.m_main_context);
    assert(current_context->hasSubmodules());

    current_context->clearSubmodules();

    setCurrentContext(nullptr);

    bool res = real_wglMakeCurrent(hdc, hglrc);
    assert(res);

    return true;
  }

  if (real_wglMakeCurrent(hdc, hglrc))
  {
    auto d = g_data.m_data_for_context[hglrc];

    if (!d)
    {
      d = new ContextData;

      d->iface = std::make_shared<gl_wrapper::GL_Interface>(&getProcAddress_ext);

      g_data.m_data_for_context[hglrc] = d;
    }

    setCurrentContext(d);

    return true;
  }
  else
    return false;
}


BOOL WINAPI wrap_wglDeleteContext(HGLRC hglrc)
{
  Lock lock(g_data.m_mutex);

  assert(GetCurrentThreadId() == g_data.m_main_thread);

  ContextData *c = g_data.m_data_for_context[hglrc];

  g_data.m_data_for_context.erase(hglrc);

  auto current_context = g_data.m_current_context_for_thread[GetCurrentThreadId()];

  if (c)
  {
    c->printSubmodules();
    assert(!c->hasSubmodules());

    if (c == g_data.m_main_context)
    {
      assert(g_data.m_in_shutdown);
      g_data.m_main_context = nullptr;
    }

    if (c == current_context)
    {
      setCurrentContext(nullptr);
    }
  }

  delete c;
  c = nullptr;

  bool res =  real_wglDeleteContext(hglrc);
  assert(res);

  return res;
}


} // namespace


namespace wgl_wrapper
{


void init()
{
  g_data.m_mutex = CreateMutexA(
      NULL,              // default security attributes
      FALSE,             // initially not owned
      NULL);             // unnamed mutex

  assert(g_data.m_mutex);
}


bool isMainContextCurrent()
{
  Lock lock(g_data.m_mutex);

  auto current_thread = GetCurrentThreadId();

  ContextData *c = g_data.m_current_context_for_thread[current_thread];

  if (c && c == g_data.m_main_context)
  {
    assert(current_thread == g_data.m_main_thread);
    return true;
  }
  else
  {
    return false;
  }
}


bool isMainThread()
{
  Lock lock(g_data.m_mutex);
  return GetCurrentThreadId() == g_data.m_main_thread;
}


void *getProcAddress(HMODULE module, LPCSTR name)
{
  {
    Lock lock(g_data.m_mutex);
    if (!g_data.m_main_thread)
      g_data.m_main_thread = GetCurrentThreadId();

    assert(GetCurrentThreadId() == g_data.m_main_thread);
  }

  void *proc = (void*) GetProcAddress(module, name);

  if (strcmp(name, "wglGetProcAddress") == 0)
  {
    real_wglGetProcAddress = (wglGetProcAddress_t*) proc;
    return (void*) &wrap_wglGetProcAddress;
  }

  if (strcmp(name, "wglMakeCurrent") == 0)
  {
    real_wglMakeCurrent = (wglMakeCurrent_t*) proc;
    return (void*) &wrap_wglMakeCurrent;
  }

  if (strcmp(name, "wglDeleteContext") == 0)
  {
    real_wglDeleteContext = (wglDeleteContext_t*) proc;
    return (void*) &wrap_wglDeleteContext;
  }


  assert(!isMainContextCurrent());

  void *wrap_proc = core_gl_wrapper::getProc(name);
  if (wrap_proc && proc)
    return wrap_proc;

  return proc;
}


Module *getContext()
{
  Lock lock(g_data.m_mutex);

  assert(GetCurrentThreadId() == g_data.m_main_thread);

  assert(!g_data.m_in_shutdown);
  assert(g_data.m_main_context);

  auto current_context = g_data.m_current_context_for_thread[GetCurrentThreadId()];

  assert(current_context);
  assert(current_context == g_data.m_main_context);

  return current_context;
}


} // namespace wgl_wrapper
