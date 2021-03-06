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
#include <misc.h>
#include <core.h>
#include <configuration.h>
#include <render_util/terrain_util.h>
#include <render_util/map_textures.h>
#include <render_util/image_loader.h>
#include <render_util/image_util.h>
#include <render_util/texunits.h>
#include <render_util/water.h>
#include <render_util/atmosphere.h>
#include <il2ge/map_loader.h>

#include <GL/gl.h>
#include <GL/glext.h>

#include <render_util/gl_binding/gl_functions.h>

using namespace std;
using namespace render_util;
using namespace il2ge;
using namespace glm;


namespace
{
  const bool g_terrain_use_lod = true;
  const string dump_base_dir = "il2ge_dump/"; //HACK
}


namespace core
{


struct Map::Private : public render_util::MapBase
{
  glm::vec2 size;
  shared_ptr<render_util::MapTextures> textures;
  shared_ptr<render_util::WaterAnimation> water_animation;
  std::shared_ptr<TerrainBase> terrain;
  std::unique_ptr<render_util::CirrusClouds> cirrus_clouds;
  glm::vec2 base_map_origin = glm::vec2(0);
  render_util::TerrainBase::MaterialMap::ConstPtr material_map;
  render_util::ImageGreyScale::ConstPtr pixel_map_h;
  std::shared_ptr<GenericImage> cirrus_texture;

  MapTextures &getTextures() override { return *textures; }
  WaterAnimation &getWaterAnimation() override { return *water_animation; }

  void setMaterialMap(TerrainBase::MaterialMap::ConstPtr map) override
  {
    material_map = map;
  }

