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
#include <render_util/gl_binding/gl_interface.h>

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
typedef HGLRC WINAPI wglCreateContext_t(HDC);
typedef BOOL WINAPI wglDeleteContext_t(HGLRC);

wglMakeCurrent_t *real_wglMakeCurrent = nullptr;
wglCreateContext_t *real_wglCreateContext = nullptr;
wglDeleteContext_t *real_wglDeleteContext = nullptr;
wglGetProcAddress_t *real_wglGetProcAddress = nullptr;


struct GlobalData
{
  std::unordered_map<HGLRC, ContextData*> m_data_for_context;
  ContextData *m_current_context;
  ContextData *m_main_context = nullptr;
  bool m_in_shutdown = false;
  HMODULE m_gl_module = 0;
};


GlobalData g_data;


void *getProcAddress_ext(const char *name)
{
  void *func = (void*) GetProcAddress(g_data.m_gl_module, name);
  if (!func)
    func = (void*) real_wglGetProcAddress(name);

  return func;
}


ContextData *getContextData(HGLRC handle)
{
  auto d = g_data.m_data_for_context[handle];

  if (!d)
  {
    d = new ContextData;
    g_data.m_data_for_context[handle] = d;
  }

  return d;
}


void currentContextChanged(ContextData *new_current)
{
  render_util::gl_binding::GL_Interface *iface = nullptr;

  if (new_current)
  {
    if (!new_current->getGLInterface())
    {
      auto iface = std::make_shared<render_util::gl_binding::GL_Interface>(&getProcAddress_ext);
      new_current->setGLInterface(iface);
    }

    iface = new_current->getGLInterface();
  }

  g_data.m_current_context = new_current;
  render_util::gl_binding::GL_Interface::setCurrent(iface);
}


BOOL WINAPI wrap_wglMakeContextCurrentARB(HDC hDrawDC, HDC hReadDC, HGLRC hglrc)
{
  printf("wglMakeContextCurrentARB\n");
  _Exit(0);
}


PROC WINAPI wrap_wglGetProcAddress(LPCSTR name)
{
  cout<<"wrap_wglGetProcAddress: "<<name<<endl;

  if (isMainContextCurrent())
  {
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
  assert(!g_data.m_in_shutdown);

  if (!hglrc)
  {
    // assumtion: this is called by the game prior to deleting the main context (and only then)
    // so we use this last chance to free our ressources while the the context is still current

    g_data.m_in_shutdown = true;

    assert(g_data.m_main_context);
    assert(g_data.m_current_context);
    assert(g_data.m_current_context == g_data.m_main_context);

    g_data.m_current_context->freeResources();

    currentContextChanged(nullptr);

    bool res = real_wglMakeCurrent(hdc, hglrc);
    assert(res);

    return true;
  }

  if (real_wglMakeCurrent(hdc, hglrc))
  {
    auto d = getContextData(hglrc);

    currentContextChanged(d);

    return true;
  }
  else
    return false;
}


HGLRC WINAPI wrap_wglCreateContext(HDC dc)
{
  auto handle = real_wglCreateContext(dc);

  if (!g_data.m_main_context)
  {
    g_data.m_main_context = getContextData(handle);
  }

  return handle;
}


BOOL WINAPI wrap_wglDeleteContext(HGLRC hglrc)
{
  ContextData *c = g_data.m_data_for_context[hglrc];

  g_data.m_data_for_context.erase(hglrc);

  if (c)
  {
    if (c == g_data.m_main_context)
    {
      assert(g_data.m_in_shutdown);
      g_data.m_main_context = nullptr;
    }

    if (c == g_data.m_current_context)
    {
      currentContextChanged(nullptr);
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
  g_data.m_gl_module = LoadLibrary("opengl32.dll");
  assert(g_data.m_gl_module);

  real_wglGetProcAddress = (wglGetProcAddress_t*)
    GetProcAddress(g_data.m_gl_module, "wglGetProcAddress");
  real_wglMakeCurrent = (wglMakeCurrent_t*)
    GetProcAddress(g_data.m_gl_module, "wglMakeCurrent");
  real_wglCreateContext = (wglCreateContext_t*)
    GetProcAddress(g_data.m_gl_module, "wglCreateContext");
  real_wglDeleteContext = (wglDeleteContext_t*)
    GetProcAddress(g_data.m_gl_module, "wglDeleteContext");
}


HMODULE getGLModule()
{
  return g_data.m_gl_module;
}


bool isMainContextCurrent()
{
  return g_data.m_current_context && (g_data.m_current_context == g_data.m_main_context);
}


void *getProcAddress(HMODULE module, LPCSTR name)
{
  void *proc = (void*) GetProcAddress(module, name);

  if (strcmp(name, "wglGetProcAddress") == 0)
  {
    return (void*) &wrap_wglGetProcAddress;
  }

  if (strcmp(name, "wglMakeCurrent") == 0)
  {
    return (void*) &wrap_wglMakeCurrent;
  }

  if (strcmp(name, "wglCreateContext") == 0)
  {
    return (void*) &wrap_wglCreateContext;
  }

  if (strcmp(name, "wglDeleteContext") == 0)
  {
    return (void*) &wrap_wglDeleteContext;
  }

  assert(!isMainContextCurrent());

  void *wrap_proc = core_gl_wrapper::getProc(name);
  if (wrap_proc && proc)
    return wrap_proc;

  return proc;
}


ContextData *getContext()
{
  assert(!g_data.m_in_shutdown);
  assert(isMainContextCurrent());

  return g_data.m_current_context;
}


core::Scene *getScene()
{
  return getContext()->getScene();
}


core::Scene *ContextData::getScene()
{
  if (!m_scene)
    m_scene = std::make_shared<core::Scene>();

  return m_scene.get();
}


void ContextData::freeResources()
{
  m_scene.reset();
  m_gl_wrapper_context.reset();
}


} // namespace wgl_wrapper
