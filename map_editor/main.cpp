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


#include <il2ge/exception_handler.h>
#include <il2ge/map_loader.h>
#include <render_util/viewer.h>
#include <render_util/image_loader.h>
#include <util.h>
#include <log.h>

#include <iostream>
#include <windows.h>
#include <shlobj.h>
#include <direct.h>

using namespace std;


bool il2ge::map_loader::isDumpEnabled()
{
  return false;
}


namespace {


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


struct ElevationMapLoader : public render_util::ElevationMapLoaderBase
{
  std::string m_map_path;

  ElevationMapLoader(std::string map_path) : m_map_path(map_path) {}

  render_util::ElevationMap::Ptr createElevationMap() const override
  {
    auto hm_image =
      render_util::loadImageFromFile<render_util::ImageGreyScale>(m_map_path);;

    if (hm_image)
      return il2ge::map_loader::createElevationMap(hm_image);
    else
      exit(1);
  }

  int getMetersPerPixel() const override
  {
    return 200;
  }
};


} // namespace


int main(int argc, char **argv)
{
  il2ge::exception_handler::install(g_crash_log_file_name);
  il2ge::exception_handler::watchModule(GetModuleHandle(0));

  string il2_dir = util::getDirFromPath(getExeFilePath());
  string map_path;

  if (argc == 2)
  {
    map_path = argv[1];
  }
  else
  {
    cerr << "Wrong number of arguments: " << argc << endl;
    return 1;
  }

  assert(!map_path.empty());

  _chdir(il2_dir.c_str());

  render_util::viewer::CreateElevationMapLoaderFunc create_map_loader_func = [&map_path] ()
  {
    return make_shared<ElevationMapLoader>(map_path);
  };

  render_util::viewer::runSimpleViewer(create_map_loader_func, "il2ge_map_editor");
}
