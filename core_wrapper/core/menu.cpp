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

#include "menu.h"
#include "core_p.h"
#include <keys.h>
#include <core/scene.h>
#include <render_util/quad_2d.h>
#include <render_util/gl_binding/gl_functions.h>

#include <glm/glm.hpp>
#include <functional>
#include <sstream>

#include <windows.h>

using namespace core;
using namespace std;
using namespace render_util::gl_binding;


namespace core
{


Menu::Menu(Scene &scene, render_util::TextureManager &tex_mgr,
           render_util::ShaderSearchPath shader_seach_path) :
  m_scene(scene),
  m_display(tex_mgr, shader_seach_path)
{
  rebuild();
}


bool Menu::hasActiveParameter()
{
  return m_active_param >= 0 && m_active_param < m_scene.getNumParameters();
}


render_util::ParameterWrapper<float> &Menu::getActiveParameter()
{ 
  return m_scene.getParameter(m_active_param);
}


void Menu::setActiveParameter(int index)
{
  auto size = m_scene.getNumParameters();

  if (!size)
    return;

  if (index < 0)
    index += size;
  index = index % size;
  m_active_param = index;
}


void Menu::rebuild()
{
  m_display.clear();

  auto color = 0.7f * glm::vec3(0.4, 1.0, 0.4);

  m_display.addLine();
  m_display.addLine("Escape: exit menu", color);

  if (m_scene.getNumParameters())
  {
    m_display.addLine("Down/Up: navigate menu", color);
    m_display.addLine();
    m_display.addLine("Left/Right: change value by 0.01", color);
    m_display.addLine("-/+: change value by 1.00", color);
    m_display.addLine("PageDown/PageUp: change value by 100", color);
    m_display.addLine();
    m_display.addLine("(Shift multiplies change amount by 10)", color);
    m_display.addLine();
    m_display.addLine("r: reset value", color);
  }

  m_display.addLine();

  for (int i = 0; i < m_scene.getNumParameters(); i++)
  {
    auto &param = m_scene.getParameter(i);

    char buf[100];
    snprintf(buf, sizeof(buf), "%.2f", param.get());

    glm::vec3 color = (i == m_active_param) ? glm::vec3(1) : glm::vec3(0.6);

    m_display.addLine(param.name + ": " + buf, color);
    m_display.addLine();
  }
}


void Menu::draw()
{
  gl::Disable(GL_CULL_FACE);
  gl::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl::Enable(GL_BLEND);
  gl::Disable(GL_DEPTH_TEST);

  m_display.draw(m_scene.getTextRenderer(), 0, 0);
}


void Menu::nextEntry()
{
  setActiveParameter(m_active_param+1);
  rebuild();
}


void Menu::prevEntry()
{
  setActiveParameter(m_active_param-1);
  rebuild();
}


void Menu::changeValue(double increment)
{
  if (!hasActiveParameter())
    return;

  auto value = getActiveParameter().get();
  getActiveParameter().set(value + increment);

  rebuild();
}


void Menu::resetValue()
{
  if (hasActiveParameter())
    getActiveParameter().reset();

  rebuild();
}


void Menu::handleKey(int key, bool ctrl, bool alt, bool shift)
{
  using namespace il2ge::keys;

  float increment = 1.0;
  if (shift)
    increment = 10;

  switch (key)
  {
    case UP:
      prevEntry();
      break;
    case DOWN:
      nextEntry();
      break;
    case R:
      resetValue();
      break;
    case LEFT:
      changeValue(-0.01 * increment);
      break;
    case RIGHT:
      changeValue(+0.01 * increment);
      break;
    case SUBTRACT:
      changeValue(-1.0 * increment);
      break;
    case ADD:
      changeValue(+1.0 * increment);
      break;
    case PAGE_DOWN:
      changeValue(-100 * increment);
      break;
    case PAGE_UP:
      changeValue(+100 * increment);
      break;
  }
}


}
