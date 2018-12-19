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

#ifndef CORE_GL_WRAPPER_H
#define CORE_GL_WRAPPER_H

#include <render_util/texunits.h>
#include <render_util/shader.h>
#include <render_util/gl_context.h>

#include <unordered_map>
#include <map>

namespace core_gl_wrapper
{
  class Context
  {
  public:
    struct Impl;

    Context();
    ~Context();

    Impl *getImpl() { return impl.get(); }

  private:
    std::unique_ptr<Impl> impl;
  };

  void init();
  void *getProc(const char *name);
}

#endif
