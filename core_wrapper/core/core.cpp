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
#include <core.h>
#include <misc.h>
#include <render_util/render_util.h>

#include <functional>
#include <iostream>
#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>

#include <gl_wrapper/gl_functions.h>

using namespace std;

namespace
{


void refreshFile(const char *path,
                 std::function<bool(const char*)> generate_func)
{
  string core_wrapper_file = getCoreWrapperFilePath();
  cout<<"core wrapper library: "<<core_wrapper_file<<endl;

  struct stat stat_res;

  if (stat(core_wrapper_file.c_str(), &stat_res) != 0)
  {
    assert(0);
  }

  auto program_mtime = stat_res.st_mtime;

  if (stat(path, &stat_res) == 0)
  {
    if (!(program_mtime > stat_res.st_mtime))
    {
      cout << path << " is up to date." << endl;
      return;
    }
  }

  cout << "generating " << path << " ..." << endl;

  bool success = generate_func(path);
  assert(success);
}


} // namespace


namespace core
{


void init()
{
#ifndef NO_REFRESH_MAPS
  refreshFile(IL2GE_DATA_DIR "/atmosphere_map", render_util::createAtmosphereMap);
  refreshFile(IL2GE_DATA_DIR "/curvature_map", render_util::createCurvatureMap);
#endif
}


Scene *getScene()
{
  Module *current_context = getGLContext();

  assert(current_context);

  Scene *scene = current_context->getSubModule<Scene>();

  if (!scene)
  {
    scene = new Scene;
    current_context->setSubModule(scene);
  }

  return scene;
}


void unloadMap()
{
  printf("unloadMap()\n");
  getScene()->unloadMap();
}


void loadMap(const char *path)
{
  getScene()->loadMap(path);
}


void updateTerrain()
{
  getScene()->updateTerrain(getCameraPos());
}


void setTerrainDrawDistance(float distance)
{
  getScene()->setTerrainDrawDistance(distance);
}


void drawTerrain(render_util::ShaderProgramPtr program)
{
  getScene()->drawTerrain(program);
}


void updateUniforms(render_util::ShaderProgramPtr program)
{
  program->setUniform("cameraPosWorld", core::getCameraPos());
  program->setUniform("projectionMatrixFar", core::getProjectionMatrixFar());
  program->setUniform("world2ViewMatrix", core::getWorld2ViewMatrix());
  program->setUniform("view2WorldMatrix", core::getView2WorldMatrix());
  program->setUniform("terrainColor", glm::vec3(0,1,0));
  program->setUniform("sunDir", core::getSunDir());

  //FIXME
//   program->setUniform("shore_wave_scroll", core::getShoreWavePos());

  getScene()->updateUniforms(program);

  CHECK_GL_ERROR();
}


render_util::TextureManager &textureManager()
{
  return getScene()->texture_manager;
}


} // namespace core
