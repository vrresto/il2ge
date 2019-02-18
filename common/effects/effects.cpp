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
//     auto ViewMatrix = camera.getWorld2ViewMatrix();
//     vec3 CameraRight_worldspace {ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]};
//     vec3 CameraUp_worldspace {ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]};

    auto view_to_world_rot_mat = inverse(camera.getWorldToViewRotation());

    for (auto p : m_list)
    {
      vec3 pos = p->pos;
      auto size = p->size;
      auto &color = p->color;
      auto rotation = p->rotation;

      gl::Color4f(color.x, color.y, color.z, color.w);


      vec3 v0 = size * vec3{-0.5f, -0.5f, 0};
      vec3 v1 = size * vec3{+0.5f, -0.5f, 0};
      vec3 v2 = size * vec3{+0.5f, +0.5f, 0};
      vec3 v3 = size * vec3{-0.5f, +0.5f, 0};


      v0 = rotate(v0, rotation, vec3{0,0,1});
      v1 = rotate(v1, rotation, vec3{0,0,1});
      v2 = rotate(v2, rotation, vec3{0,0,1});
      v3 = rotate(v3, rotation, vec3{0,0,1});


      v0 = view_to_world_rot_mat * vec4(v0, 1);
      v1 = view_to_world_rot_mat * vec4(v1, 1);
      v2 = view_to_world_rot_mat * vec4(v2, 1);
      v3 = view_to_world_rot_mat * vec4(v3, 1);

      v0 += pos;
      v1 += pos;
      v2 += pos;
      v3 += pos;


      gl::Begin(GL_POLYGON);
      gl::Vertex3f(v0.x, v0.y, v0.z);
      gl::Vertex3f(v1.x, v1.y, v1.z);
      gl::Vertex3f(v2.x, v2.y, v2.z);
      gl::Vertex3f(v3.x, v3.y, v3.z);
      gl::End();

//       vec3 v0 =
//           pos
//           + CameraRight_worldspace * -0.5f * size
//           + CameraUp_worldspace * -0.5f * size;
//
//       vec3 v1 =
//           pos
//           + CameraRight_worldspace * 0.5f * size
//           + CameraUp_worldspace * -0.5f * size;
//
//       vec3 v2 =
//           pos
//           + CameraRight_worldspace * 0.5f * size
//           + CameraUp_worldspace * 0.5f * size;
//
//       vec3 v3 =
//           pos
//           + CameraRight_worldspace * -0.5f * size
//           + CameraUp_worldspace * 0.5f * size;
//
//       gl::Begin(GL_POLYGON);
//       gl::Vertex3f(v0.x, v0.y, v0.z);
//       gl::Vertex3f(v1.x, v1.y, v1.z);
//       gl::Vertex3f(v2.x, v2.y, v2.z);
//       gl::Vertex3f(v3.x, v3.y, v3.z);
//       gl::End();
    }
  }

};


} // namespace


namespace il2ge
{


struct Effects::Impl
{
  RenderList m_render_list;
  std::list<std::unique_ptr<Effect3D>> m_effects;
  std::unordered_map<Effect3D*, std::list<std::unique_ptr<Effect3D>>::iterator> m_map;

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
  p->m_render_list.clear();

  p->m_render_list.reserve(p->getNumParticles());

  for (auto &e : p->m_effects)
  {
    if (e->getIntensity() > 0)
      e->addToRenderList(p->m_render_list, camera);
  }

  p->m_render_list.sort();
  p->m_render_list.render(camera);
}


} // namespace il2ge
