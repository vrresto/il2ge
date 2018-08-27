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

#include "gl_wrapper.h"
#include "gl_wrapper_private.h"
#include "misc.h"
#include "core.h"
#include <wgl_wrapper.h>

#include <render_util/shader_util.h>
#include <render_util/texture_manager.h>
#include <render_util/image.h>
#include <render_util/image_loader.h>
#include <render_util/texunits.h>

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <GL/gl.h>

#include "wgl_interface.h"
#include <gl_wrapper/gl_functions.h>


using namespace core;
using namespace gl_wrapper::gl_functions;
using namespace std;

#include <render_util/skybox.h>


namespace core_gl_wrapper
{
  void drawTerrain();
  void arbProgramInit();
  void updateUniforms(render_util::ShaderProgramPtr program);
}


namespace
{


const std::string SHADER_PATH = IL2GE_DATA_DIR "/shaders";

unordered_map<string, void*> g_procs;
int g_viewport_w = 0;
int g_viewport_h = 0;
//   unordered_set<string> g_forest_shader_names;



render_util::ShaderProgramPtr getRedProgram()
{
  core_gl_wrapper::Context *ctx = core_gl_wrapper::getContext();
  if (!ctx->red_program)
    ctx->red_program = render_util::createShaderProgram("red", core::textureManager(), SHADER_PATH);
  return ctx->red_program;
}


render_util::ShaderProgramPtr getInvisibleProgram()
{
  core_gl_wrapper::Context *ctx = core_gl_wrapper::getContext();
  if (!ctx->invisible_program)
    ctx->invisible_program = render_util::createShaderProgram("invisible", core::textureManager(), SHADER_PATH);
  return ctx->invisible_program;
}


render_util::ShaderProgramPtr getSkyProgram()
{
  core_gl_wrapper::Context *ctx = core_gl_wrapper::getContext();
  if (!ctx->sky_program)
    ctx->sky_program = render_util::createSkyProgram(core::textureManager(), SHADER_PATH);
  return ctx->sky_program;
}


render_util::ShaderProgramPtr getTreeProgram()
{
  core_gl_wrapper::Context *ctx = core_gl_wrapper::getContext();
  if (!ctx->tree_program)
  {
    ctx->tree_program = render_util::createShaderProgram("tree", core::textureManager(), SHADER_PATH);

    for (int i = 0; i < 20; i++)
    {
      char tmp[100];
      snprintf(tmp, sizeof(tmp), "sampler_%d", i);
      ctx->tree_program->setUniformi(tmp, i);
    }
  }

  return ctx->tree_program;
}


render_util::ShaderProgramPtr getTerrainProgram()
{
  core_gl_wrapper::Context *ctx = core_gl_wrapper::getContext();
  if (!ctx->terrain_program)
  {
    CHECK_GL_ERROR();

    map<unsigned int, string> attribute_locations = { { 4, "attrib_pos" } };

    ctx->terrain_program = render_util::createShaderProgram("terrain_cdlod",
                                                        core::textureManager(),
                                                        SHADER_PATH,
                                                        attribute_locations);
    CHECK_GL_ERROR();
  }
  return ctx->terrain_program;
}


render_util::ShaderProgramPtr getForestProgram()
{
  core_gl_wrapper::Context *ctx = core_gl_wrapper::getContext();
  if (!ctx->forest_program)
  {
    CHECK_GL_ERROR();
    ctx->forest_program = render_util::createShaderProgram("forest_cdlod", core::textureManager(), SHADER_PATH);
    CHECK_GL_ERROR();
  }
  return ctx->forest_program;
}


/////////////////////////////////////////


void GLAPIENTRY wrap_glCallLists(GLsizei n,  GLenum type,  const GLvoid * lists)
{
  assert(0);
}


void GLAPIENTRY wrap_glCallList(GLuint list)
{
  assert(0);
}


////////////////////////////////////////////


void GLAPIENTRY wrap_glClear(GLbitfield mask)
{
  assert(wgl_wrapper::isMainThread());

  gl::Clear(mask);

//     onClear();

  if (mask & GL_COLOR_BUFFER_BIT && wgl_wrapper::isMainContextCurrent())
  {
    onClear();
//       is_skybox_finished = false;
  }
}


void GLAPIENTRY wrap_glViewport(GLint x,  GLint y,  GLsizei width,  GLsizei height)
{
  assert(wgl_wrapper::isMainThread());
  assert(wgl_wrapper::isMainContextCurrent());

  gl::Viewport(x, y, width, height);

  g_viewport_w = width;
  g_viewport_h = height;

  Il2RenderState state;
  getRenderState(&state);

//     cout<<"glViewport: "<<width<<"x"<<height<<" phase: "<<state.render_phase<<endl;

  if (state.render_phase == IL2_Landscape0_PreTerrain)
  {
    if (width == 256 && height == 256)
    {
//         onCubeMapBegin();
    }
  }

}


//   void GLAPIENTRY wrap_glOrtho(GLdouble left,
//                     GLdouble right,
//                     GLdouble bottom,
//                     GLdouble top,
//                     GLdouble nearVal,
//                     GLdouble farVal)
//   {
//     gl::Ortho(left, right, bottom, top, nearVal, farVal);
//
//     Il2RenderState state;
//     getRenderState(&state);
//
//     if (state.render_phase == IL2_Landscape0_PreTerrain)
//     {
//     }
//   }


void GLAPIENTRY wrap_glTexImage2D(GLenum target,
  GLint level,
  GLint internalFormat,
  GLsizei width,
  GLsizei height,
  GLint border,
  GLenum format,
  GLenum type,
  const GLvoid* data)
{
  assert(wgl_wrapper::isMainThread());
  assert(wgl_wrapper::isMainContextCurrent());

  gl::TexImage2D(target, level, internalFormat, width, height, border, format, type, data);

  Il2RenderState state;
  getRenderState(&state);

  switch (target)
  {
    case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
    case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
    case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
      cout<<"wrap_glTexImage2D: "<<width<<"x"<<height<<endl;
      if (state.render_phase == IL2_Landscape0_CubeMap)
      {
        assert(0);
        onCubeMapFaceFinished();
      }
      break;
    default:
      break;
  }
}


void GLAPIENTRY wrap_glBegin(GLenum mode)
{
  assert(wgl_wrapper::isMainThread());

  if (!wgl_wrapper::isMainContextCurrent())
  {
    return gl::Begin(mode);
  }

  {
    Il2RenderState state;
    getRenderState(&state);

    core_gl_wrapper::updateARBProgram();

    bool is_cubemap = (g_viewport_w == 256 && g_viewport_h == 256);

//       if (gl::IsEnabled(GL_DEPTH_TEST))
//       {
//         is_skybox_finished = false;
//       }
//       else if (state.render_phase < IL2_Landscape0_Finished && !is_skybox_finished)
//       {
//           core_gl_wrapper::drawTerrain();
//           onFarTerrainDone();
//           is_skybox_finished = true;
//       }

    if (
        !is_cubemap &&
        state.render_phase > IL2_Landscape0_CubeMap &&
        state.render_phase < IL2_Landscape0_Finished &&
        state.num_rendered_objects == 0
        )
    {
//       cout << "is cube updated: " << Core::isCubeUpdated() <<endl;
//         if (!core::isCubeUpdated()) {
        core_gl_wrapper::drawTerrain();
        onFarTerrainDone();
//         }
    }


    if (state.render_phase >= IL2_Landscape0_PreTerrain &&
        state.render_phase < IL2_Landscape0_Finished
//           && state.num_rendered_objects < 6
//             || true
        )
    {
//         core_gl_wrapper::setActiveShader(getSkyProgram());
//         updateUniforms(getSkyProgram());
//       }
//       else {
      core_gl_wrapper::setActiveShader(getInvisibleProgram());
//         core_gl_wrapper::setActiveShader(getRedProgram());
    }
  }

//     discardGlCalls(true);

//     core_gl_wrapper::setActiveShader(getRedProgram());
//     gl::PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  gl::Begin(mode);
}


void GLAPIENTRY wrap_glEnd()
{
  assert(wgl_wrapper::isMainThread());

  gl::End();

  if (!wgl_wrapper::isMainContextCurrent())
  {
    return;
  }

//     gl::PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

//     discardGlCalls(false);

//     if (!is_arb_program_active())
//       gl::UseProgram(0);

  core_gl_wrapper::setActiveShader(nullptr);

  onObjectRendered();

//     Il2RenderState state;
//     getRenderState(&state);
//     if (
//         state.render_phase < IL2_Landscape0_Finished
//         &&
// //         state.num_rendered_objects == 6
//         state.num_rendered_objects == 0
// //         && !isARBProgramActive()
//        ) {
// //       cout << "is cube updated: " << Core::isCubeUpdated() <<endl;
// //       if (!Core::isCubeUpdated())
//         core_gl_wrapper::drawTerrain();
//         onFarTerrainDone();
//     }

}


#if 1
// trees
void GLAPIENTRY wrap_glDrawElements(
    GLenum mode,
    GLsizei count,
    GLenum type,
    const GLvoid * indices)
{
  assert(wgl_wrapper::isMainThread());
  assert(wgl_wrapper::isMainContextCurrent());

  gl::GetError();
//   	return;

//     core_gl_wrapper::setActiveShader(getTreeProgram());
#if 0
  {
    GLint values[4];
    gl::GetIntegerv(GL_SCISSOR_BOX, values);
    gl::Enable(GL_SCISSOR_TEST);
    gl::Scissor(0, 0, 900, values[3]);

    gl::UseProgram(0);

//       gl::DrawElements(mode, count, type, indices);

    gl::Scissor(values[0], values[1], values[2], values[3]);
    gl::Disable(GL_SCISSOR_TEST);
  }
  CHECK_GL_ERROR();
#endif

  core_gl_wrapper::updateARBProgram();

  CHECK_GL_ERROR();

//     assert(!isARBProgramActive());
//
//     cout<<"mode: "<<mode<<endl;
//     cout<<"count: "<<count<<endl;

//     gl::PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//     gl::PolygonMode(GL_FRONT_AND_BACK, GL_POINT);

  gl::PointSize(1);

//     gl::DrawElements(mode, count, type, indices);
//     gl::DrawElements(GL_POINTS, count, type, indices);
//     gl::DrawElements(GL_LINES, count, type, indices);

  {
    GLint values[4];
    gl::GetIntegerv(GL_SCISSOR_BOX, values);
    gl::Enable(GL_SCISSOR_TEST);
//       gl::Scissor(900, 0, values[2], values[3]);

    gl::DrawElements(mode, count, type, indices);

    gl::Scissor(values[0], values[1], values[2], values[3]);
    gl::Disable(GL_SCISSOR_TEST);
  }

  CHECK_GL_ERROR();


  gl::PointSize(1);

//     gl::PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  core_gl_wrapper::setActiveShader(nullptr);
}


void GLAPIENTRY wrap_glDrawArrays(GLenum mode,
    GLint first,
    GLsizei count)
{
}


void GLAPIENTRY wrap_glDrawRangeElements(GLenum mode,
      GLuint start,
      GLuint end,
      GLsizei count,
      GLenum type,
      const GLvoid * indices)
{
  assert(wgl_wrapper::isMainThread());
  assert(wgl_wrapper::isMainContextCurrent());

  Il2RenderState state;
  getRenderState(&state);

  core_gl_wrapper::updateARBProgram();

  if (state.camera_mode == IL2_CAMERA_MODE_2D
      || state.render_phase < IL2_Landscape0
      || state.render_phase >= IL2_PostLandscape
//         || state.render_phase < IL2_Landscape0_Finished
//       || true
    )
  {
    gl::DrawRangeElements(mode, start, end, count, type, indices);
  }
  else if (core_gl_wrapper::isARBProgramActive())
  {

    const string name = core_gl_wrapper::getFragmentProgramName();

    if (name == "fpObjectsL0_2L" || name == "fpObjectsL0")
    {
      gl::DrawRangeElements(mode, start, end, count, type, indices);
    }

#if 0
    GLint values[4];
    gl::GetIntegerv(GL_SCISSOR_BOX, values);
    gl::Enable(GL_SCISSOR_TEST);
    gl::Scissor(0, 0, 900, values[3]);


    const string name = core_gl_wrapper::getFragmentProgramName();
    assert(!name.empty());
    if (g_forest_shader_names.find(name) != g_forest_shader_names.end() || false)
    {

      gl::DrawRangeElements(mode, start, end, count, type, indices);

    }

    gl::Scissor(values[0], values[1], values[2], values[3]);
    gl::Disable(GL_SCISSOR_TEST);
#endif
  }

//     if (state.render_phase == IL2_Landscape0_PreTerrain &&
//         currentFragmentProgramName() &&
//         strcmp(currentFragmentProgramName(), "fpFarLandFog") == 0)
//     {
//       setRenderPhase(IL2_Landscape0_TerrainFar);
//     }


}
#endif


void updateShaderState()
{
  auto *ctx = core_gl_wrapper::getContext();

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


} // namespace


namespace core_gl_wrapper
{


void updateUniforms(render_util::ShaderProgramPtr program)
{
  core::updateUniforms(program);
}


inline void doDrawTerrain()
{
#if 1
  texture_state::freeze();
  core::textureManager().setActive(true);

//     gl::PolygonMode(GL_FRONT_AND_BACK, GL_LINE);

//     assert(!isARBProgramActive());

//     Core::bindGroundTexture();

  CHECK_GL_ERROR();


  core_gl_wrapper::setActiveShader(getSkyProgram());
  updateUniforms(getSkyProgram());

//     gl::Disable(GL_DEPTH_TEST);

//     glDepthMask(GL_FALSE);
  gl::FrontFace(GL_CW);
  gl::PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  render_util::drawSkyBox();

  core_gl_wrapper::setActiveShader(getTerrainProgram());

  CHECK_GL_ERROR();

  core::setTerrainDrawDistance(0);
  core::updateTerrain();

  updateUniforms(getTerrainProgram());

  gl::DepthMask(true);

//     gl::Enable(GL_DEPTH_TEST);
  gl::FrontFace(GL_CCW);
  gl::CullFace(GL_BACK);
  gl::Enable(GL_CULL_FACE);
  gl::DepthFunc(GL_LEQUAL);

//     gl::Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
//     gl::Clear(GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

//     Core::drawFarTerrain();

//     GLint loc = gl::GetUniformLocation(getTerrainProgram(), "terrainColor");
//     assert(loc != -1);
//     gl::Uniform4f(loc, 0.0, 0.0, 0.0, 0.7);
//     gl::PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//     gl::Clear(GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
//     Core::drawFarTerrain();

//     gl::Uniform4f(loc, 1.0, 0.0, 0.0, 0.0);

  gl::Clear(GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

  CHECK_GL_ERROR();

  core::drawTerrain(getTerrainProgram());

  CHECK_GL_ERROR();

#if 0
  const int forest_layers = 5;
  const int forest_layer_repetitions = 1;
  const float forest_layer_height = 2.5;

  render_util::ShaderProgramPtr forest_program = getForestProgram();

  core::setTerrainDrawDistance(5000.f);
  core::updateTerrain();

  core_gl_wrapper::setActiveShader(forest_program);
  updateUniforms(forest_program);

  CHECK_GL_ERROR();

  gl::Enable(GL_BLEND);
  gl::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  gl::Disable(GL_CULL_FACE);

  CHECK_GL_ERROR();

  for (int i = 1; i < forest_layers; i++)
  {
    forest_program->setUniformi("forest_layer", i);

    float height = i * forest_layer_height + forest_layer_height;
    forest_program->setUniform("terrain_height_offset", height);
    core::drawTerrain(forest_program);
    CHECK_GL_ERROR();

  }

  gl::Enable(GL_CULL_FACE);
  gl::Disable(GL_BLEND);
#endif

//     gl::PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//     Core::drawNearTerrain();


  gl::Clear(GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

//     gl::DepthFunc(GL_ALWAYS);
//     gl::PolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  gl::Disable(GL_CULL_FACE);
  gl::DepthMask(false);

  CHECK_GL_ERROR();

  core_gl_wrapper::setActiveShader(nullptr);

  CHECK_GL_ERROR();

  core::textureManager().setActive(false);
  texture_state::restore();
#endif
}


void drawTerrain()
{
//     GLint values[4];
//     gl::GetIntegerv(GL_SCISSOR_BOX, values);
//     gl::Enable(GL_SCISSOR_TEST);
//     gl::Scissor(900, 0, 900, values[3]);

  doDrawTerrain();

//    gl::Finish();

//     gl::Scissor(values[0], values[1], values[2], values[3]);
//     gl::Disable(GL_SCISSOR_TEST);
}


void setProc(const char *name, void *func)
{
  g_procs[name] = func;
}


void *getProc(const char *name)
{
  return g_procs[name];
}


// render_util::ShaderProgramPtr activeShader()
// {
//   assert(false);
//   abort();
// }


void setActiveShader(render_util::ShaderProgramPtr shader)
{
  getContext()->current_shader = shader;
  updateShaderState();
}


// render_util::ShaderProgramPtr activeARBProgram()
// {
//   assert(false);
//   abort();
// }


void setActiveARBProgram(render_util::ShaderProgramPtr prog)
{
  getContext()->current_arb_program = prog;
  updateShaderState();
}


void setIsARBProgramActive(bool active)
{
  getContext()->is_arb_program_active = active;
  updateShaderState();
}


bool isARBProgramActive()
{
  return getContext()->is_arb_program_active;
}


Context *getContext()
{
  auto *context = wgl_wrapper::getContext()->getSubModule<Context>();

  if (!context)
  {
    context = new Context;
    wgl_wrapper::getContext()->setSubModule(context);
  }

  return context;
}


#define SET_PROC(name) setProc(#name, (void*) &wrap_##name)

void init()
{
  #if 1
//     setProc("glOrtho", (void*) &wrap_glOrtho);
//     setProc("glCallList", (void*) &wrap_glCallList);
//     setProc("glCallLists", (void*) &wrap_glCallLists);

  setProc("glBegin", (void*) &wrap_glBegin);
  setProc("glEnd", (void*) &wrap_glEnd);
  setProc("glClear", (void*) &wrap_glClear);
  setProc("glViewport", (void*) &wrap_glViewport);
  setProc("glTexImage2D", (void*) &wrap_glTexImage2D);

  SET_PROC(glDrawElements);
  SET_PROC(glDrawArrays);
  SET_PROC(glDrawRangeElements);
  setProc("glDrawRangeElementsEXT", (void*) wrap_glDrawRangeElements);
  #endif

  arbProgramInit();

//   g_forest_shader_names.insert("fpForestPlane");
//   g_forest_shader_names.insert("fpForestPlaneNoise");
//   g_forest_shader_names.insert("fpForestPlaneEdges");
//   g_forest_shader_names.insert("fpForestPlaneEdgesNoise");
}


} // namespace core_gl_wrapper
