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

#ifndef IL2GE_FOREST_H
#define IL2GE_FOREST_H

#include <render_util/image.h>

#include <vector>

namespace render_util
{
  class MapTextures;
}

namespace il2ge
{
  class RessourceLoader;
}

namespace il2ge::map_loader
{
  void createForestTextures(render_util::ImageGreyScale::ConstPtr type_map,
                            render_util::MapTextures *map_textures,
                            il2ge::RessourceLoader *loader);
}

#endif