  void setCirrusTexture(std::shared_ptr<GenericImage> texture) override
  {
    cirrus_texture = texture;
  }
};


Map::Map(const char *path, ProgressReporter *progress,
         const render_util::ShaderSearchPath &shader_search_path,
         const render_util::ShaderParameters &shader_params,
         float max_cirrus_opacity) : p(new Private)
{
  const bool enable_base_map = false;//il2ge::core_wrapper::getConfig().enable_base_map;
  const bool enable_normal_maps = il2ge::core_wrapper::getConfig().enable_bumph_maps;

  FORCE_CHECK_GL_ERROR();

  p->textures = make_shared<render_util::MapTextures>(core::textureManager());

  FORCE_CHECK_GL_ERROR();

  p->water_animation = make_shared<render_util::WaterAnimation>();

  FORCE_CHECK_GL_ERROR();

  string ini_path = path;

  string map_dir = ini_path.substr(0, ini_path.find_last_of('/')) + '/';
  assert(!map_dir.empty());

  string dump_dir = dump_base_dir + util::makeLowercase(ini_path);
  string dump_map_dir = dump_base_dir + map_dir;

  if (il2ge::map_loader::isDumpEnabled())
  {
    auto res = util::mkdir(dump_base_dir.c_str());
    assert(res);
    res = util::mkdir(dump_map_dir.c_str());
    assert(res);
    res = util::mkdir(dump_dir.c_str());
    assert(res);
  }

  map_dir = string("maps/") + map_dir;
  ini_path = string("maps/") + path;

  RessourceLoader res_loader(map_dir, ini_path, dump_dir);

  render_util::ElevationMap::Ptr elevation_map_base;
  ImageGreyScale::Ptr land_map;

#if 0
  if (enable_base_map)
  {
    progress->report(1, "Creating base map");

    std::vector<char> il2ge_ini_content;
    if (sfs::readFile((map_dir + "il2ge.ini").c_str(), il2ge_ini_content))
    {
      if (il2ge::map_loader::isDumpEnabled())
        util::writeFile(dump_dir + '/' + "il2ge.ini", il2ge_ini_content.data(), il2ge_ini_content.size());

      INIReader ini(il2ge_ini_content.data(), il2ge_ini_content.size());
      if (!ini.ParseError())
      {
        p->base_map_origin.x = ini.GetReal("", "BaseMapOriginX", 0);
        p->base_map_origin.y = ini.GetReal("", "BaseMapOriginY", 0);
      }
    }

    std::vector<char> land_map_data;
    if (sfs::readFile(map_dir + map_generator::getBaseLandMapFileName(), land_map_data))
    {
      if (il2ge::map_loader::isDumpEnabled())
        util::writeFile(dump_dir + '/' + map_generator::getBaseLandMapFileName(),
                        land_map_data.data(),
                        land_map_data.size());
      land_map = render_util::loadImageFromMemory<ImageGreyScale>(land_map_data);
    }

    if (land_map)
      land_map = image::flipY(land_map);

    elevation_map_base = map_generator::generateHeightMap(land_map);
  }
#endif


  p->pixel_map_h = map_loader::createPixelMapH(&res_loader);

  assert(p->pixel_map_h);

  auto elevation_map = map_loader::createElevationMap(p->pixel_map_h);
  p->size = glm::vec2(elevation_map->getSize() * (int)il2ge::HEIGHT_MAP_METERS_PER_PIXEL);

  progress->report(3, "Creating textures");

  auto type_map = map_loader::createTypeMap(&res_loader);

  map_loader::createMapTextures(&res_loader, type_map, p);

#if 0
  if (land_map)
  {
    progress->report(4, "Creating land map texture");
    p->textures->setTexture(render_util::TEXUNIT_WATER_MAP_BASE, land_map);
  }

  if (enable_base_map)
  {
    progress->report(5, "Generating type map");
    assert(!field_texture_mapping.empty());

    auto base_type_map = map_generator::generateTypeMap(elevation_map_base);
    auto base_type_map_remapped = image::clone(base_type_map);
    base_type_map_remapped->forEach([&] (auto &pixel)
    {
      pixel = field_texture_mapping[pixel];
    });

    p->textures->setTexture(TEXUNIT_TYPE_MAP_BASE, base_type_map_remapped);
    p->textures->setTexture(TEXUNIT_FOREST_MAP_BASE, map_loader::createForestMap(base_type_map));
  }
#endif

  FORCE_CHECK_GL_ERROR();


  LandTextures land_textures;
  map_loader::createLandTextures(&res_loader, type_map, land_textures, enable_normal_maps);

  p->textures->setTexture(TEXUNIT_TERRAIN_FAR, land_textures.far_texture);

  p->textures->bind(core::textureManager());

  assert(p->material_map);

  p->terrain = createTerrain(textureManager(), g_terrain_use_lod, shader_search_path);

  FORCE_CHECK_GL_ERROR();

  progress->report(7, "Creating terrain");

  render_util::TerrainBase::BuildParameters params =
  {
    .map = elevation_map,
    .material_map = p->material_map,
    .type_map = land_textures.type_map,
    .textures = land_textures.textures,
    .textures_nm = land_textures.textures_nm,
    .texture_scale = land_textures.texture_scale,
    .shader_parameters = shader_params,
  };

  p->terrain->build(params);

#if 0
  if (elevation_map_base)
  {
    p->terrain->setBaseElevationMap(elevation_map_base);
  }
#endif

  FORCE_CHECK_GL_ERROR();

  if (p->cirrus_texture && il2ge::core_wrapper::getConfig().enable_cirrus_clouds)
  {
    p->cirrus_clouds = std::make_unique<render_util::CirrusClouds>(max_cirrus_opacity,
        core::textureManager(), shader_search_path, shader_params, 7000, p->cirrus_texture);
  }


  progress->report(10, "task.Load_landscape", false);
}


Map::~Map()
{
  delete p;
}


render_util::WaterAnimation *Map::getWaterAnimation()
{
  return p->water_animation.get();
}


render_util::CirrusClouds *Map::getCirrusClouds()
{
  return p->cirrus_clouds.get();
}


glm::vec2 Map::getSize()
{
  return p->size;
}

void Map::setUniforms(render_util::ShaderProgramPtr program)
{
  program->setUniform("map_size", getSize());
  program->setUniform("terrain_height_offset", 0.f);
  program->setUniform("height_map_base_origin", p->base_map_origin);
  p->textures->setUniforms(program);
  p->water_animation->updateUniforms(program);
}


render_util::TerrainBase &Map::getTerrain()
{
  return *p->terrain;
}


render_util::ImageGreyScale::ConstPtr Map::getPixelMapH()
{
  return p->pixel_map_h;
}


} // namespace core
