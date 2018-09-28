/**
 *    Rendering utilities
 *    Copyright (C) 2018  Jan Lepper
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

#ifndef RENDER_UTIL_MAP_GENERATOR_H
#define RENDER_UTIL_MAP_GENERATOR_H

#include <render_util/image.h>
#include <render_util/elevation_map.h>

namespace il2ge::map_generator
{
  enum
  {
    TERRAIN_TYPE_WATER,
    TERRAIN_TYPE_GRASS,
    TERRAIN_TYPE_FIELD,
    TERRAIN_TYPE_FIELD2,
    TERRAIN_TYPE_FIELD3,
    TERRAIN_TYPE_FIELD4,
    TERRAIN_TYPE_FOREST,
    TERRAIN_TYPE_TUNDRA,
    TERRAIN_TYPE_TUNDRA2,
    TERRAIN_TYPE_ROCK,
    TERRAIN_TYPE_ICE
  };

  render_util::ImageGreyScale::Ptr generateTypeMap(render_util::ElevationMap::ConstPtr elevation_map);
}

#endif
