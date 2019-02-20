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

#ifndef IL2GE_EFFECT3D_H
#define IL2GE_EFFECT3D_H

#include <il2ge/parameter_file.h>
#include <il2ge/material.h>
#include <render_util/camera.h>

#include <glm/gtc/matrix_transform.hpp>
#include <memory>
#include <functional>

namespace il2ge
{


class Effect3D;


struct Effect3DParticleBase
{
  Effect3D *effect = nullptr;

  glm::dvec3 pos{0};
  float size = 0;
  float rotation = 0;
  glm::vec4 color{0};
  unsigned long dist_from_camera_cm = 0;
};


class Effect3DRenderListBase
{
protected:
  std::vector<const Effect3DParticleBase*> m_list;

public:
  void add(const Effect3DParticleBase &particle)
  {
    m_list.push_back(&particle);
  }
};


class Effect3DParameters
{
public:
  std::string loaded_from;
  std::string loaded_from_content;

  float LiveTime = 0;
  float FinishTime = 0;

  virtual ~Effect3DParameters() {}

  virtual void set(const Effect3DParameters &other)
  {
    *this = other;
  }

  virtual void applyFrom(const ParameterFile &file)
  {
    auto &general = file.getSection("General");

    general.get("LiveTime", LiveTime);
    general.get("FinishTime", FinishTime);
  }

  virtual std::unique_ptr<Effect3D> createEffect() const = 0;
  virtual const char *getJavaClassName() const = 0;
};


class Effect3D
{
  const Effect3DParameters &m_params;
  glm::vec3 m_pos;
  glm::vec3 m_direction;
  glm::vec3 m_yaw_pitch_roll_deg;
  bool m_finished = false;
  bool m_paused = false;
  float m_intensity = 1;
  glm::vec3 m_pitch_axis {0};

  void updateRotation()
  {
    float yaw_angle_rad = glm::radians(m_yaw_pitch_roll_deg.x);
    float pitch_angle_rad = glm::radians(m_yaw_pitch_roll_deg.y);

    glm::mat4 rot_yaw = glm::rotate(glm::mat4(1), yaw_angle_rad, glm::vec3(0,0,1));

    auto pitch_axis = rot_yaw * glm::vec4{0,1,0,0};
    m_pitch_axis = glm::vec3(pitch_axis);

    glm::mat4 rot_pitch = glm::rotate(glm::mat4(1), -pitch_angle_rad, glm::vec3(pitch_axis));

    m_direction = glm::vec3{rot_pitch * rot_yaw * glm::vec4{1,0,0,0}};
  }

public:
  std::shared_ptr<const Material> material;


  Effect3D(const Effect3DParameters &params) : m_params(params) {}
  virtual ~Effect3D() {}

  virtual void render() = 0;
  virtual void addToRenderList(Effect3DRenderListBase&, const render_util::Camera&) = 0;
  virtual size_t getNumParticles() = 0;
  virtual void update(float delta, const glm::vec2 &wind_speed) {}

  float getLifeTime() const { return m_params.LiveTime; }
  float getFinishTime() const { return m_params.FinishTime; }

  void finish()
  {
    m_finished = true;
  }

  void pause(bool value)
  {
    m_paused = value;
  }

  bool isPaused() { return m_paused; }

  void setPos(const glm::vec3 &pos)
  {
    m_pos = pos;
  }

  float getIntensity() { return m_intensity; }

  void setIntensity(float value)
  {
    m_intensity = value;
  }

  void setYawPitchRollDeg(const glm::vec3 value)
  {
    m_yaw_pitch_roll_deg = value;
  }

  void setRotationDeg(float yaw, float pitch, float roll)
  {
    m_yaw_pitch_roll_deg = glm::vec3(yaw, pitch, roll);
    updateRotation();
  }

  bool isFinished() { return m_finished; }

  const glm::vec3 &getPos() { return m_pos; }
  const glm::vec3 &getDirection() { return m_direction; }
  const glm::vec3 &getYawPitchRollDeg() { return m_yaw_pitch_roll_deg; }
  const glm::vec3 &getPitchAxis() { return m_pitch_axis; }
};


std::unique_ptr<Effect3DParameters> createEffect3DParameters(const std::string &class_name);


} // namespace il2ge

#endif
