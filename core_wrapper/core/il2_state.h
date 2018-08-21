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

#ifndef CORE_IL2_STATE_H
#define CORE_IL2_STATE_H

#include <core.h>
#include <render_util/camera.h>

#include <chrono>
#include <glm/glm.hpp>

namespace core
{


struct IL2State
{
  render_util::Camera camera;
  glm::vec3 sun_dir;
  Il2RenderState render_state;
//   std::chrono::steady_clock Clock::time_point last_frame_time; // = Clock::time_point(std::chrono::seconds(0));
//   float frame_delta = 0;
};


}

#endif
