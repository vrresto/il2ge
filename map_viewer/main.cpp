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
#include <render_util/viewer.h>

namespace
{
  const char map_path[] = "../../../dumps/narvik";
}

using namespace std;

namespace render_util
{
  const std::string &getResourcePath()
  {
    static string path = "../../render_util";
    return path;
  }

  const std::string &getDataPath()
  {
    static string path = "../../../ge_data";
    return path;
  }
}

int main()
{
  runViewer(make_shared<MapLoaderDump>(), map_path);
}
