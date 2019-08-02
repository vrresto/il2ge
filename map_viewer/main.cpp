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
#include <il2ge/exception_handler.h>
#include <render_util/viewer.h>
#include <util.h>
#include <log.h>

#include <iostream>
#include <windows.h>
#include <shlobj.h>
#include <direct.h>

using namespace std;


namespace {


const char* const g_log_file_name = "il2ge_map_viewer.log";


void atexitHandler()
{
}


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


// taken from https://www.codeproject.com/Articles/13088/%2FArticles%2F13088%2FHow-to-Browse-for-a-Folder
bool getFolder(string root_path, string &path)
{
  bool ret = false;

  OleInitialize(nullptr);

  root_path += "/il2ge_dump"; //HACK hardcoded

  PIDLIST_ABSOLUTE idl_root = ILCreateFromPathA(root_path.c_str());

  BROWSEINFO bi;
  memset(&bi, 0, sizeof(bi));

  bi.ulFlags = BIF_USENEWUI;
  bi.pidlRoot = idl_root;

  LPITEMIDLIST idl = SHBrowseForFolderA(&bi);

  CoTaskMemFree(idl_root);
  idl_root = nullptr;

  if(idl)
  {
    char buffer[_MAX_PATH] = {'\0'};
    if(SHGetPathFromIDListA(idl, buffer) != 0)
    {
      path = buffer;
      ret = true;
    }

    CoTaskMemFree(idl);
    idl = nullptr;
  }

  OleUninitialize();

  return ret;
}


} // namespace


int main(int argc, char **argv)
{
  std::atexit(atexitHandler);


  il2ge::exception_handler::install(g_log_file_name);
  il2ge::exception_handler::watchModule(GetModuleHandle(0));

  string il2_dir = util::getDirFromPath(getExeFilePath());
  string map_path;

  if (argc == 1)
  {
    string path;
    if (getFolder(il2_dir, path))
    {
      LOG_INFO<<"path: "<<path<<endl;
      map_path = path;
    }
    else
      return 1;
  }
  else if (argc == 2)
  {
    map_path = argv[1];
  }
  else
  {
    cerr << "Wrong number of arguments: " << argc << endl;
    cerr << "Usage: map_viewer [path to map directory]" << endl;
    return 1;
  }

  assert(!map_path.empty());

  _chdir(il2_dir.c_str());

  render_util::viewer::CreateMapLoaderFunc
    create_map_loader_func = [&map_path] (const render_util::TextureManager &texture_mgr)
  {
    return make_shared<MapLoaderDump>(map_path, texture_mgr);
  };

  render_util::viewer::runViewer(create_map_loader_func);
}
