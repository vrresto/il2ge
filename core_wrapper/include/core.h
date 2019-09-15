/**
 *    IL-2 Graphics Extender
 *    Copyright (C) 2018 Jan Lepper
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

#ifndef CORE_H
#define CORE_H

#include <il2ge/effect3d.h>
#include <render_util/shader.h>
#include <render_util/texture_manager.h>
#include <render_util/terrain_util.h>
#include <glm/glm.hpp>


namespace render_util
{
  class CirrusClouds;
  class Camera;
}

namespace core
{
  enum Il2RenderPhase
  {
    #define IL2_DECLARE_RENDER_PHASE(name) IL2_##name,
    #include "il2_render_phase.inc"
    #undef IL2_DECLARE_RENDER_PHASE
    IL2_RENDER_PHASE_MAX
  };

  enum Il2CameraMode
  {
    IL2_CAMERA_MODE_UNKNOWN,
    IL2_CAMERA_MODE_2D,
    IL2_CAMERA_MODE_3D
  };

  struct Il2RenderState
  {
    Il2CameraMode camera_mode = IL2_CAMERA_MODE_UNKNOWN;
    Il2RenderPhase render_phase = IL2_RENDER_PHASE_UNKNOWN;
    bool is_mirror = false;
  };

  bool isFMBActive();
  bool isMapLoaded();

  Il2CameraMode getCameraMode();
  void setCameraMode(Il2CameraMode);

  void getRenderState(Il2RenderState *state);

  void updateUniforms(render_util::ShaderProgramPtr program);

  void loadMap(const char *path, void*);
  void unloadMap();

  const glm::vec3 &getSunDir();
  void setSunDir(const glm::vec3 &dir);

  render_util::Camera *getCamera();
  render_util::TextureManager &textureManager();
  render_util::TerrainBase &getTerrain();
  const render_util::ShaderSearchPath &getShaderSearchPath();
  render_util::ShaderParameters getShaderParameters();
  render_util::CirrusClouds *getCirrusClouds();

  il2ge::Effect3D *getEffect(int cpp_obj);
  void addEffect(std::unique_ptr<il2ge::Effect3D> effect, int cpp_obj);
  bool removeEffect(int cpp_obj);
  void renderEffects();

  void setTime(uint64_t);
  void setWindSpeed(const glm::vec2&);

  render_util::ImageGreyScale::ConstPtr getPixelMapH();

  void onPrePreRenders();
  void onPostPreRenders();
  void onPostRenders();
  void onLandscapePreRender();
  void onLandscapeRender0(bool is_mirror);
  void onFarTerrainDone();
  void onLandscapeRender0Done();
  void onLandscapeRender1(bool is_mirror);
  void onLandscapePostRender();
  void onRender3D1FlushBegin();
  void onRender3D1FlushEnd();
  void onRenderCockpitBegin();
  void onRenderCockpitEnd();

  void showMenu(bool);
  bool isMenuShown();
  void handleKey(int key, bool ctrl, bool alt, bool shift);

  void init();
  void initJavaClasses();
}

#endif
