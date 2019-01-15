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

#include <render_util/render_util.h>
#include <render_util/shader_util.h>
#include <render_util/texture_manager.h>
#include <render_util/image.h>
#include <render_util/image_loader.h>
#include <render_util/texunits.h>
#include <render_util/terrain_util.h>
#include <render_util/globals.h>

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
using namespace core_gl_wrapper;
using namespace gl_wrapper::gl_functions;
using namespace std;

#include <render_util/skybox.h>



namespace
{


void drawTerrain();


class Globals : public render_util::Globals
{
public:
  std::shared_ptr<render_util::GLContext> getCurrentGLContext() override
  {
    auto context = core_gl_wrapper::getContext();
    if (!context->render_util_gl_context)
      context->render_util_gl_context = make_shared<render_util::GLContext>();

    return context->render_util_gl_context;
  }
};


const std::string SHADER_PATH = IL2GE_DATA_DIR "/shaders";

shared_ptr<Globals> g_globals;
unordered_map<string, void*> g_procs;
//   unordered_set<string> g_forest_shader_names;


render_util::ShaderProgramPtr getRedProgram()
{
  auto ctx = core_gl_wrapper::getContext();
  if (!ctx->red_program)
    ctx->red_program = render_util::createShaderProgram("red", core::textureManager(), SHADER_PATH);
  return ctx->red_program;
}


render_util::ShaderProgramPtr getInvisibleProgram()
{
  auto ctx = core_gl_wrapper::getContext();
  if (!ctx->invisible_program)
    ctx->invisible_program = render_util::createShaderProgram("invisible", core::textureManager(), SHADER_PATH);
  return ctx->invisible_program;
}


render_util::ShaderProgramPtr getSkyProgram()
{
  auto ctx = core_gl_wrapper::getContext();
  if (!ctx->sky_program)
    ctx->sky_program = render_util::createSkyProgram(core::textureManager(), SHADER_PATH);
  return ctx->sky_program;
}


render_util::ShaderProgramPtr getTreeProgram()
{
  auto ctx = core_gl_wrapper::getContext();
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


render_util::ShaderProgramPtr getForestProgram()
{
  auto ctx = core_gl_wrapper::getContext();
  if (!ctx->forest_program)
  {
    CHECK_GL_ERROR();

    map<unsigned int, string> attribute_locations = { { 4, "attrib_pos" } };

    ctx->forest_program = render_util::createShaderProgram("forest_cdlod", core::textureManager(), SHADER_PATH, attribute_locations);
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


// void GLAPIENTRY wrap_glClear(GLbitfield mask)
// {
//   assert(wgl_wrapper::isMainThread());
//
//   if (mask & GL_COLOR_BUFFER_BIT && wgl_wrapper::isMainContextCurrent())
//   {
//     getContext()->onClear();
//   }
//
//   gl::Clear(mask);
// }


void GLAPIENTRY wrap_glViewport(GLint x,  GLint y,  GLsizei width,  GLsizei height)
{
  assert(wgl_wrapper::isMainThread());

  gl::Viewport(x, y, width, height);

  if (wgl_wrapper::isMainContextCurrent())
  {
    getContext()->setViewport(width, height);
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


// void GLAPIENTRY wrap_glTexImage2D(GLenum target,
//   GLint level,
//   GLint internalFormat,
//   GLsizei width,
//   GLsizei height,
//   GLint border,
//   GLenum format,
//   GLenum type,
//   const GLvoid* data)
// {
//   assert(wgl_wrapper::isMainThread());
//   assert(wgl_wrapper::isMainContextCurrent());
//
//   gl::TexImage2D(target, level, internalFormat, width, height, border, format, type, data);
//
//   Il2RenderState state;
//   getRenderState(&state);
//
//   switch (target)
//   {
//     case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
//     case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
//     case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
//     case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
//     case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
//     case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
// //       cout<<"wrap_glTexImage2D: "<<width<<"x"<<height<<endl;
// //       if (state.render_phase == IL2_Landscape0_CubeMap)
// //       {
// //         assert(0);
// // //         onCubeMapFaceFinished();
// //       }
// //       break;
//     default:
//       break;
//   }
// }


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


    if (state.render_phase == IL2_Landscape0 &&
        !getContext()->wasTerrainDrawn() &&
        !getContext()->isRenderingCubeMap())
    {
      drawTerrain();
      getContext()->onTerrainDrawn();
    }


    if (state.render_phase == IL2_Landscape0)
    {
      core_gl_wrapper::setActiveShader(getInvisibleProgram());
    }
  }

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

//   core_gl_wrapper::setActiveShader(nullptr);
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
    if (core_gl_wrapper::arb_program::isObjectProgramActive())
    {
      gl::DrawRangeElements(mode, start, end, count, type, indices);
    }

#if 0
    GLint values[4];
    gl::GetIntegerv(GL_SCISSOR_BOX, values);
    gl::Enable(GL_SCISSOR_TEST);
    gl::Scissor(0, 0, 900, values[3]);


    const string name = core_gl_wrapper::arb_program::getFragmentProgramName();
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
      render_util::getCurrentGLContext()->setCurrentProgram(new_active_shader);
      ctx->active_shader = new_active_shader;
//       }
  }
  else
  {
    render_util::getCurrentGLContext()->setCurrentProgram(nullptr);
    ctx->active_shader = nullptr;
  }
}


void updateUniforms(render_util::ShaderProgramPtr program, const render_util::Camera &camera)
{
  core::updateUniforms(program);
  render_util::updateUniforms(program, camera);
}


void doDrawTerrain(render_util::TerrainRenderer &renderer,
                   const render_util::Camera &camera,
                   bool enable_details)
{
  auto program = renderer.getProgram();

  ::updateUniforms(program, camera);

  program->setUniform("draw_near_forest", enable_details);
  program->setUniform("enable_waves", enable_details);
  program->setUniform("enable_terrain_noise", enable_details);

  renderer.getTerrain()->update(camera);
  renderer.getTerrain()->draw();
}


void doDrawTerrain(render_util::TerrainRenderer &renderer)
{
  renderer.getTerrain()->setDrawDistance(0);

  core_gl_wrapper::setActiveShader(renderer.getProgram());

  auto z_far = core::getCamera()->getZFar();

  render_util::Camera far_camera(*core::getCamera());
  far_camera.setProjection(far_camera.getFov(), z_far - 4000, 1500000);

  int dept_func_save;
  gl::GetIntegerv(GL_DEPTH_FUNC, &dept_func_save);

  gl::FrontFace(GL_CCW);
  gl::DepthFunc(GL_LEQUAL);

  doDrawTerrain(renderer, far_camera, false);

  gl::Clear(GL_DEPTH_BUFFER_BIT);

  doDrawTerrain(renderer, *core::getCamera(), true);

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

  gl::DepthFunc(dept_func_save);

  core_gl_wrapper::setActiveShader(nullptr);
}


void drawTerrain()
{
  texture_state::freeze();
  core::textureManager().setActive(true);

  int front_face_save;
  gl::GetIntegerv(GL_FRONT_FACE, &front_face_save);

  gl::DepthMask(true);
//   gl::Clear(GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  gl::Clear(GL_DEPTH_BUFFER_BIT);
  gl::FrontFace(GL_CW);
  gl::CullFace(GL_BACK);
//     gl::Disable(GL_DEPTH_TEST);
//     glDepthMask(GL_FALSE);
  gl::Enable(GL_CULL_FACE);

  core_gl_wrapper::setActiveShader(getSkyProgram());
  core_gl_wrapper::updateUniforms(getSkyProgram());

  render_util::drawSkyBox();

  gl::Clear(GL_DEPTH_BUFFER_BIT);

//   gl::PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  doDrawTerrain(core::getTerrainRenderer());
//   gl::PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  core_gl_wrapper::setActiveShader(nullptr);

  gl::FrontFace(front_face_save);
  gl::Disable(GL_CULL_FACE);
  gl::DepthMask(false);

  core::textureManager().setActive(false);
  texture_state::restore();
}


} // namespace


namespace core_gl_wrapper
{


void updateUniforms(render_util::ShaderProgramPtr program)
{
  ::updateUniforms(program, *core::getCamera());
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


Context::Impl *getContext()
{
  return wgl_wrapper::getContext()->getGLWrapperContext()->getImpl();
}


#define SET_PROC(name) setProc(#name, (void*) &wrap_##name)

void init()
{
  g_globals = make_shared<Globals>();

  #if 1
//     setProc("glOrtho", (void*) &wrap_glOrtho);
//     setProc("glCallList", (void*) &wrap_glCallList);
//     setProc("glCallLists", (void*) &wrap_glCallLists);

  setProc("glBegin", (void*) &wrap_glBegin);
  setProc("glEnd", (void*) &wrap_glEnd);
//   setProc("glClear", (void*) &wrap_glClear);
  setProc("glViewport", (void*) &wrap_glViewport);
//   setProc("glTexImage2D", (void*) &wrap_glTexImage2D);

//   SET_PROC(glDrawElements);
//   SET_PROC(glDrawArrays);
  SET_PROC(glDrawRangeElements);
  setProc("glDrawRangeElementsEXT", (void*) wrap_glDrawRangeElements);
  #endif

  texture_state::init();
  arb_program::init();

//   g_forest_shader_names.insert("fpForestPlane");
//   g_forest_shader_names.insert("fpForestPlaneNoise");
//   g_forest_shader_names.insert("fpForestPlaneEdges");
//   g_forest_shader_names.insert("fpForestPlaneEdgesNoise");
}


texture_state::TextureState *Context::Impl::getTextureState()
{
  if (!m_texture_state)
    m_texture_state = make_unique<texture_state::TextureState>();

  return m_texture_state.get();
}


Context::Context() : impl(make_unique<Context::Impl>()) {}
Context::~Context() {}

Context::Impl::Impl() {}
Context::Impl::~Impl() {}

arb_program::Context *Context::Impl::getARBProgramContext()
{
  if (!m_arb_program_context)
    m_arb_program_context = std::make_unique<arb_program::Context>();

  return m_arb_program_context.get();
}


void onRenderPhaseChanged(const core::Il2RenderState &state)
{
  getContext()->onRenderPhaseChanged(state);
}


} // namespace core_gl_wrapper
