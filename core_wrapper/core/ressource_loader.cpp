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
#include <util.h>

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

}


core::RessourceLoader::RessourceLoader(const string &map_dir, const string &ini_path, const std::string &dump_path) :
  map_dir(map_dir),
  dump_dir(dump_path + '/')
{
  cout<<"reading load.ini"<<endl;
  std::vector<char> ini_content;
  if (!sfs::readFile(ini_path.c_str(), ini_content)) {
    assert(0);
  }
  cout<<"done reading load.ini"<<endl;

  dumpFile("load.ini", ini_content.data(), ini_content.size(), dump_dir);

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

    try
    {
      *scale = stof(scale_str);
    }
    catch (...)
    {
      cerr<<"can't convert '"<<scale_str<<"' to float"<<endl;
      cerr<<"section: "<<section<<endl;
      cerr<<"key: "<<name<<endl;
      cerr<<"value: "<<value<<endl;
      *scale = 1;
    }
  }

  string filename = value.substr(0, comma_pos);

  string filename_base = filename.substr(0, filename.find_last_of('.'));

  bool was_read = false;

  string dir = (from_map_dir) ? map_dir : "maps/_Tex/";

  was_read = ::readTextureFile(dir, filename_base, ".tgb", redirect, content);
  if (!was_read)
    was_read = ::readTextureFile(dir, filename_base, ".tga", redirect, content);

  return was_read;
}


bool core::RessourceLoader::readWaterAnimation(const string &file_name, std::vector<char> &content)
{
  string path = getWaterAnimationDir() + file_name;
  cout<<"reading "<<path<<endl;
  return sfs::readFile(path, content);
}
