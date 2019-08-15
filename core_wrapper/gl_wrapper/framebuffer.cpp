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

#include "gl_wrapper_private.h"


static_assert((GL_COLOR_ATTACHMENT0 + 1) == GL_COLOR_ATTACHMENT1);
static_assert((GL_COLOR_ATTACHMENT0 + 15) == GL_COLOR_ATTACHMENT15);


using render_util::TexturePtr;
using namespace render_util::gl_binding;


namespace core_gl_wrapper
{


FrameBuffer::FrameBuffer(glm::ivec2 size, size_t num_draw_buffers) : m_color_textures(num_draw_buffers)
{
  setSize(size);
  create();

  gl::NamedFramebufferTexture(m_id, GL_DEPTH_ATTACHMENT, m_depth_texture->getID(), 0);

  gl::NamedFramebufferReadBuffer(m_id, GL_COLOR_ATTACHMENT0);

  std::vector<GLuint> draw_buffers;

  for (int i = 0; i < m_color_textures.size(); i++)
  {
    gl::NamedFramebufferTexture(m_id, GL_COLOR_ATTACHMENT0 + i,
                                m_color_textures.at(i)->getID(), 0);

    draw_buffers.push_back(GL_COLOR_ATTACHMENT0 + i);
  }

  gl::NamedFramebufferDrawBuffers(m_id, draw_buffers.size(), draw_buffers.data());

  FORCE_CHECK_GL_ERROR();
}


FrameBuffer::~FrameBuffer()
{
  assert(m_id);
  assert(gl::IsFramebuffer(m_id));

  gl::DeleteFramebuffers(1, &m_id);

  m_id = 0;
}


void FrameBuffer::setSize(glm::ivec2 size)
{
  assert(size != glm::ivec2(0));

  if (size == m_size)
    return;

  {
    if (!m_depth_texture)
      m_depth_texture = render_util::Texture::create(GL_TEXTURE_2D);
    render_util::TemporaryTextureBinding binding(m_depth_texture);
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl::TexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, size.x, size.y,
                  0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  }

  for (int i = 0; i < m_color_textures.size(); i++)
  {
    auto &texture = m_color_textures[i];

    if (!texture)
      texture = render_util::Texture::create(GL_TEXTURE_2D);
    render_util::TemporaryTextureBinding binding(texture);
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl::TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y,
                  0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  }

  m_size = size;
}


void FrameBuffer::create()
{
  FORCE_CHECK_GL_ERROR();

  gl::GenFramebuffers(1, &m_id);
  FORCE_CHECK_GL_ERROR();

  gl::BindFramebuffer(GL_FRAMEBUFFER, m_id);
  gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
  FORCE_CHECK_GL_ERROR();

  assert(gl::IsFramebuffer(m_id));
}


}
