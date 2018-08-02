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

#include "gl_objects.h"
#include "core_p.h"
#include <render_util/terrain.h>
#include <render_util/texture_util.h>

#include <string>
#include <GL/gl.h>

#include <gl_wrapper/gl_functions.h>

using namespace gl_wrapper::gl_functions;

namespace
{
  const std::string resource_path = "ge/";
}

namespace core
{

  GLObjects::GLObjects()
  {
    curvature_map = render_util::createCurvatureTexture(texture_manager, resource_path);
    atmosphere_map = render_util::createAmosphereThicknessTexture(texture_manager, resource_path);

    GLenum active_unit_save;
    gl::GetIntegerv(GL_ACTIVE_TEXTURE, reinterpret_cast<GLint*>(&active_unit_save));

    gl::ActiveTexture(GL_TEXTURE0 + 20); //FIXME

    gl::BindTexture(atmosphere_map->getTarget(), atmosphere_map->getID());

    gl::ActiveTexture(active_unit_save);

    CHECK_GL_ERROR();
  }


  GLObjects *glObjects()
  {
    Module *current_context = getGLContext();

    assert(current_context);

    GLObjects *gl_objects = current_context->getSubModule<GLObjects>();

    if (!gl_objects)
    {
      gl_objects = new GLObjects;
      current_context->setSubModule(gl_objects);
    }

    return gl_objects;
  }


}
