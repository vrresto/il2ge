/**
 *    IL-2 Graphics Extender
 *    Copyright (C) 2019 Jan Lepper
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

#ifndef CORE_EFFECTS_H
#define CORE_EFFECTS_H

#include <il2ge/effect3d.h>
#include <render_util/shader.h>

#include <unordered_map>

namespace core
{


class Effects
{
  std::unordered_map<int, std::unique_ptr<il2ge::Effect3D>> m_map;
  render_util::ShaderProgramPtr m_default_shader;

  render_util::ShaderProgramPtr getDefaultShader();

public:
  void add(std::unique_ptr<il2ge::Effect3D> effect, int cpp_obj);
  bool remove(int cpp_obj);
  il2ge::Effect3D *get(int cpp_obj);

  void update(float delta, const glm::vec2 &wind_speed);
  void render();
};


}


#endif
