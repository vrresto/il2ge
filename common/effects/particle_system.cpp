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


#include "factory.h"
#include <render_util/gl_binding/gl_functions.h>
#include <render_util/camera.h>

// #define GLM_FORCE_SSE2
// #define GLM_FORCE_AVX
// #define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
// #define GLM_FORCE_INTRINSICS
// #define GLM_FORCE_SIMD_AVX2
// #define GLM_FORCE_AVX512

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <random>
#include <set>
#include <algorithm>

static_assert(glm::vec4::length() == 4);

using namespace il2ge;
using namespace glm;
using namespace render_util::gl_binding;


#include <render_util/draw_box.h>

namespace
{


struct ParticleBase
{
  dvec3 pos{0};
  float size = 0;
  float rotation = 0;
  vec4 color{0};
  unsigned long dist_from_camera_cm = 0;
};


class RenderList
{
  std::vector<const ParticleBase*> m_list;

public:
  void reserve(size_t size)
  {
    m_list.reserve(size);
  }

  void clear()
  {
    m_list.clear();
  }

  void add(const ParticleBase &particle)
  {
    m_list.push_back(&particle);
  }

  void sort()
  {
    struct
    {
      bool operator()(const ParticleBase *a, const ParticleBase *b) const
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
    auto ViewMatrix = camera.getWorld2ViewMatrix();

    vec3 CameraRight_worldspace {ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]};
    vec3 CameraUp_worldspace {ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]};

    for (auto p : m_list)
    {

      vec3 pos = p->pos;
      auto size = p->size;
      auto &color = p->color;

      gl::Color4f(color.x, color.y, color.z, color.w);

      vec3 v0 =
          pos
          + CameraRight_worldspace * -0.5f * size
          + CameraUp_worldspace * -0.5f * size;

      vec3 v1 =
          pos
          + CameraRight_worldspace * 0.5f * size
          + CameraUp_worldspace * -0.5f * size;

      vec3 v2 =
          pos
          + CameraRight_worldspace * 0.5f * size
          + CameraUp_worldspace * 0.5f * size;

      vec3 v3 =
          pos
          + CameraRight_worldspace * -0.5f * size
          + CameraUp_worldspace * 0.5f * size;

      gl::Begin(GL_POLYGON);
      gl::Vertex3f(v0.x, v0.y, v0.z);
      gl::Vertex3f(v1.x, v1.y, v1.z);
      gl::Vertex3f(v2.x, v2.y, v2.z);
      gl::Vertex3f(v3.x, v3.y, v3.z);
      gl::End();
    }
  }

};


struct ParticleSystemParameters : public Effect3DParameters
{
  int nParticles = 0;
  float EmitFrq = 0;
  float VertAccel = 0;
  float Wind = 0;
  float GasResist = 0;
  vec2 EmitVelocity {0};
  vec2 Size {0};
  vec4 Color0 {0};
  vec4 Color1 {0};
  vec2 EmitTheta {0};
  float Rnd = 0;


  const char *getJavaClassName() const override
  {
    return "com/maddox/il2/engine/EffParticles";
  }

  void set(const Effect3DParameters &other_) override
  {
    auto other = dynamic_cast<const ParticleSystemParameters*>(&other_);
    assert(other);
    *this = *other;
  }


  void applyFrom(const ParameterFile &file) override
  {
    Effect3DParameters::applyFrom(file);

    auto &general = file.getSection("General");

    general.get("LiveTime", LiveTime);
    general.get("FinishTime", FinishTime);
    general.get("nParticles", nParticles);
    general.get("EmitFrq", EmitFrq);
    general.get("VertAccel", VertAccel);
    general.get("EmitVelocity", EmitVelocity);
    general.get("Size", Size);
    general.get("Color0", Color0);
    general.get("Color1", Color1);
    general.get("GasResist", GasResist);
    general.get("Wind", Wind);
    general.get("Rnd", Rnd);
    general.get("EmitTheta", EmitTheta);


//     if (!nParticles)
//     {
//       std::cout<<"nParticles: "<<nParticles<<std::endl;
//       std::cout<<"file: "<<loaded_from<<std::endl;
//       std::cout<<"content:\n"<<loaded_from_content<<std::endl;
//     }

    assert(LiveTime);
  }


  std::unique_ptr<Effect3D> createEffect() const override;
};


class ParticleSystem : public Effect3D
{
  struct Particle : public ParticleBase
  {
    float age = 0;
    vec3 speed {0};
  };

  const ParticleSystemParameters &m_params;
  std::vector<Particle> m_particles;
  float m_emit_timeout = 0;
  size_t m_num_particles = 0;
  size_t m_oldest_particle = 0;
  float m_age = 0;

  std::default_random_engine m_rand_engine;

  std::uniform_real_distribution<float> m_rand_speed_dist
  {
    m_params.EmitVelocity.x, m_params.EmitVelocity.y
  };

  std::uniform_real_distribution<float> m_rand_pitch_dist
  {
    glm::radians(m_params.EmitTheta.x), glm::radians(m_params.EmitTheta.y)
  };

  std::uniform_real_distribution<float> m_rand_yaw_dist {glm::radians(0.f), glm::radians(360.f)};


  static std::set<ParticleSystem*> s_all_effects;

