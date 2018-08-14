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
#include "gl_objects.h"
#include <render_util/camera.h>

#include <chrono>
#include <iostream>
#include <GL/gl.h>

#include <gl_wrapper/gl_functions.h>

using namespace gl_wrapper::gl_functions;
using namespace glm;
using namespace core;
using namespace std;

using Clock = std::chrono::steady_clock;


namespace core_gl_wrapper
{
  void drawTerrain();
}

namespace
{
//   const float shore_wave_hz = 0.05;
  const vec4 shore_wave_hz = vec4(0.05, 0.07, 0, 0);

  Clock::time_point last_frame_time = Clock::time_point(std::chrono::seconds(0));
  float frame_delta = 0;
  vec4 shore_wave_pos = vec4(0);

  vec3 sun_dir;
  Il2RenderState render_state;

  render_util::Camera camera;


  int num_trees_drawn = 0;

//   int numTimesFarTerrainDrawn = 0;

  void setRenderPhase(Il2RenderPhase phase)
  {
    if (phase != IL2_RENDER_PHASE_UNKNOWN)
    {
      if (render_state.render_phase == phase)
      {
//         cout<<"error: already in render phase "<<phase<<endl;
//         exit(1);
      }
//       assert (render_state.render_phase != phase);
    }

    render_state.render_phase = phase;
//     render_state.num_rendered_objects = 0;
//     render_state.num_rendered_array_objects = 0;
  }

  void updateShaderState()
  {
    auto *ctx = glObjects();

    bool is_arb_program_active = ctx->is_arb_program_active;

//     if (is_arb_program_active && render_state.render_phase == IL2_Landscape0_PreTerrain)
//     {
//       setRenderPhase(IL2_Landscape0_TerrainFar);
//     }

    render_util::ShaderProgramPtr new_active_shader;

    if (ctx->current_shader)
    {
      new_active_shader = ctx->current_shader;
    }
    else if (is_arb_program_active)
    {
      new_active_shader = ctx->current_arb_program;
    }
//     if (is_arb_program_active)
//     {
//       new_active_shader = ctx->current_arb_program;
//     }
//     else
//     {
//       new_active_shader = ctx->current_shader;
//     }

    if (new_active_shader)
    {
      assert(new_active_shader->isValid());
//       if (new_active_shader != ctx->active_shader)
//       {
        gl::UseProgram(new_active_shader->getId());
        ctx->active_shader = new_active_shader;
//       }
    }
    else
    {
      gl::UseProgram(0);
      ctx->active_shader = nullptr;
    }
  }

  void onCubeMapFinished()
  {
    assert(0);

    render_state.num_rendered_objects = 0;
    render_state.num_rendered_array_objects = 0;

    setRenderPhase(IL2_Landscape0_PreTerrain);
  }


}

namespace core
{

  void getRenderState(Il2RenderState *state)
  {
    *state = render_state;
  }

  render_util::Camera *getCamera()
  {
    return &camera;
  }

  vec4 getShoreWavePos()
  {
    return shore_wave_pos;
  }

  void onObjectRendered()
  {
    render_state.num_rendered_objects++;
  }

  void onArrayObjectRendered()
  {
    render_state.num_rendered_array_objects++;
  }

  void onClear()
  {
    render_state.num_rendered_objects = 0;
    render_state.num_rendered_array_objects = 0;
  }

  void onClearStates()
  {
    switch (render_state.render_phase)
    {
      case core::IL2_PostPreRenders:
        render_state.num_rendered_objects = 0;
        render_state.num_rendered_array_objects = 0;
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
    num_trees_drawn = 0;

    Clock::time_point frame_time =  Clock::now();

    std::chrono::microseconds delta = std::chrono::duration_cast<std::chrono::microseconds>(frame_time - last_frame_time);
    frame_delta = ((float)delta.count()) / 1000000.0;

    last_frame_time = frame_time;

//     shore_wave_pos = fract(shore_wave_pos + (frame_delta * shore_wave_hz));
    shore_wave_pos.x = shore_wave_pos.x + (frame_delta * shore_wave_hz.x);
    shore_wave_pos.y = shore_wave_pos.y + (frame_delta * shore_wave_hz.y);

    setRenderPhase(IL2_PrePreRenders);
  }

  void onPostPreRenders()
  {
    setRenderPhase(IL2_PostPreRenders);
  }

  void onPostRenders()
  {
//   printf("rendered %d objects.\n", render_state.num_rendered_objects);
//   printf("rendered %d array objects.\n", render_state.num_rendered_array_objects);
//   printf("=========================================\n");

    setRenderPhase(IL2_PostRenders);

//    cout<<"trees: "<<num_trees_drawn<<endl;
	}

  void onLandscapePreRender()
  {
    setRenderPhase(IL2_Landscape_cPreRender);
  }

  int numRenderedCubeFaces = 0;

  void onLandscapeRender0()
  {
    render_state.num_rendered_objects = 0;
    render_state.num_rendered_array_objects = 0;
    numRenderedCubeFaces = 0;

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
    numRenderedCubeFaces++;
    if (numRenderedCubeFaces == 6)
      onCubeMapFinished();
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
    return sun_dir;
  }

  void setSunDir(const vec3 &dir)
  {
    sun_dir = dir;
  }

  void setCameraMode(Il2CameraMode mode)
  {
    render_state.camera_mode = mode;
  }

  Il2CameraMode getCameraMode()
  {
    return render_state.camera_mode;
  }

  Il2RenderPhase getRenderPhase()
  {
    return render_state.render_phase;
  }

  render_util::ShaderProgramPtr activeShader()
  {
    assert(false);
    abort();
  }

  void setActiveShader(render_util::ShaderProgramPtr shader)
  {
    glObjects()->current_shader = shader;
    updateShaderState();
  }

  render_util::ShaderProgramPtr activeARBProgram()
  {
    assert(false);
    abort();
  }

  void setActiveARBProgram(render_util::ShaderProgramPtr prog)
  {
    glObjects()->current_arb_program = prog;
    updateShaderState();
  }

  void setIsARBProgramActive(bool active)
  {
    glObjects()->is_arb_program_active = active;
    updateShaderState();
  }

  bool isARBProgramActive()
  {
    return glObjects()->is_arb_program_active;
  }

  render_util::TextureManager &textureManager()
  {
    return glObjects()->texture_manager;
  }

  const glm::mat4 &getView2WorldMatrix()
  {
    return camera.getView2WorldMatrix();
  }

  const glm::mat4 &getWorld2ViewMatrix()
  {
    return camera.getWorld2ViewMatrix();
  }

  const glm::mat4 &getProjectionMatrixFar()
  {
    return camera.getProjectionMatrixFar();
  }

  const glm::vec3 &getCameraPos()
  {
    return camera.getPos();
  }

  float getFrameDelta()
  {
    return frame_delta;
  }

  void onTreeDrawn()
  {
    num_trees_drawn++;
  }


}
