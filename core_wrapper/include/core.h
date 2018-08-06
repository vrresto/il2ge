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

#include <glm/glm.hpp>
#include <render_util/shader.h>
#include <render_util/texture_manager.h>

namespace render_util
{
  class Camera;
  class Terrain;
}

namespace core
{

  class GLObjects;

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
    int num_rendered_objects = 0;
    int num_rendered_array_objects = 0;
  };

  unsigned int getTexUnitNum();
  int getTexUnit(int num);
  const char* getTexUnitName(int num);

  glm::vec2 getMapSize();
  glm::ivec2 getTypeMapSize();

  void drawTerrain(render_util::ShaderProgramPtr program);
  void updateTerrain();
  void setTerrainDrawDistance(float distance);

  bool isCubeUpdated();

  const glm::mat4 &getView2WorldMatrix();
  const glm::mat4 &getWorld2ViewMatrix();
  const glm::mat4 &getProjectionMatrixFar();
  const glm::vec3 &getCameraPos();

  float getWaterMapShift();
  float getWaterMapScale();

  const glm::vec3 &getSunDir();
  float getWaterAnimationFrameDelta();
  int getWaterAnimationStep();

  Il2CameraMode getCameraMode();
  void setCameraMode(Il2CameraMode);

  void getRenderState(Il2RenderState *state);

  float getFrameDelta();
  glm::vec4 getShoreWavePos();

  void onCubeMapBegin();
  void onCubeMapFaceFinished();
  void onObjectRendered();
  void onArrayObjectRendered();
  void onClear();
  void onClearStates();
  void onPrePreRenders();
  void onPostPreRenders();
  void onPostRenders();
  void onLandscapePreRender();
  void onLandscapeRender0();
  void onFarTerrainDone();
  void onLandscapeRender0Done();
  void onLandscapeRender1();
  void onLandscapePostRender();

  void onTreeDrawn();

  GLObjects *glObjects();

  render_util::ShaderProgramPtr activeShader();
  void setActiveShader(render_util::ShaderProgramPtr);
  render_util::ShaderProgramPtr activeARBProgram();
  void setActiveARBProgram(render_util::ShaderProgramPtr);
  void setIsARBProgramActive(bool active);

  void updateUniforms(render_util::ShaderProgramPtr program);

  bool isARBProgramActive();

  void loadMap(const char *path);
  void unloadMap();

  void updateWaterAnimation();

  void setSunDir(const glm::vec3 &dir);

  render_util::Camera *getCamera();

  render_util::TextureManager &textureManager();
}

#endif