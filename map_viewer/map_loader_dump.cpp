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
    static string dir = "/dev/null/";
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
    if (scale)
    {
      string scale_path = map_dir + '/' + section + '_' + name + "_scale";
      vector<char> scale_content;
      if (util::readFile(scale_path, scale_content, false))
      {
        string scale_str(scale_content.data(), scale_content.size());
        try
        {
          *scale = stof(scale_str);
        }
        catch (...)
        {
          cerr<<"can't convert '"<<scale_str<<"' to float"<<endl;
          cerr<<"section: "<<section<<endl;
          cerr<<"key: "<<name<<endl;
          *scale = 1;
        }
      }
    }

    string path = map_dir + '/' + section + '_' + name + ".tga";

    return util::readFile(path, content);
  }

  bool readWaterAnimation(const std::string &file_name, std::vector<char> &content) override
  {
    string path = map_dir + '/' + file_name;
    return util::readFile(path, content);
  }


};


MapLoaderDump::MapLoaderDump(const std::string &path,
                             const render_util::TextureManager &texture_mgr) :
  m_path(path),
  m_texture_mgr(texture_mgr),
  m_res_loader(new RessourceLoader(m_path.c_str()))
{
  m_type_map = il2ge::map_loader::createTypeMap(m_res_loader);
  assert(m_type_map);
}

MapLoaderDump::~MapLoaderDump()
{
  delete m_res_loader;
  m_res_loader = nullptr;
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

  cout << "creating land map from: " << land_map_path <<endl;

  auto land_map = render_util::loadImageFromFile<ImageGreyScale>(land_map_path);

  if (land_map)
    land_map = image::flipY(land_map);

  return land_map;
}


void MapLoaderDump::createMapTextures(render_util::MapBase *map) const
{
  il2ge::map_loader::createMapTextures(m_res_loader, m_type_map, map);
}


void MapLoaderDump::createTerrainTextures(TerrainTextures &terrain_textures) const
{
  il2ge::map_loader::createTerrainTextures(m_res_loader, m_type_map, terrain_textures);
}


int MapLoaderDump::getHeightMapMetersPerPixel() const
{
  return il2ge::HEIGHT_MAP_METERS_PER_PIXEL;
}
