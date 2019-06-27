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

#ifndef CORE_MENU_H
#define CORE_MENU_H

#include <core/scene.h>
#include <render_util/text_display.h>

namespace core
{


class Menu
{
  bool m_is_shown = false;
  int m_active_param = 0;
  render_util::TextDisplay m_display;
  Scene &m_scene;

  void nextEntry();
  void prevEntry();
  void changeValue(double increment);
  void resetValue();
  void rebuild();
  bool hasActiveParameter();
  render_util::ParameterWrapper<float> &getActiveParameter();
  void setActiveParameter(int index);

public:
  Menu(Scene &scene, render_util::TextureManager&, render_util::ShaderSearchPath);

  bool isShown() { return m_is_shown; }
  void show(bool show) { m_is_shown = show; }

  void draw();

  void handleKey(int key);
};


}

#endif
