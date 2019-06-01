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

#include <misc.h>
#include <wgl_wrapper.h>
#include <core.h>
#include <render_util/state.h>

#include "gl_wrapper_private.h"

#include <array>
#include <cassert>
#include <GL/gl.h>

#include <render_util/gl_binding/gl_functions.h>

using namespace render_util::gl_binding;
using namespace core_gl_wrapper::texture_state;
using namespace std;

namespace
{
  TextureState *getState()
  {
    return core_gl_wrapper::getContext()->getTextureState();
  }

  void GLAPIENTRY wrap_glBindTexture(GLenum target, GLuint texture)
  {
    if (wgl_wrapper::isMainContextCurrent())
    {
      auto state = getState();

      assert(!state->is_frozen);

      Unit &u = state->units[state->active_unit];
      u.bindings[target] = texture;
    }

    gl::BindTexture(target, texture);
  }

  void GLAPIENTRY wrap_glActiveTexture(GLenum texture)
  {
    assert(wgl_wrapper::isMainContextCurrent());

    auto state = getState();

    assert(!state->is_frozen);

    int unit = texture - GL_TEXTURE0;
    assert(unit >= 0);

    assert(texture < MAX_UNITS);
    state->active_unit = texture;
    gl::ActiveTexture(texture);
  }

}


#define SET_PROC(name) core_gl_wrapper::setProc(#name, (void*) &wrap_##name)

namespace core_gl_wrapper::texture_state
{
  void init()
  {
    SET_PROC(glBindTexture);
    SET_PROC(glActiveTexture);
  }

  void freeze()
  {
    auto state = getState();
    assert(!state->is_frozen);
    state->is_frozen = true;
  }

  void restore()
  {
    auto state = getState();
    assert(state->is_frozen);

    for (size_t i = core::textureManager().getLowestUnit();
         i < state->units.size() && i <= core::textureManager().getHighestUnit();
         i++)
    {
      gl::ActiveTexture(GL_TEXTURE0 + i);

      Unit &unit = state->units[i];
      for (auto &it : unit.bindings)
      {
        gl::BindTexture(it.first, it.second);
      }
    }

    gl::ActiveTexture(GL_TEXTURE0 + state->active_unit);

    state->is_frozen = false;
  }
}
