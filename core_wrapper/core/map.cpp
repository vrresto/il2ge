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

#include "map.h"
#include "ressource_loader.h"
#include "core_p.h"

#include <sfs.h>
#include <core.h>
#include <render_util/terrain_util.h>
#include <render_util/map_textures.h>
#include <render_util/image_loader.h>
#include <render_util/water.h>
#include <il2ge/map_loader.h>

#include <GL/gl.h>
#include <GL/glext.h>

#include <gl_wrapper/gl_functions.h>

using namespace std;
using namespace render_util;
using namespace il2ge;
using namespace glm;
using namespace gl_wrapper::gl_functions;


namespace
{
  const bool g_terrain_use_lod = true;
  const std::string g_shader_path = IL2GE_DATA_DIR "/shaders";
}


namespace core
{


struct Map::Private
{
  glm::vec2 size;
  shared_ptr<render_util::MapTextures> textures;
  shared_ptr<render_util::WaterAnimation> water_animation;
  TerrainRenderer terrain_renderer;
};


Map::Map(const char *path) : p(new Private)
{
  p->textures = make_shared<render_util::MapTextures>(core::textureManager());

  p->water_animation = make_shared<render_util::WaterAnimation>();

  string terrain_program_name;

  p->terrain_renderer = createTerrainRenderer(textureManager(),
                                              g_terrain_use_lod,
                                              g_shader_path,
                                              terrain_program_name,
                                              core::isBaseMapEnabled());

  p->terrain_renderer.getProgram()->setUniform("terrain_color", glm::vec3(1,0,0));

  RessourceLoader res_loader(path);

  render_util::ElevationMap::Ptr elevation_map_base;
  if (core::isBaseMapEnabled())
    elevation_map_base = map_generator::generateHeightMap();

  auto elevation_map = map_loader::createElevationMap(&res_loader);

  map_loader::createMapTextures(&res_loader,
                                p->textures.get(),
                                p->water_animation.get(),
                                elevation_map_base);

  p->size = glm::vec2(elevation_map->getSize() * (int)il2ge::HEIGHT_MAP_METERS_PER_PIXEL);

  if (elevation_map_base)
    p->terrain_renderer.getTerrain()->build(elevation_map, elevation_map_base);
  else
    p->terrain_renderer.getTerrain()->build(elevation_map);

  p->textures->bind();
}

Map::~Map()
{
  delete p;
}


render_util::WaterAnimation *Map::getWaterAnimation()
{
  return p->water_animation.get();
}


glm::vec2 Map::getSize()
{
  return p->size;
}

void Map::setUniforms(render_util::ShaderProgramPtr program)
{
  program->setUniform("map_size", getSize());
  program->setUniform("terrain_height_offset", 0);
  p->textures->setUniforms(program);
  p->water_animation->updateUniforms(program);
}


render_util::TerrainRenderer &Map::getTerrainRenderer()
{
  return p->terrain_renderer;
}




} // namespace core
