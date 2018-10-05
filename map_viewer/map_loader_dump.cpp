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


namespace
{


class RessourceLoader : public il2ge::RessourceLoader
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


} //namespace


void MapLoaderDump::loadMap(
    render_util::Map &map,
    bool &has_base_water_map,
    render_util::ElevationMap::Ptr &elevation_map,
    render_util::ElevationMap::Ptr *elevation_map_base)
{
  map.base_map_origin = glm::vec2(0);

  RessourceLoader res_loader(m_path.c_str());
  INIReader ini(m_path + "/il2ge.ini");
  if (!ini.ParseError())
  {
    map.base_map_origin.x = ini.GetReal("", "BaseMapOriginX", 0);
    map.base_map_origin.y = ini.GetReal("", "BaseMapOriginY", 0);
  }

  auto land_map_path = m_path + '/' + il2ge::map_generator::getBaseLandMapFileName();
  auto land_map = render_util::loadImageFromFile<ImageGreyScale>(land_map_path);

  if (elevation_map_base)
    *elevation_map_base = il2ge::map_generator::generateHeightMap(land_map);

  elevation_map = il2ge::map_loader::createElevationMap(&res_loader);

  il2ge::map_loader::createMapTextures(&res_loader,
                                       map.textures.get(),
                                       map.water_animation.get(),
                                       elevation_map_base ? *elevation_map_base :
                                          render_util::ElevationMap::ConstPtr());

  if (land_map)
  {
    has_base_water_map = true;
    map.textures->setTexture(render_util::TEXUNIT_WATER_MAP_BASE, image::flipY(land_map));
  }

  map.size = glm::vec2(elevation_map->getSize() * (int)il2ge::HEIGHT_MAP_METERS_PER_PIXEL);
}
