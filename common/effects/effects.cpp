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

#include <il2ge/effects.h>
#include <render_util/gl_binding/gl_functions.h>
#include <render_util/texture_manager.h>
#include <render_util/texture_util.h>
#include <render_util/globals.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <list>
#include <unordered_map>

using namespace il2ge;
using namespace glm;
using namespace render_util::gl_binding;


namespace
{

struct RenderList : public Effect3DRenderListBase
{
  std::unordered_map<const Material*, render_util::TexturePtr> &m_textures;
  std::unordered_map<const Material*, bool> &m_texture_is_greyscale;


  RenderList(std::unordered_map<const Material*, render_util::TexturePtr> &textures,
             std::unordered_map<const Material*, bool> &texture_is_greyscale) :
    m_textures(textures), m_texture_is_greyscale(texture_is_greyscale) {}


  bool isEmpty() { return m_list.empty(); }


  void reserve(size_t size)
  {
    m_list.reserve(size);
  }


  void clear()
  {
    m_list.clear();
  }


  void sort()
  {
    struct
    {
      bool operator()(const Effect3DParticleBase *a, const Effect3DParticleBase *b) const
      {
        return (a->dist_from_camera_cm > b->dist_from_camera_cm);
      }
    }
    customLess;

    std::sort(m_list.begin(),
              m_list.end(),
              customLess);
  }


  void render(const render_util::Camera &camera)
  {
    auto view_to_world_rot_mat = inverse(camera.getWorldToViewRotation());

    const std::array<const vec3, 4> vertices
    {
      vec3{-0.5f, -0.5f, 0},
      vec3{+0.5f, -0.5f, 0},
      vec3{+0.5f, +0.5f, 0},
      vec3{-0.5f, +0.5f, 0},
    };

    gl::ActiveTexture(GL_TEXTURE0);

    for (auto p : m_list)
    {
      auto &color = p->color;
      gl::Color4f(color.x, color.y, color.z, color.w);

      assert(p->effect);
      assert(p->effect->material);

      auto &texture = m_textures[p->effect->material.get()];
      bool is_greyscale = m_texture_is_greyscale[p->effect->material.get()];

      assert(texture);

      gl::BindTexture(GL_TEXTURE_2D, texture->getID());
      auto prog = render_util::getCurrentGLContext()->getCurrentProgram();
      assert(prog);
      prog->setUniform("is_alpha_texture", is_greyscale);

      gl::Begin(GL_QUADS);
      for (auto v : vertices)
      {
        vec2 texcoord {v};
        texcoord += vec2(0.5);

        v *= p->size;
        v = rotate(v, p->rotation, vec3{0,0,1});
        v = view_to_world_rot_mat * vec4{v, 1};
        v += p->pos;

        gl::TexCoord4f(texcoord.x, texcoord.y, texcoord.x, texcoord.y);
        gl::MultiTexCoord4f(GL_TEXTURE0, texcoord.x, texcoord.y, texcoord.x, texcoord.y);
        gl::Vertex3f(v.x, v.y, v.z);

      }
      gl::End();
    }
  }

};


} // namespace


namespace il2ge
{


struct Effects::Impl
{
  std::list<std::unique_ptr<Effect3D>> m_effects;
  std::unordered_map<Effect3D*, std::list<std::unique_ptr<Effect3D>>::iterator> m_map;
  std::unordered_map<const Material*, render_util::TexturePtr> m_textures;
  std::unordered_map<const Material*, bool> m_texture_is_greyscale;
  RenderList m_render_list { m_textures, m_texture_is_greyscale };

  size_t getNumParticles()
  {
    size_t num = 0;
    for (auto &e : m_effects)
    {
      num += e->getNumParticles();
    }
    return num;
  }
};


Effects::Effects() : p(std::make_unique<Impl>()) {}


Effects::~Effects() {}


void Effects::add(std::unique_ptr<il2ge::Effect3D> effect)
{
  assert(effect->material);
  if (!p->m_textures[effect->material.get()])
  {
    auto image = createTexture(*effect->material);

    p->m_textures[effect->material.get()] = render_util::createTexture(image);
    p->m_texture_is_greyscale[effect->material.get()] = image->numComponents() == 1;
  }

  auto key = effect.get();
  auto pos = p->m_effects.insert(p->m_effects.end(), std::move(effect));
  p->m_map[key] = pos;
}


void Effects::remove(il2ge::Effect3D *effect)
{
  auto it = p->m_map.find(effect);
  assert(it != p->m_map.end());

  p->m_effects.erase(it->second);
  p->m_map.erase(it);
}


void Effects::update(float delta, const glm::vec2 &wind_speed)
{
  for (auto &e : p->m_effects)
  {
    e->update(delta, wind_speed);
  }
}


void Effects::render(const render_util::Camera &camera)
{
  assert(p->m_render_list.isEmpty());

  p->m_render_list.reserve(p->getNumParticles());

  for (auto &e : p->m_effects)
  {
    if (e->getIntensity() > 0)
      e->addToRenderList(p->m_render_list, camera);
  }

  p->m_render_list.sort();
  p->m_render_list.render(camera);

  p->m_render_list.clear();
}


} // namespace il2ge
