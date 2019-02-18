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

#include <core/effects.h>
#include <gl_wrapper.h>
#include <render_util/shader_util.h>
#include <render_util/gl_binding/gl_functions.h>



using namespace render_util::gl_binding;

void ParticleSystem_renderAll(const render_util::Camera &camera);

namespace core
{


const std::string SHADER_PATH = IL2GE_DATA_DIR "/shaders";


render_util::ShaderProgramPtr Effects::getDefaultShader()
{
  if (!m_default_shader)
    m_default_shader = render_util::createShaderProgram("generic", core::textureManager(), SHADER_PATH);
  assert(m_default_shader);
  return m_default_shader;
}


void Effects::add(std::unique_ptr<il2ge::Effect3D> effect, int cpp_obj)
{
  m_map[cpp_obj] = std::move(effect);
}


bool Effects::remove(int cpp_obj)
{
  return m_map.erase(cpp_obj);
}


il2ge::Effect3D *Effects::get(int cpp_obj)
{
  return m_map[cpp_obj].get();
}


void Effects::update(float delta, const glm::vec2 &wind_speed)
{
  for (auto &it : m_map)
  {
    it.second->update(delta, wind_speed);
  }
}


void Effects::render()
{
  core_gl_wrapper::setShader(getDefaultShader());
  core_gl_wrapper::updateUniforms(getDefaultShader());

  gl::Disable(GL_STENCIL_TEST);
  gl::Enable(GL_BLEND);
  gl::Enable(GL_DEPTH_TEST);
  gl::Enable(GL_CULL_FACE);
  gl::CullFace(GL_BACK);
  gl::DepthFunc(GL_LEQUAL);
  gl::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl::PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  gl::DepthMask(false);

//   for (auto &it : m_map)
//   {
//     gl::PointSize(2);
//     it.second->render();
//   }

  ParticleSystem_renderAll(*core::getCamera());

  core_gl_wrapper::setShader(nullptr);

  gl::Disable(GL_DEPTH_TEST);
  gl::Disable(GL_CULL_FACE);
  gl::Enable(GL_BLEND);
}


}
