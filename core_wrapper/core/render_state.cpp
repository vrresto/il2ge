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
#include <gl_wrapper.h>
#include <core/scene.h>
#include <render_util/water.h>

#include <iostream>

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

  void setRenderPhase(core::Il2RenderPhase phase, bool is_mirror = false)
  {
    g_il2_state.render_state.render_phase = phase;
    g_il2_state.render_state.is_mirror = is_mirror;
    core_gl_wrapper::onRenderPhaseChanged(g_il2_state.render_state);
  }


}

namespace core
{
  void setFMBActive(bool active)
  {
    g_il2_state.is_fmb_active = active;
  }

  bool isFMBActive()
  {
    return g_il2_state.is_fmb_active;
  }

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

  void onClearStates()
  {
    switch (g_il2_state.render_state.render_phase)
    {
      case core::IL2_PostPreRenders:
        setRenderPhase(core::IL2_PreLandscape);
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
    g_il2_state.render_state.render3d1_flushed = false;

    if (!g_il2_state.last_frame_time)
      g_il2_state.last_frame_time = g_il2_state.current_time;

    auto delta = g_il2_state.current_time - g_il2_state.last_frame_time;

    g_il2_state.last_frame_time = g_il2_state.current_time;
    g_il2_state.frame_delta = (float)delta / 1000.f;

//     shore_wave_pos.x = shore_wave_pos.x + (frame_delta * shore_wave_hz.x);
//     shore_wave_pos.y = shore_wave_pos.y + (frame_delta * shore_wave_hz.y);

    setRenderPhase(IL2_PrePreRenders);

    jni_wrapper::cleanGarbage();

    getScene()->update(g_il2_state.frame_delta, g_il2_state.wind_speed);
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

    if (isMenuShown())
    {
      getMenu().draw();
    }

//    cout<<"trees: "<<num_trees_drawn<<endl;
	}

  void onLandscapePreRender()
  {
    setRenderPhase(IL2_Landscape_cPreRender);
  }

//   int numRenderedCubeFaces = 0;

  void onLandscapeRender0(bool is_mirror)
  {
    setRenderPhase(IL2_Landscape0, is_mirror);

    //   core::updateWaterAnimation();
    //   getWaterAnimation()->update();
    //   core::bindGroundTexture();
  }

  void onLandscapeRender0Done()
  {
    setRenderPhase(IL2_Landscape0_Finished);

    //   core::updateWaterAnimation();
    //   getWaterAnimation()->update();
    //   core::bindGroundTexture();

    //   core_gl_wrapper::drawTerrain();
  }

  void onLandscapeRender1(bool is_mirror)
  {
    setRenderPhase(IL2_Landscape1, is_mirror);
  }

  void onLandscapePostRender()
  {
    setRenderPhase(IL2_PostLandscape);
  }

  void onRender3D1Flush()
  {
    if (!g_il2_state.render_state.render3d1_flushed)
    {
      renderEffects();
      g_il2_state.render_state.render3d1_flushed = true;
    }
  }

  void onRenderCockpitBegin()
  {
    setRenderPhase(IL2_Cockpit);
  }

  void onRenderCockpitEnd()
  {
    setRenderPhase(IL2_Cockpit_Finished);
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

//   float getFrameDelta()
//   {
//     return frame_delta;
//   }

  void setTime(uint64_t time)
  {
    g_il2_state.current_time = time;
  }


  void setWindSpeed(const glm::vec2 &speed)
  {
    g_il2_state.wind_speed = speed;
  }

}
