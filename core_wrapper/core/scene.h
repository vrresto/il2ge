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

#ifndef CORE_SCENE_H
#define CORE_SCENE_H

#include "map.h"
#include <misc.h>
#include <core.h>
#include <render_util/camera.h>

#include <memory>
#include <cassert>

namespace core
{
  class Map;

  class Scene : public Module
  {
    render_util::TexturePtr atmosphere_map;
    render_util::TexturePtr curvature_map;
    std::unique_ptr<Map> map;

  public:
    render_util::TextureManager texture_manager = render_util::TextureManager(0);

    Scene();

    void unloadMap();
    void loadMap(const char *path);
    void update();
    void updateTerrain(const render_util::Camera &camera);
    void setTerrainDrawDistance(float distance);
    void drawTerrain(render_util::ShaderProgramPtr program);
    void updateUniforms(render_util::ShaderProgramPtr program);
    render_util::ShaderProgramPtr getTerrainProgram();
  };
};

#endif
