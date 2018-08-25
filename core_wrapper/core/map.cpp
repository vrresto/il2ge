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
#include <render_util/terrain_cdlod.h>
#include <render_util/terrain.h>
#include <render_util/map_textures.h>
#include <render_util/image_loader.h>
#include <render_util/water.h>
#include <il2ge/map_loader.h>

#include <GL/gl.h>
#include <GL/glext.h>

#include <gl_wrapper/gl_functions.h>

using namespace std;
using namespace render_util;
using namespace glm;
using namespace gl_wrapper::gl_functions;


namespace core
{


typedef render_util::TerrainCDLOD Terrain;
//   typedef render_util::Terrain Terrain;


struct Map::Private
{
  glm::vec2 size;
  glm::ivec2 type_map_size;
  shared_ptr<render_util::TerrainBase> terrain;
  shared_ptr<render_util::MapTextures> textures;
  shared_ptr<render_util::WaterAnimation> water_animation;
};


Map::Map(const char *path) : p(new Private)
{
  p->textures = make_shared<render_util::MapTextures>(core::textureManager());

  p->water_animation = make_shared<render_util::WaterAnimation>();

  p->terrain = make_shared<core::Terrain>();
  p->terrain->setTextureManager(&core::textureManager());

  RessourceLoader res_loader(path);

  il2ge::loadMap(&res_loader,
                 p->textures.get(),
                 p->terrain.get(),
                 p->water_animation.get(),
                 p->size,
                 p->type_map_size);

  p->textures->bind();
}

Map::~Map()
{
  delete p;
}

render_util::TerrainBase *Map::getTerrain()
{
  return p->terrain.get();
}

render_util::WaterAnimation *Map::getWaterAnimation()
{
  return p->water_animation.get();
}

//
glm::vec2 Map::getSize()
{
  return p->size;
}
//
// glm::ivec2 Map::getTypeMapSize()
// {
//   return p->type_map_size;
// }

void Map::setUniforms(render_util::ShaderProgramPtr program)
{
  program->setUniform("map_size", getSize());
  program->setUniform("terrain_height_offset", 0);
  p->textures->setUniforms(program);
  p->water_animation->updateUniforms(program);
}
//
// glm::vec2 getMapSize()
// {
//   return currentMap()->getSize();
// }
//
// glm::ivec2 getTypeMapSize()
// {
//   return currentMap()->getTypeMapSize();
// }





} // namespace core
