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
#include <render_util/texture_util.h>
#include <il2ge/map_loader.h>
#include <il2ge/ressource_loader.h>
#include <util.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>

using namespace std;
using namespace render_util;
using namespace glm;


namespace il2ge
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


void MapLoaderDump::loadMap(const std::string &path, render_util::Map &map)
{
  RessourceLoader res_loader(path.c_str());

  vec2 size;
  ivec2 type_map_size;

  il2ge::loadMap(&res_loader,
                 map.textures.get(),
                 map.terrain.get(),
                 map.water_animation.get(),
                 size,
                 type_map_size);

  map.size = size;
  map.type_map_size = type_map_size;
}
