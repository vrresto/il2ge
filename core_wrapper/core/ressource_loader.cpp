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
#include <util.h>

#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;
using namespace util;

namespace
{
  const string dump_dir = "il2ge_dump/";
  const bool dump_enabled = false;

  void dumpFile(string name, const char *data, size_t data_size)
  {
    if (!dump_enabled)
      return;

    bool res = util::writeFile(dump_dir + name, data, data_size);
    assert(res);
  }
}


core::RessourceLoader::RessourceLoader(const char *path_)
{
  string path = "maps/";
  path += path_;

  map_dir = path.substr(0, path.find_last_of('/')) + '/';
  assert(!map_dir.empty());

  cout<<"reading load.ini"<<endl;
  std::vector<char> ini_content;
  if (!sfs::readFile(path.c_str(), ini_content)) {
    assert(0);
  }
  cout<<"done reading load.ini"<<endl;

  dumpFile("load.ini", ini_content.data(), ini_content.size());

  cout<<"parsing load.ini"<<endl;
  reader = make_unique<INIReader>(ini_content.data(), ini_content.size());
  if (reader->ParseError()) {
    printf("parse error at line %d\n", reader->ParseError());
    exit(1);
  }
  cout<<"done parsing load.ini"<<endl;
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
          float *scale)
{
  string value = reader->Get(section, name, default_path);
  if (value.empty())
    return false;

  size_t comma_pos = value.find_first_of(',');
  if (scale && comma_pos != string::npos && value.size() > comma_pos+1)
  {
    string scale_str = value.substr(comma_pos+1, string::npos);
//       cout<<"scale: "<<scale_str<<endl;
    *scale = stof(scale_str);
  }

  string filename = value.substr(0, comma_pos);

  string path = (from_map_dir) ? map_dir : "maps/_Tex/";
  path += filename;

  if (redirect)
    sfs::redirect(sfs::getHash(path.c_str()), sfs::getHash("dummy.tga"));

  cout<<"reading "<<path<<endl;

  return sfs::readFile(path, content);
}


bool core::RessourceLoader::readTextureFile(const char *path_,
          std::vector<char> &content,
          bool redirect)
{
  string path = path_;

  if (redirect)
    sfs::redirect(sfs::getHash(path.c_str()), sfs::getHash("dummy.tga"));

  return sfs::readFile(path, content);
}
