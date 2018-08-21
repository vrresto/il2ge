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

#include "core.h"
#include "core_p.h"
#include "il2_state.h"

#include <iostream>
#include <GL/gl.h>

#include <gl_wrapper/gl_functions.h>

using namespace gl_wrapper::gl_functions;
using namespace glm;
using namespace core;
using namespace std;

using Clock = std::chrono::steady_clock;


namespace
{
//   const vec4 shore_wave_hz = vec4(0.05, 0.07, 0, 0);
//   vec4 shore_wave_pos = vec4(0);


//   void onCubeMapFinished()
//   {
//     assert(0);
//
//     g_il2_state.render_state.num_rendered_objects = 0;
//     g_il2_state.render_state.num_rendered_array_objects = 0;
//
//     setRenderPhase(IL2_Landscape0_PreTerrain);
//   }

  IL2State g_il2_state;

  void setRenderPhase(core::Il2RenderPhase phase)
  {
    g_il2_state.render_state.render_phase = phase;
  }


}

namespace core
{

  void getRenderState(Il2RenderState *state)
  {
    *state = g_il2_state.render_state;
  }

  render_util::Camera *getCamera()
  {
    return &g_il2_state.camera;
  }

//   vec4 getShoreWavePos()
//   {
//     return shore_wave_pos;
//   }

  void onObjectRendered()
  {
    g_il2_state.render_state.num_rendered_objects++;
  }

  void onArrayObjectRendered()
  {
    g_il2_state.render_state.num_rendered_array_objects++;
  }

  void onClear()
  {
    g_il2_state.render_state.num_rendered_objects = 0;
    g_il2_state.render_state.num_rendered_array_objects = 0;
  }

  void onClearStates()
  {
    switch (g_il2_state.render_state.render_phase)
    {
      case core::IL2_PostPreRenders:
        g_il2_state.render_state.num_rendered_objects = 0;
        g_il2_state.render_state.num_rendered_array_objects = 0;
        setRenderPhase(core::IL2_PreLandscape);
        break;
      case core::IL2_PostLandscape:
        setRenderPhase(core::IL2_Cockpit);
        break;
      default:
//         printf("====================================================================================\n");
//         printf("!!!!!!!!!!!!!!  unexpected clearStates in render phase %d !!!!!!!!!!!!!!!!\n", core::getRenderPhase());
//         printf("====================================================================================\n");
//         setRenderPhase(core::IL2_RENDER_PHASE_UNKNOWN);
        break;
    }
  }

  void onPrePreRenders()
  {
//     Clock::time_point frame_time =  Clock::now();

//     std::chrono::microseconds delta = std::chrono::duration_cast<std::chrono::microseconds>(frame_time - last_frame_time);
//     frame_delta = ((float)delta.count()) / 1000000.0;

//     last_frame_time = frame_time;

//     shore_wave_pos.x = shore_wave_pos.x + (frame_delta * shore_wave_hz.x);
//     shore_wave_pos.y = shore_wave_pos.y + (frame_delta * shore_wave_hz.y);

    setRenderPhase(IL2_PrePreRenders);
  }

  void onPostPreRenders()
  {
    setRenderPhase(IL2_PostPreRenders);
  }

  void onPostRenders()
  {
//   printf("rendered %d objects.\n", g_il2_state.render_state.num_rendered_objects);
//   printf("rendered %d array objects.\n", g_il2_state.render_state.num_rendered_array_objects);
//   printf("=========================================\n");

    setRenderPhase(IL2_PostRenders);

//    cout<<"trees: "<<num_trees_drawn<<endl;
	}

  void onLandscapePreRender()
  {
    setRenderPhase(IL2_Landscape_cPreRender);
  }

//   int numRenderedCubeFaces = 0;

  void onLandscapeRender0()
  {
    g_il2_state.render_state.num_rendered_objects = 0;
    g_il2_state.render_state.num_rendered_array_objects = 0;
//     numRenderedCubeFaces = 0;

    setRenderPhase(IL2_Landscape0_PreTerrain);

//     setRenderPhase(isCubeUpdated() ? IL2_Landscape0_CubeMap : IL2_Landscape0_PreTerrain);

    //   core::updateWaterAnimation();
    //   getWaterAnimation()->update();
    //   core::bindGroundTexture();
  }

  void onCubeMapBegin()
  {
    setRenderPhase(IL2_Landscape0_CubeMap);
  }

  void onCubeMapFaceFinished()
  {
//     numRenderedCubeFaces++;
//     if (numRenderedCubeFaces == 6)
//       onCubeMapFinished();
  }

  void onFarTerrainDone()
  {
    setRenderPhase(IL2_Landscape0_TerrainNear);
  }

  void onLandscapeRender0Done()
  {
    setRenderPhase(IL2_Landscape0_Finished);

    //   core::updateWaterAnimation();
    //   getWaterAnimation()->update();
    //   core::bindGroundTexture();

    //   core_gl_wrapper::drawTerrain();
  }

  void onLandscapeRender1()
  {
    setRenderPhase(IL2_Landscape1);
  }

  void onLandscapePostRender()
  {
    setRenderPhase(IL2_PostLandscape);
  }

  const vec3 &getSunDir()
  {
    return g_il2_state.sun_dir;
  }

  void setSunDir(const vec3 &dir)
  {
    g_il2_state.sun_dir = dir;
  }

  void setCameraMode(Il2CameraMode mode)
  {
    g_il2_state.render_state.camera_mode = mode;
  }

  Il2CameraMode getCameraMode()
  {
    return g_il2_state.render_state.camera_mode;
  }

  Il2RenderPhase getRenderPhase()
  {
    return g_il2_state.render_state.render_phase;
  }

  const glm::mat4 &getView2WorldMatrix()
  {
    return g_il2_state.camera.getView2WorldMatrix();
  }

  const glm::mat4 &getWorld2ViewMatrix()
  {
    return g_il2_state.camera.getWorld2ViewMatrix();
  }

  const glm::mat4 &getProjectionMatrixFar()
  {
    return g_il2_state.camera.getProjectionMatrixFar();
  }

  const glm::vec3 &getCameraPos()
  {
    return g_il2_state.camera.getPos();
  }

//   float getFrameDelta()
//   {
//     return frame_delta;
//   }


}
