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

#include "core_p.h"
#include <core.h>
#include <core/scene.h>
#include <wgl_wrapper.h>
#include <misc.h>
#include <il2ge/map_loader.h>

#include <atmosphere_map.h>
#include <curvature_map.h>
#include <render_util/render_util.h>

#include <INIReader.h>
#include <functional>
#include <iostream>
#include <cassert>
#include <sys/types.h>
#include <sys/stat.h>

#include <gl_wrapper/gl_functions.h>

using namespace std;

namespace
{


bool g_dump_enabled = false;
bool g_base_map_enabled = false;


void refreshFile(const char *path,
                 size_t size,
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
      if (stat_res.st_size != size)
      {
        cout << path << " size mismatch." << endl;
      }
      else
      {
        cout << path << " is up to date." << endl;
        return;
      }
    }
  }

  cout << "generating " << path << " ..." << endl;

  bool success = generate_func(path);
  assert(success);
}


} // namespace


namespace il2ge::map_loader
{
  bool isDumpEnabled() { return g_dump_enabled; }
}


namespace core
{


void init()
{
  INIReader ini("il2ge.ini");
  if (!ini.ParseError())
  {
    g_dump_enabled = ini.GetBoolean("", "EnableDump", g_dump_enabled);
    g_base_map_enabled = ini.GetBoolean("", "EnableBaseMap", g_base_map_enabled);
  }

  cout << "IL2GE: enable dump: " << g_dump_enabled << endl;

#ifndef NO_REFRESH_MAPS
  auto res = util::mkdir(IL2GE_CACHE_DIR);
  assert(res);
  refreshFile(IL2GE_CACHE_DIR "/atmosphere_map", atmosphere_map_size_bytes, render_util::createAtmosphereMap);
  refreshFile(IL2GE_CACHE_DIR "/curvature_map", curvature_map_size_bytes, render_util::createCurvatureMap);
#endif
}


bool isBaseMapEnabled()
{
  return g_base_map_enabled;
}


Scene *getScene()
{
  auto current_context = wgl_wrapper::getContext();

  assert(current_context);

  return current_context->getScene();
}


void unloadMap()
{
  FORCE_CHECK_GL_ERROR();
  getScene()->unloadMap();
  FORCE_CHECK_GL_ERROR();
}


void loadMap(const char *path)
{
  FORCE_CHECK_GL_ERROR();
  getScene()->loadMap(path);
  FORCE_CHECK_GL_ERROR();
}


void updateUniforms(render_util::ShaderProgramPtr program)
{
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


render_util::TerrainRenderer &getTerrainRenderer()
{
  return getScene()->getTerrainRenderer();
}


// render_util::TerrainRenderer &getTerrainRendererLOD()
// {
//   return getScene()->getTerrainRendererLOD();
// }


} // namespace core
