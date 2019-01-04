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

#ifndef IL2GE_CORE_WGL_WRAPPER_H
#define IL2GE_CORE_WGL_WRAPPER_H

#include <misc.h>
#include <gl_wrapper/gl_interface.h>
#include <gl_wrapper.h>
#include <core/scene.h>

#include <windef.h>

namespace wgl_wrapper
{
  struct ContextData
  {
    ContextData(std::shared_ptr<gl_wrapper::GL_Interface> iface);

    gl_wrapper::GL_Interface *getGLInterface() { return m_iface.get(); }
    core::Scene *getScene();
    core_gl_wrapper::Context *getGLWrapperContext();

    void freeResources();

  private:
    std::shared_ptr<gl_wrapper::GL_Interface> m_iface;
    std::shared_ptr<core::Scene> m_scene;
    std::shared_ptr<core_gl_wrapper::Context> m_gl_wrapper_context;
  };

  bool isMainThread();
  bool isMainContextCurrent();
  void *getProcAddress(HMODULE module, LPCSTR name);
  ContextData *getContext();
  core::Scene *getScene();
  HMODULE getGLModule();
  void init();
}

#endif
