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

#ifndef IL2GE_EFFECTS_H
#define IL2GE_EFFECTS_H

#include <il2ge/effect3d.h>
#include <render_util/texture_manager.h>
#include <render_util/image.h>

namespace il2ge
{


class Effects
{
  struct Impl;
  std::unique_ptr<Impl> p;

public:
  Effects();
  ~Effects();
  void add(std::unique_ptr<il2ge::Effect3D>);
  void remove(il2ge::Effect3D*);
  void update(float delta, const glm::vec2 &wind_speed);
  void render(const render_util::Camera &camera);

  virtual std::shared_ptr<render_util::GenericImage> createTexture(const Material&) = 0;
};


}


#endif
