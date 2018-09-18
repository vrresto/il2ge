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

#ifndef CORE_MAP_H
#define CORE_MAP_H

#include <render_util/shader.h>

#include <glm/glm.hpp>

namespace render_util
{
  class TerrainBase;
  class WaterAnimation;
}

namespace core
{
  class Map
  {
    struct Private;
    Private *p = 0;

  public:
    Map(const char *path);
    ~Map();

    render_util::TerrainBase *getTerrain();
    render_util::ShaderProgramPtr getTerrainProgram();
    render_util::WaterAnimation *getWaterAnimation();
    glm::vec2 getSize();
    glm::ivec2 getTypeMapSize();

    void setUniforms(render_util::ShaderProgramPtr program);
  };
}

#endif
