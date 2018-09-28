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

#ifndef IL2GE_MAP_LOADER_H
#define IL2GE_MAP_LOADER_H

#include <render_util/elevation_map.h>

#include <glm/glm.hpp>

namespace render_util
{
  class MapTextures;
  class TerrainBase;
  class WaterAnimation;
}

namespace il2ge
{
  class RessourceLoader;

  bool isDumpEnabled();

  void loadMap(il2ge::RessourceLoader *loader,
      render_util::MapTextures *map_textures,
      render_util::TerrainBase *terrain,
      render_util::WaterAnimation *water_animation,
      glm::vec2 &size,
      glm::ivec2 &type_map_size,
      render_util::ElevationMap::ConstPtr base_elevation_map = {});

  void createTerrain(il2ge::RessourceLoader *loader, render_util::TerrainBase *terrain);

  render_util::ElevationMap::Ptr generateHeightMap();
  render_util::ElevationMap::Ptr createElevationMap(il2ge::RessourceLoader *loader);
}

#endif
