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


#define GLM_ENABLE_EXPERIMENTAL

#include <il2ge/exception_handler.h>
#include <il2ge/map_loader.h>
#include <render_util/viewer.h>
#include <render_util/image_loader.h>
#include <util.h>
#include <log.h>

#include <INIReader.h>

#include <glm/glm.hpp>
#include <glm/gtx/vec_swizzle.hpp>
#include <iostream>
#include <windows.h>
#include <shlobj.h>
#include <direct.h>

using namespace std;
using namespace glm;


bool il2ge::map_loader::isDumpEnabled()
{
  return false;
}


namespace
{


const char* const g_crash_log_file_name = "il2ge_map_editor_crash.log";


std::string getExeFilePath()
{
  char module_file_name[MAX_PATH];

  if (!GetModuleFileNameA(0,
      module_file_name,
      sizeof(module_file_name)))
  {
    abort();
  }

  return module_file_name;
}


int showOkCancelDialog(std::string text, std::string title)
{
  return MessageBoxA(nullptr, text.c_str(), title.c_str(), MB_OKCANCEL | MB_ICONQUESTION);
}


struct ElevationMapLoader : public render_util::ElevationMapLoaderBase
{
  std::string m_map_path;
  std::string m_base_map_path;

  ElevationMapLoader(std::string map_path, std::string base_map_path) :
    m_map_path(map_path), m_base_map_path(base_map_path)
  {
  }

  render_util::ElevationMap::Ptr createElevationMap() const override
  {
    auto hm_image =
      render_util::loadImageFromFile<render_util::ImageGreyScale>(m_map_path);

    if (hm_image)
      return il2ge::map_loader::createElevationMap(hm_image);
    else
      exit(1);
  }

  int getMetersPerPixel() const override
  {
    return 200;
  }

  render_util::ElevationMap::Ptr createBaseElevationMapRAW() const
  {
    constexpr auto BYTES_PER_PIXEL = 2;

    ivec2 size(4096);

    auto num_pixels = size.x * size.y;
    auto data_size = num_pixels * BYTES_PER_PIXEL;

    auto data = util::readFile<unsigned char>(m_base_map_path);

    cout<<"data.size(): "<<data.size()<<endl;
    assert(data.size() == data_size);

    auto image = make_shared<render_util::Image<int16_t>>(size, std::move(data));
    return render_util::image::convert<float>(image);
  }

  render_util::ElevationMap::Ptr createBaseElevationMapTGA() const
  {
      auto hm_image =
        render_util::loadImageFromFile<render_util::ImageGreyScale>(m_base_map_path);

      if (hm_image)
        return il2ge::map_loader::createElevationMap(hm_image);
      else
        exit(1);
  }

  render_util::ElevationMap::Ptr createBaseElevationMap() const override
  {
    if (!m_base_map_path.empty())
    {
      auto ext = util::makeLowercase(util::getFileExtensionFromPath(m_base_map_path));

      if (ext == "tga")
      {
        return createBaseElevationMapTGA();
      }
      else if(ext == "raw")
      {
        return createBaseElevationMapRAW();
      }
      else
      {
        cerr<<"Unhandled extension: "<<ext<<endl;
        exit(1);
      }
    }
    else
      return {};
  }

  int getBaseElevationMapMetersPerPixel() const
  {
    return 200;
  }

  glm::vec3 getBaseElevationMapOrigin(const glm::vec3 &default_value) const override
  {
    auto cfg_file_path = m_base_map_path + ".cfg";
    INIReader ini(cfg_file_path);
    if (!ini.ParseError())
    {
      return
      {
        .x = ini.GetReal("", "origin_x", default_value.x),
        .y = ini.GetReal("", "origin_y", default_value.y),
        .z = ini.GetReal("", "origin_z", default_value.z),
      };
    }
    else if (ini.ParseError() == -1)
    {
      return default_value;
    }
    else
    {
      throw std::runtime_error("Error parsing " + cfg_file_path
                                + " at line " + std::to_string(ini.ParseError()));
    }
  }

  void saveBaseElevationMapOrigin(const glm::vec3 &origin) override
  {
    auto cfg_file_path = m_base_map_path + ".cfg";
    if (util::fileExists(cfg_file_path))
    {
      if (showOkCancelDialog("Overwrite " + cfg_file_path + "?", "Confirm overwrite") != IDOK)
        return;
    }
    std::ofstream out(cfg_file_path);
    out << "origin_x = " << origin.x << std::endl;
    out << "origin_y = " << origin.y << std::endl;
    out << "origin_z = " << origin.z << std::endl;
  }
};


} // namespace


int main(int argc, char **argv)
{
  il2ge::exception_handler::install(g_crash_log_file_name);
  il2ge::exception_handler::watchModule(GetModuleHandle(0));

  string il2_dir = util::getDirFromPath(getExeFilePath());
  string map_path;
  string base_map_path;

  if (argc == 2)
  {
    map_path = argv[1];
  }
  else if (argc == 3)
  {
    map_path = argv[1];
    base_map_path = argv[2];
  }
  else
  {
    cerr << "Wrong number of arguments: " << argc << endl;
    return 1;
  }

  assert(!map_path.empty());

  _chdir(il2_dir.c_str());

  render_util::viewer::CreateElevationMapLoaderFunc create_map_loader_func = [map_path, base_map_path] ()
  {
    return make_shared<ElevationMapLoader>(map_path, base_map_path);
  };

  render_util::viewer::runSimpleViewer(create_map_loader_func, "il2ge_map_editor");
}
