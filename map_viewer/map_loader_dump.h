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

#ifndef MAP_LOADER_DUMP_H
#define MAP_LOADER_DUMP_H

#include <render_util/viewer.h>


class MapLoaderDump : public render_util::viewer::MapLoaderBase
{
  class RessourceLoader;

  std::string m_path;
  const render_util::TextureManager &m_texture_mgr;
  RessourceLoader *m_res_loader = nullptr;
  render_util::ImageGreyScale::Ptr m_type_map;

public:
  MapLoaderDump(const std::string &path, const render_util::TextureManager &texture_mgr);
  ~MapLoaderDump();

  render_util::ElevationMap::Ptr createElevationMap() const override;
  render_util::ElevationMap::Ptr
      createBaseElevationMap(render_util::ImageGreyScale::ConstPtr land_map) const override;
  render_util::ImageGreyScale::Ptr createBaseLandMap() const override;
  glm::vec2 getBaseMapOrigin() const override;

  void createLandTextures(render_util::LandTextures&) const override;
  void createMapTextures(render_util::MapBase*) const override;
  int getHeightMapMetersPerPixel() const override;
};

#endif