  static size_t getNumParticles()
  {
    size_t num = 0;
    for (auto e : s_all_effects)
    {
      num += e->m_num_particles;
    }
    return num;
  }


public:
  ParticleSystem(const ParticleSystemParameters &params) : Effect3D(params), m_params(params)
  {
    m_particles.resize(m_params.nParticles);
    s_all_effects.insert(this);
  }

  ~ParticleSystem()
  {
    s_all_effects.erase(s_all_effects.find(this));
  }



  void initParticle(Particle &p, const glm::vec2 &wind_speed)
  {
    p.pos = getPos();
    p.age = 0;
    p.size = m_params.Size.x;

    float rand_pitch_angle_rad = m_rand_pitch_dist(m_rand_engine);
    float rand_yaw_angle_rad = m_rand_yaw_dist(m_rand_engine);

    auto dir = glm::rotate(getDirection(), rand_pitch_angle_rad, getPitchAxis());
    dir = glm::rotate(dir, rand_yaw_angle_rad, getDirection());

    float rand_speed = m_rand_speed_dist(m_rand_engine);

    p.speed = dir * rand_speed;
  }


  void emitParticle(const glm::vec2 &wind_speed)
  {
    if (m_particles.empty())
    {
    }
    else if (m_num_particles < m_particles.size())
    {
      initParticle(m_particles[m_num_particles], wind_speed);
      m_num_particles++;
    }
    else
    {
      initParticle(m_particles[m_oldest_particle], wind_speed);
      m_oldest_particle = (m_oldest_particle+1) % m_particles.size();
    }
  }


  vec3 applyAirResistance(vec3 speed_vec, const glm::vec2 &wind_speed, float delta)
  {
    vec3 airspeed_vec = speed_vec - vec3(wind_speed, 0);
    vec3 airspeed_dir = normalize(airspeed_vec);
    auto airspeed = length(airspeed_vec);
    auto drag_force = m_params.GasResist * airspeed * airspeed;

    airspeed -= drag_force * delta;
    airspeed_vec = airspeed_dir * airspeed;

    return airspeed_vec + vec3(wind_speed, 0);
  }


  bool isFinished()
  {
    return Effect3D::isFinished() || ((m_params.FinishTime > 0) && (m_age > m_params.FinishTime));
  }


  void update(float delta, const glm::vec2 &wind_speed_) override
  {
    if (isPaused())
      return;

    if (!m_params.EmitFrq)
      return;

    m_age += delta;

    auto wind_speed = wind_speed_ * m_params.Wind;

    m_emit_timeout -= delta;

    for (size_t i = 0; i < m_num_particles; i++)
    {
      auto &p = m_particles[(m_oldest_particle + i) % m_particles.size()];

      p.age += delta;

      float relative_age = p.age / m_params.LiveTime;

      vec3 speed = p.speed;

      vec3 accel {0};
      accel.z += m_params.VertAccel;

      speed += accel * delta;

      speed = applyAirResistance(speed, wind_speed_, delta);

      p.speed = speed;

      speed += vec3(wind_speed, 0);

      p.pos += speed * delta;

      p.size = mix(m_params.Size.x, m_params.Size.y, relative_age);

      p.color = mix(m_params.Color0, m_params.Color1, relative_age);
    }

    if (!isFinished())
    {
      while (m_emit_timeout <= 0)
      {
        emitParticle(wind_speed);
        m_emit_timeout += (1 / m_params.EmitFrq);
      }
    }
  }


  void render() override
  {
    if (getIntensity() <= 0)
      return;

    for (size_t i = 0; i < m_num_particles; i++)
    {
      auto &p = m_particles[(m_oldest_particle + i) % m_particles.size()];

      if (p.age > m_params.LiveTime)
        continue;

      auto &pos = p.pos;
      auto size = p.size / 2;
      auto &color = p.color;

      gl::Color4f(color.x, color.y, color.z, color.w);

      render_util::drawBox(pos.x, pos.y, pos.z, size);
    }
  }


  void addToRenderList(RenderList &list, const render_util::Camera &camera)
  {
    for (size_t i = 0; i < m_num_particles; i++)
    {
      auto &p = m_particles[(m_oldest_particle + i) % m_particles.size()];

      if (p.age >= m_params.LiveTime)
        continue;

      p.dist_from_camera_cm = distance(camera.getPosD(), p.pos) * 1000;

      list.add(p);
    }
  }


  static void renderAll(const render_util::Camera &camera)
  {
    RenderList render_list;

    render_list.clear();

    auto render_list_size = getNumParticles();

    render_list.reserve(render_list_size);

    for (auto e : s_all_effects)
    {
      if (e->getIntensity() > 0)
        e->addToRenderList(render_list, camera);
    }

    render_list.sort();
    render_list.render(camera);
  }

};


std::unique_ptr<Effect3D> ParticleSystemParameters::createEffect() const
{
  return std::make_unique<ParticleSystem>(*this);
}


std::set<ParticleSystem*> ParticleSystem::s_all_effects;


} // namespace


namespace il2ge
{
  std::unique_ptr<Effect3DParameters> create_TParticlesSystemParams()
  {
    return std::make_unique<ParticleSystemParameters>();
  }
}

void ParticleSystem_renderAll(const render_util::Camera &camera);
void ParticleSystem_renderAll(const render_util::Camera &camera)
{
  ParticleSystem::renderAll(camera);
}
