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

#include "scene.h"
#include "core_p.h"
#include <sfs.h>
#include <render_util/terrain.h>
#include <render_util/texture_util.h>
#include <render_util/render_util.h>
#include <render_util/water.h>
#include <render_util/texunits.h>

#include <string>
#include <memory>
#include <GL/gl.h>

#include <gl_wrapper/gl_functions.h>

using namespace gl_wrapper::gl_functions;
using namespace render_util;
using namespace std;

namespace core
{

  Scene::Scene() : Module("Scene")
  {
    curvature_map = render_util::createCurvatureTexture(texture_manager, IL2GE_CACHE_DIR);
    atmosphere_map = render_util::createAmosphereThicknessTexture(texture_manager, IL2GE_CACHE_DIR);

    GLenum active_unit_save;
    gl::GetIntegerv(GL_ACTIVE_TEXTURE, reinterpret_cast<GLint*>(&active_unit_save));

    int atmosphere_map_unit = texture_manager.getHighestUnit() + 1;
    assert(atmosphere_map_unit < MAX_GL_TEXUNITS);

    gl::ActiveTexture(GL_TEXTURE0 + atmosphere_map_unit); //FIXME

    gl::BindTexture(atmosphere_map->getTarget(), atmosphere_map->getID());

    gl::ActiveTexture(active_unit_save);

    CHECK_GL_ERROR();
  }


  void Scene::unloadMap()
  {
    map.reset();
    gl::Finish();
    sfs::clearRedirections();
  }

  void Scene::loadMap(const char *path)
  {
    printf("load map: %s\n", path);

    unloadMap();

    map = make_unique<Map>(path);
  }


  void Scene::update()
  {
    if (map)
      map->getWaterAnimation()->update();
  }


  void Scene::updateUniforms(render_util::ShaderProgramPtr program)
  {
    assert(map);
    map->setUniforms(program);
  }


  render_util::TerrainRenderer &Scene::getTerrainRenderer()
  {
    assert(map);
    return map->getTerrainRenderer();
  }

}
