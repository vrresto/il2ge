/**
 *    IL-2 Graphics Extender
 *    Copyright (C) 2020 Jan Lepper
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

#include "gl_version_check.h"
#include <render_util/gl_binding/gl_functions.h>
#include <log.h>

using namespace render_util::gl_binding;


namespace
{
  constexpr auto MIN_MAJOR = 4;
  constexpr auto MIN_MINOR = 5;
}


namespace il2ge
{


void checkGLVersion()
{
  int major = 0;
  int minor = 0;
  gl::GetIntegerv(GL_MAJOR_VERSION, &major);
  gl::GetIntegerv(GL_MINOR_VERSION, &minor);

  if (major < MIN_MAJOR || minor < MIN_MINOR)
  {
    LOG_ERROR << "Error: OpenGL version isn't recent enough." << std::endl;
    LOG_ERROR << "Required: " << MIN_MAJOR << "." << MIN_MINOR << std::endl;
    LOG_ERROR << "Got: " << major << "." << minor << std::endl;
    LOG_FLUSH;
    _Exit(EXIT_FAILURE);
  }
}


}
