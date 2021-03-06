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

#include <render_util/map_base.h>
#include <render_util/elevation_map.h>
#include <render_util/terrain_base.h>
#include <render_util/land_textures.h>

#include <glm/glm.hpp>
#include <map>


namespace render_util
{
  class MapTextures;
  class WaterAnimation;
}


namespace il2ge
{
  class RessourceLoader;

  enum
  {
    HEIGHT_MAP_METERS_PER_PIXEL = 200,
    TYPE_MAP_METERS_PER_PIXEL = 200,
    TERRAIN_METERS_PER_TEXTURE_TILE = 1600
  };
}


namespace il2ge::map_loader
{
  bool isDumpEnabled();

  void createLandTextures(il2ge::RessourceLoader*,
                          render_util::ImageGreyScale::ConstPtr type_map,
                          render_util::LandTextures&,
                          bool enable_normal_maps);

  void createMapTextures(il2ge::RessourceLoader*,
                        render_util::ImageGreyScale::ConstPtr,
                        render_util::MapBase*);

  render_util::ImageGreyScale::Ptr createTypeMap(il2ge::RessourceLoader*);
  render_util::ImageGreyScale::Ptr createForestMap(render_util::ImageGreyScale::ConstPtr type_map);

  render_util::ImageGreyScale::Ptr createPixelMapH(il2ge::RessourceLoader*);

  render_util::ElevationMap::Ptr createElevationMap(render_util::ImageGreyScale::ConstPtr);

  inline render_util::ElevationMap::Ptr createElevationMap(il2ge::RessourceLoader *loader)
  {
    return createElevationMap(createPixelMapH(loader));
  }
}


namespace il2ge::map_generator
{
  const std::string &getBaseLandMapFileName();

  render_util::ElevationMap::Ptr generateHeightMap(render_util::ImageGreyScale::ConstPtr land_map = {});

  render_util::ImageGreyScale::Ptr generateTypeMap(render_util::ElevationMap::ConstPtr elevation_map);
}



#endif
