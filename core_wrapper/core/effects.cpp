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
#include <il2ge/image_loader.h>
#include <gl_wrapper.h>
#include <sfs.h>
#include <render_util/shader_util.h>
#include <render_util/gl_binding/gl_functions.h>


using namespace render_util::gl_binding;


namespace
{
  inline std::shared_ptr<render_util::GenericImage> createDummyTexture()
  {
    return std::make_shared<render_util::GenericImage>(glm::ivec2(2), 1);
  }
}


namespace core
{


const std::string SHADER_PATH = IL2GE_DATA_DIR "/shaders";


render_util::ShaderProgramPtr Effects::getDefaultShader()
{
  if (!m_default_shader)
  {
    m_default_shader = render_util::createShaderProgram("generic", core::textureManager(), SHADER_PATH);
    assert(m_default_shader);
    m_default_shader->setUniformi("sampler_0", 0);
  }

  return m_default_shader;
}


void Effects::add(std::unique_ptr<il2ge::Effect3D> effect, int cpp_obj)
{
  m_map[cpp_obj] = effect.get();
  il2ge::Effects::add(std::move(effect));
}


bool Effects::remove(int cpp_obj)
{
  auto e = m_map[cpp_obj];
  if (e)
  {
    m_map.erase(cpp_obj);
    il2ge::Effects::remove(e);
    return true;
  }
  else
    return false;
}


il2ge::Effect3D *Effects::get(int cpp_obj)
{
  return m_map[cpp_obj];
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

  il2ge::Effects::render(*core::getCamera());

  core_gl_wrapper::setShader(nullptr);

  gl::Disable(GL_DEPTH_TEST);
  gl::Disable(GL_CULL_FACE);
  gl::Enable(GL_BLEND);
}


std::shared_ptr<render_util::GenericImage> Effects::createTexture(const il2ge::Material &mat)
{
  if (mat.getLayers().empty())
    return createDummyTexture();

  auto path = mat.getLayers().front().texture_path;
  if (path.empty())
    return createDummyTexture();

  std::vector<char> content;
  if (sfs::readFile(path, content))
  {
//     util::writeFile("il2ge_dump/" + util::basename(path), content.data(), content.size());

    auto image = il2ge::loadImageFromMemory(content, path.c_str());
    if (!image)
    {
//       std::cout<<"path: "<<path<<std::endl;
//       util::writeFile("failed_image.tga", content.data(), content.size());
      return createDummyTexture();
    }
    else
      return image;
  }
  else
    return createDummyTexture();
}


} // namespace core
