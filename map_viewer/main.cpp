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
#include <il2ge/log.h>
#include <il2ge/loader.h>
#include <il2ge/exception_handler.h>
#include <render_util/viewer.h>

#include <iostream>
#include <windows.h>

using namespace std;


const char* const g_log_file_name = "il2ge_map_viewer.log";

Logger g_log;


const char *getLogFileName()
{
  return g_log_file_name;
}


HMODULE getLoaderModule()
{
  return GetModuleHandle(0);
}


HMODULE getCoreWrapperModule()
{
  return GetModuleHandle(0);
}


static void atexitHandler()
{
  g_log.flush();
  g_log.m_outputs.clear();
}


int main(int argc, char **argv)
{
  std::atexit(atexitHandler);

  g_log.m_outputs.push_back(&cerr);

  installExceptionHandler();

  string map_path;

  if (argc == 1)
  {
    map_path = "il2ge_dump";
  }
  else if (argc == 2)
  {
    map_path = argv[1];
  }
  else
  {
    cerr << "Usage: map_viewer [path to map directory]" << endl;
    return 1;
  }

  assert(!map_path.empty());

  render_util::viewer::CreateMapLoaderFunc
    create_map_loader_func = [&map_path] (const render_util::TextureManager &texture_mgr)
  {
    return make_shared<MapLoaderDump>(map_path, texture_mgr);
  };

  render_util::viewer::runViewer(create_map_loader_func);
}
