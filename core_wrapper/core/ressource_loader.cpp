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

#include "ressource_loader.h"
#include "sfs.h"
#include <il2ge/map_loader.h>
#include <il2ge/image_loader.h>
#include <util.h>
#include <render_util/image_util.h>
#include <render_util/image_loader.h>
#include <render_util/texture_util.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;
using namespace util;

namespace
{


  void dumpFile(string name, const char *data, size_t data_size, string dir)
  {
    if (!il2ge::map_loader::isDumpEnabled())
      return;

    bool res = util::writeFile(dir + name, data, data_size);
    assert(res);
  }

  const string &getWaterAnimationDir()
  {
    static string dir = "maps/_Tex/Water/Animated/";
    return dir;
  }


  bool readTextureFile(string dir, string filename_base, string ext,
                      bool redirect, std::vector<char> &content)
  {
    string path = dir + filename_base + ext;

    bool was_read = sfs::readFile(path, content);

    if (was_read && redirect)
      sfs::redirect(sfs::getHash(path.c_str()), sfs::getHash("il2ge/dummy.tga"));

    return was_read;
  }


  void cacheNormalMap(string bumph_path, string cache_path, float scale)
  {
    cout<<"cacheNormalMap: "<<bumph_path<<endl;

    std::vector<char> content;
    if (sfs::readFile(bumph_path, content))
    {
      auto image = il2ge::loadImageFromMemory(content, bumph_path.c_str());
      if (image)
      {
        auto heightmap = render_util::image::getChannel(image, 0);

        auto normal_map = render_util::createNormalMap(heightmap, 5.0,
                                                       scale * il2ge::TERRAIN_METERS_PER_TEXTURE_TILE);
        cout<<"caching "<<bumph_path<<" -> "<<cache_path<<endl;
        render_util::saveImageToFile(cache_path, normal_map.get());
      }
      else
      {
        cout<<bumph_path<<": loadImageFromMemory() failed"<<endl;
      }
    }
    else
    {
      cout<<bumph_path<<" not found"<<endl;
    }
  }


  bool readNormalMapFile(string dir, string filename_base,
                      bool redirect, float scale, std::vector<char> &content)
  {
    string bumph_path = dir + filename_base + ".BumpH";

    cout<<"readNormalMapFile: "<<bumph_path<<endl;

    union
    {
      __int64 as_signed;
      uint64_t as_unsigned;
    } sfs_hash;

    sfs_hash.as_signed = sfs::getHash(bumph_path.c_str());

    auto res = util::mkdir(IL2GE_CACHE_DIR);
    assert(res);
    res = util::mkdir(IL2GE_CACHE_DIR "/bumph");
    assert(res);

    auto cache_path = string(IL2GE_CACHE_DIR "/bumph/") +
      to_string(sfs_hash.as_unsigned) + "_" + to_string(scale) + ".tga";

    bool was_read = util::readFile(cache_path, content, true);

    if (!was_read)
    {
      cacheNormalMap(bumph_path, cache_path, scale);
      was_read = util::readFile(cache_path, content, true);
    }

    if (was_read && redirect)
      sfs::redirect(sfs::getHash(bumph_path.c_str()), sfs::getHash("NULL"));

    return was_read;
  }


}


core::RessourceLoader::RessourceLoader(const string &map_dir, const string &ini_path, const std::string &dump_path) :
  map_dir(map_dir),
  dump_dir(dump_path + '/')
{
  LOG_TRACE<<"reading load.ini"<<endl;
  std::vector<char> ini_content;
  if (!sfs::readFile(ini_path.c_str(), ini_content)) {
    assert(0);
  }
  LOG_TRACE<<"done reading load.ini"<<endl;

  dumpFile("load.ini", ini_content.data(), ini_content.size(), dump_dir);

  LOG_TRACE<<"parsing load.ini"<<endl;
  reader = make_unique<INIReader>(ini_content.data(), ini_content.size());
  if (reader->ParseError()) {
    LOG_ERROR << "parse error at line " << reader->ParseError() << endl;
    exit(1);
  }
  LOG_TRACE<<"done parsing load.ini"<<endl;
}


std::string core::RessourceLoader::getDumpDir()
{
  return dump_dir;
}


glm::vec3 core::RessourceLoader::getWaterColor(const glm::vec3 &default_value)
{
  glm::vec3 water_color = default_value;

  string value = reader->Get("WATER", "WaterColorATI", "");
//   string value = reader->Get("WATER", "WaterColorNV", "");

  if (!value.empty())
  {
    istringstream stream(value);
    stream >> water_color.r;
    stream >> water_color.g;
    stream >> water_color.b;
  }

 return water_color;
}


bool core::RessourceLoader::readFile(const char *section,
          const char *name,
          const char *default_path,
          const char *suffix,
          std::vector<char> &content)
{
  string path = reader->Get(section, name, default_path);
  if (path.empty())
    return false;

  path = map_dir + path;

  if (suffix)
    path += suffix;

  return sfs::readFile(path, content);
}


bool core::RessourceLoader::readTextureFile(const char *section,
          const char *name,
          const char *default_path,
          std::vector<char> &content,
          bool from_map_dir,
          bool redirect,
          float *scale,
          bool is_bumpmap)
{
  string value = reader->Get(section, name, default_path);
  if (value.empty())
    return false;

  size_t comma_pos = value.find_first_of(',');
  if (scale && comma_pos != string::npos && value.size() > comma_pos+1)
  {
    string scale_str = value.substr(comma_pos+1, string::npos);

    try
    {
      *scale = stof(scale_str);
    }
    catch (...)
    {
      LOG_ERROR<<"can't convert '"<<scale_str<<"' to float"<<endl;
      LOG_ERROR<<"section: "<<section<<endl;
      LOG_ERROR<<"key: "<<name<<endl;
      LOG_ERROR<<"value: "<<value<<endl;
      *scale = 1;
    }
  }

  string filename = value.substr(0, comma_pos);

  string filename_base = filename.substr(0, filename.find_last_of('.'));

  bool was_read = false;

  string dir = (from_map_dir) ? map_dir : "maps/_Tex/";

  if (is_bumpmap)
  {
    was_read = ::readNormalMapFile(dir, filename_base, redirect, *scale, content);
  }
  else
  {
    was_read = ::readTextureFile(dir, filename_base, ".tgb", redirect, content);
    if (!was_read)
      was_read = ::readTextureFile(dir, filename_base, ".tga", redirect, content);
  }

  return was_read;
}


bool core::RessourceLoader::readWaterAnimation(const string &file_name, std::vector<char> &content)
{
  string path = getWaterAnimationDir() + file_name;
  LOG_TRACE<<"reading "<<path<<endl;
  return sfs::readFile(path, content);
}
