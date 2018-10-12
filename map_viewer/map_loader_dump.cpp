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

#include "map_loader_dump.h"
#include <render_util/map.h>
#include <render_util/image.h>
#include <render_util/image_loader.h>
#include <render_util/image_util.h>
#include <render_util/texture_util.h>
#include <render_util/texunits.h>
#include <il2ge/map_loader.h>
#include <il2ge/ressource_loader.h>
#include <util.h>

#include <INIReader.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

using namespace std;
using namespace render_util;
using namespace glm;


namespace il2ge::map_loader
{
  bool isDumpEnabled() { return false; }
}




class MapLoaderDump::RessourceLoader : public il2ge::RessourceLoader
{
  std::string map_dir;

public:
  RessourceLoader(const char *path)
  {
    map_dir = path;
    assert(!map_dir.empty());
  }

  std::string getDumpDir() override
  {
    static string dir = "/dev/null";
    return dir;
  }

  glm::vec3 getWaterColor(const glm::vec3 &default_value) override
  {
    return default_value;
  }

  bool readFile(const char *section,
            const char *name,
            const char *default_path,
            const char *suffix,
            std::vector<char> &content)
  {
    string path = map_dir + '/' + section + '_' + name + ".tga";
    if (suffix)
      path += suffix;

    return util::readFile(path, content);
  }

  bool readTextureFile(const char *section,
            const char *name,
            const char *default_path,
            std::vector<char> &content,
            bool from_map_dir,
            bool redirect,
            float *scale) override
  {
    string path = map_dir + '/' + section + '_' + name + ".tga";

    return util::readFile(path, content);
  }

  bool readWaterAnimation(const std::string &file_name, std::vector<char> &content) override
  {
    string path = map_dir + '/' + file_name;
    return util::readFile(path, content);
  }


};


namespace
{


} //namespace


namespace il2ge::viewer
{


class Map : public render_util::MapBase
{
  std::shared_ptr<MapTextures> m_textures;
  std::shared_ptr<WaterAnimation> m_water_animation;
  map<unsigned, unsigned> m_field_texture_mapping;

public:
  Map(const render_util::TextureManager &texture_mgr, RessourceLoader*);
  MapTextures &getTextures() override { return *m_textures; }
  WaterAnimation &getWaterAnimation() override { return *m_water_animation; }

  getHeightMapMetersPerPixel() const override
  {
    return il2ge::HEIGHT_MAP_METERS_PER_PIXEL;
  }

  void buildBaseMap(render_util::ElevationMap::ConstPtr elevation_map,
                    render_util::ImageGreyScale::ConstPtr land_map) override;
};


Map::Map(const render_util::TextureManager &texture_mgr, RessourceLoader *res_loader) :
  m_textures(make_shared<MapTextures>(texture_mgr)),
  m_water_animation(make_shared<WaterAnimation>())
{
  assert(m_textures);
  assert(m_water_animation);

  il2ge::map_loader::createMapTextures(res_loader,
                                       m_textures.get(),
                                       m_water_animation.get(),
                                       m_field_texture_mapping);
}


void Map::buildBaseMap(render_util::ElevationMap::ConstPtr elevation_map,
                       render_util::ImageGreyScale::ConstPtr land_map)
{
  assert(!m_field_texture_mapping.empty());

  auto base_type_map = map_generator::generateTypeMap(elevation_map);

  auto base_type_map_remapped = image::clone(base_type_map);
  base_type_map_remapped->forEach([&] (auto &pixel)
  {
    pixel = m_field_texture_mapping[pixel];
  });

  m_textures->setTexture(TEXUNIT_TYPE_MAP_BASE, base_type_map_remapped);
  m_textures->setTexture(TEXUNIT_FOREST_MAP_BASE, map_loader::createForestMap(base_type_map));
}


} // namespace il2ge::viewer


MapLoaderDump::MapLoaderDump(const std::string &path,
                             const render_util::TextureManager &texture_mgr) :
  m_path(path),
  m_texture_mgr(texture_mgr),
  m_res_loader(new RessourceLoader(m_path.c_str()))
{
}

MapLoaderDump::~MapLoaderDump()
{
  delete m_res_loader;
  m_res_loader = nullptr;
}

std::shared_ptr<render_util::MapBase> MapLoaderDump::loadMap() const
{
  return make_shared<il2ge::viewer::Map>(m_texture_mgr, m_res_loader);
  abort();
}

render_util::ElevationMap::Ptr MapLoaderDump::createElevationMap() const
{
  assert(m_res_loader);
  return il2ge::map_loader::createElevationMap(m_res_loader);
}

render_util::ElevationMap::Ptr
MapLoaderDump::createBaseElevationMap(render_util::ImageGreyScale::ConstPtr land_map) const
{
  return il2ge::map_generator::generateHeightMap(land_map);
}


glm::vec2 MapLoaderDump::getBaseMapOrigin() const
{
  glm::vec2 ret = glm::vec2(0);

  INIReader ini(m_path + "/il2ge.ini");
  if (!ini.ParseError())
  {
    ret.x = ini.GetReal("", "BaseMapOriginX", 0);
    ret.y = ini.GetReal("", "BaseMapOriginY", 0);
  }

  return ret;
}


ImageGreyScale::Ptr MapLoaderDump::createBaseLandMap() const
{
  auto land_map_path = m_path + '/' + il2ge::map_generator::getBaseLandMapFileName();
  auto land_map = render_util::loadImageFromFile<ImageGreyScale>(land_map_path);

  assert(land_map);

  land_map = image::flipY(land_map);

  return land_map;
}
