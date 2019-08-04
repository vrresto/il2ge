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
#include <configuration.h>
#include <wgl_wrapper.h>
#include <config.h>

#include <render_util/state.h>
#include <render_util/shader_util.h>
#include <render_util/texture_manager.h>
#include <render_util/terrain_base.h>
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
#include <render_util/gl_binding/gl_functions.h>


using namespace core;
using namespace core_gl_wrapper;
using namespace render_util::gl_binding;
using namespace std;

using render_util::State;
using render_util::StateModifier;

#include <render_util/skybox.h>


namespace
{


void drawTerrain();


render_util::TerrainBase *getTerrain()
{
  return &core::getTerrain();
}


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


class TerrainClient : public render_util::TerrainBase::Client
{
  core_gl_wrapper::Context::Impl &m_context;
  const render_util::Camera &m_camera;
  const bool m_is_far_camera = 0;

  void setActiveProgram(render_util::ShaderProgramPtr p) override
  {
    m_context.setActiveShader(p);
    m_context.updateUniforms(p, m_camera, m_is_far_camera);
  }

public:
  TerrainClient(core_gl_wrapper::Context::Impl &context,
                const render_util::Camera &camera,
                bool is_far_camera) :
      m_context(context),
      m_camera(camera),
      m_is_far_camera(is_far_camera)
  {}
};


shared_ptr<Globals> g_globals;
unordered_map<string, void*> g_procs;
bool g_better_shadows = false;

#if ENABLE_SHORTCUTS
bool g_enable = true;
bool g_enable_object_shaders = true;
bool g_enable_terrain = true;
bool isEnabled() { return g_enable; }
bool isObjectShadersEnabled() { return g_enable && g_enable_object_shaders; }
bool isTerrainEnabled() { return g_enable && g_enable_terrain; }
#else
constexpr bool isEnabled() { return true; }
constexpr bool isObjectShadersEnabled() { return true; }
constexpr bool isTerrainEnabled() { return true; }
#endif


render_util::ShaderProgramPtr getRedProgram()
{
  auto ctx = core_gl_wrapper::getContext();
  if (!ctx->red_program)
  {
    ctx->red_program =
      render_util::createShaderProgram("red", core::textureManager(), core::getShaderSearchPath());
  }
  return ctx->red_program;
}


render_util::ShaderProgramPtr getInvisibleProgram()
{
  auto ctx = core_gl_wrapper::getContext();
  if (!ctx->invisible_program)
  {
    ctx->invisible_program =
      render_util::createShaderProgram("invisible", core::textureManager(), core::getShaderSearchPath());
  }
  return ctx->invisible_program;
}


render_util::ShaderProgramPtr getSkyProgram()
{
  auto ctx = core_gl_wrapper::getContext();
  if (!ctx->sky_program)
  {
    ctx->sky_program =
      render_util::createShaderProgram("sky", core::textureManager(),
                                       core::getShaderSearchPath(), {}, core::getShaderParameters());
  }
  return ctx->sky_program;
}

#if 0
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
#endif

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

  if (!wgl_wrapper::isMainContextCurrent() || core::isFMBActive() || !isEnabled())
  {
    return gl::Begin(mode);
  }

  getContext()->updateARBProgram();

  gl::Begin(mode);
}


void GLAPIENTRY wrap_glEnd()
{
  assert(wgl_wrapper::isMainThread());
  gl::End();
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

  if (wgl_wrapper::isMainContextCurrent())
  {
    getContext()->onObjectDraw();
  }

  gl::DrawElements(mode, count, type, indices);
  return;


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


// tree stem/shadow
void GLAPIENTRY wrap_glDrawArrays(GLenum mode,
    GLint first,
    GLsizei count)
{
  if (wgl_wrapper::isMainContextCurrent())
  {
    getContext()->onObjectDraw();
  }

  gl::DrawArrays(mode, first, count);
}


void GLAPIENTRY wrap_glDrawRangeElements(GLenum mode,
      GLuint start,
      GLuint end,
      GLsizei count,
      GLenum type,
      const GLvoid * indices)
{
  assert(wgl_wrapper::isMainThread());

  if (!wgl_wrapper::isMainContextCurrent())
  {
    gl::DrawRangeElements(mode, start, end, count, type, indices);
    return;
  }

  auto ctx = getContext();
  auto &state = ctx->getRenderState();

  ctx->onObjectDraw();

  if (core::isFMBActive()
      || !isTerrainEnabled()
      || state.is_mirror
      || state.camera_mode == IL2_CAMERA_MODE_2D
      || state.render_phase < IL2_Landscape0
      || state.render_phase >= IL2_PostLandscape
//         || state.render_phase < IL2_Landscape0_Finished
//       || true
    )
  {
    if (state.render_phase == IL2_Cockpit && ctx->active_shader)
    {
      bool blend = gl::IsEnabled(GL_BLEND);
      bool blend_add = false;

      if (blend)
      {
        GLenum src_alpha, dst_alpha;
        gl::GetIntegerv(GL_BLEND_SRC_ALPHA, (int*)&src_alpha);
        gl::GetIntegerv(GL_BLEND_DST_ALPHA, (int*)&dst_alpha);

        if (src_alpha == GL_SRC_ALPHA && dst_alpha == GL_ONE)
          blend_add = true;
      }

      ctx->active_shader->setUniform("blend_add", blend_add);
      ctx->active_shader->setUniform("blend", blend);
    }

    gl::DrawRangeElements(mode, start, end, count, type, indices);
  }
  else if (ctx->is_arb_program_active)
  {
    if (ctx->getARBProgramContext()->isObjectProgramActive())
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


void doDrawTerrain(render_util::TerrainBase &terrain,
                   const render_util::Camera &camera,
                   bool is_far_camera)
{
  TerrainClient client(*getContext(), camera, is_far_camera);

  terrain.update(camera, is_far_camera);
  terrain.draw(&client);
}


void doDrawTerrain(render_util::TerrainBase &terrain, StateModifier &state)
{
  terrain.setDrawDistance(0);

  auto z_far = core::getCamera()->getZFar();

  render_util::Camera far_camera(*core::getCamera());
  far_camera.setProjection(far_camera.getFov(), z_far - 4000, 1500000);


  state.setFrontFace(GL_CCW);

  doDrawTerrain(terrain, far_camera, true);

  gl::Clear(GL_DEPTH_BUFFER_BIT);

  doDrawTerrain(terrain, *core::getCamera(), false);

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

  getContext()->setActiveShader(nullptr);
}


void drawTerrain()
{
  if (g_better_shadows)
  {
    gl::ActiveTexture(GL_TEXTURE0 + TEXUNIT_SHADOW_COLOR);
    gl::BindTexture(GL_TEXTURE_2D, 0);
    gl::ActiveTexture(GL_TEXTURE0);
  }

  auto ctx = getContext();

  assert(ctx->getRenderState().camera_mode == IL2_CAMERA_MODE_3D);

  if (core::isFMBActive() || !isTerrainEnabled() || ctx->getRenderState().is_mirror)
    return;

  const auto original_state = State::fromCurrent();

  StateModifier state(original_state);
  state.setDefaults();

  assert(gl::IsEnabled(GL_COLOR_LOGIC_OP) == GL_FALSE);

  ctx->updateFramebufferTextureSize();

  texture_state::freeze();
  core::textureManager().setActive(true);

  if (g_better_shadows)
  {
    gl::BindFramebuffer(GL_FRAMEBUFFER, ctx->framebuffer_id);

    gl::FramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, ctx->framebuffer_depth_texture->getID(), 0);
    gl::FramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, ctx->framebuffer_texture0->getID(), 0);
    gl::FramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, ctx->framebuffer_texture1->getID(), 0);

    const GLuint drawBuffers[2] =
    {
      GL_COLOR_ATTACHMENT0,
      GL_COLOR_ATTACHMENT1,
    };

    gl::DrawBuffers(2, drawBuffers);
    gl::ReadBuffer(GL_COLOR_ATTACHMENT0);
  }

//   gl::Clear(GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  gl::Clear(GL_DEPTH_BUFFER_BIT);

  ctx->setActiveShader(getSkyProgram());
  ctx->updateUniforms(getSkyProgram());

  render_util::drawSkyBox();

  gl::Clear(GL_DEPTH_BUFFER_BIT);

//   gl::PolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  doDrawTerrain(core::getTerrain(), state);
//   gl::PolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  ctx->setActiveShader(nullptr);

  core::textureManager().setActive(false);
  texture_state::restore();

  if (g_better_shadows)
  {
    gl::ActiveTexture(GL_TEXTURE0 + TEXUNIT_SHADOW_COLOR);
    gl::BindTexture(GL_TEXTURE_2D, ctx->framebuffer_texture1->getID());
    gl::ActiveTexture(GL_TEXTURE0);
  }

}


} // namespace


namespace core_gl_wrapper
{


void setProc(const char *name, void *func)
{
  g_procs[name] = func;
}


void *getProc(const char *name)
{
  return g_procs[name];
}


#define SET_PROC(name) setProc(#name, (void*) &wrap_##name)

void init()
{
  g_globals = make_shared<Globals>();

#if ENABLE_CONFIGURABLE_SHADOWS
  g_better_shadows = il2ge::core_wrapper::getConfig().better_shadows;
#endif

  #if 1
//     setProc("glOrtho", (void*) &wrap_glOrtho);
//     setProc("glCallList", (void*) &wrap_glCallList);
//     setProc("glCallLists", (void*) &wrap_glCallLists);

  setProc("glBegin", (void*) &wrap_glBegin);
  setProc("glEnd", (void*) &wrap_glEnd);
//   setProc("glClear", (void*) &wrap_glClear);
  setProc("glViewport", (void*) &wrap_glViewport);
//   setProc("glTexImage2D", (void*) &wrap_glTexImage2D);

  SET_PROC(glDrawElements);
  SET_PROC(glDrawArrays);
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


void Context::Impl::updateShaderState()
{
  render_util::ShaderProgramPtr new_active_shader;

  if (current_shader)
  {
    new_active_shader = current_shader;
  }
  else if (isObjectShadersEnabled() && is_arb_program_active && !core::isFMBActive())
  {
    new_active_shader = current_arb_program;
  }

  if (new_active_shader)
  {
    assert(new_active_shader->isValid());
//       if (new_active_shader != active_shader)
//       {
      render_util::getCurrentGLContext()->setCurrentProgram(new_active_shader);
      active_shader = new_active_shader;
//       }
  }
  else
  {
    render_util::getCurrentGLContext()->setCurrentProgram(nullptr);
    active_shader = nullptr;
  }
}


Context::Context() : impl(make_unique<Context::Impl>()) {}
Context::~Context() {}

Context::Impl::Impl()
{
  FORCE_CHECK_GL_ERROR();
  gl::GenFramebuffers(1, &framebuffer_id);
  FORCE_CHECK_GL_ERROR();
}

Context::Impl::~Impl()
{
  if (framebuffer_id)
    gl::DeleteFramebuffers(1, &framebuffer_id);
}


void Context::Impl::updateFramebufferTextureSize()
{
  if (!g_better_shadows)
    return;

  glm::ivec2 viewport_size(m_viewport_w, m_viewport_h);
  assert(viewport_size != glm::ivec2(0));

  if (viewport_size == m_framebuffer_texture_size)
    return;

  {
    if (!framebuffer_depth_texture)
      framebuffer_depth_texture = render_util::Texture::create(GL_TEXTURE_2D);
    render_util::TemporaryTextureBinding binding(framebuffer_depth_texture);
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl::TexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, viewport_size.x, viewport_size.y,
                  0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
  }

  {
    if (!framebuffer_texture0)
      framebuffer_texture0 = render_util::Texture::create(GL_TEXTURE_2D);
    render_util::TemporaryTextureBinding binding(framebuffer_texture0);
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl::TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport_size.x, viewport_size.y,
                  0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  }

  {
    if (!framebuffer_texture1)
      framebuffer_texture1 = render_util::Texture::create(GL_TEXTURE_2D);
    render_util::TemporaryTextureBinding binding(framebuffer_texture1);
    gl::TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl::TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, viewport_size.x, viewport_size.y,
                  0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  }

  m_framebuffer_texture_size = viewport_size;
}


void Context::Impl::onRenderPhaseChanged(const core::Il2RenderState &new_state)
{
  assert(wgl_wrapper::isMainContextCurrent());

  drawTerrainIfNeccessary();

  m_render_state = new_state;
  m_was_terrain_drawn = false;

  switch (new_state.render_phase)
  {
    case core::IL2_PrePreRenders:
      m_frame_nr++;
      break;
    case core::IL2_Landscape0:
      if (!core::isFMBActive() && isEnabled())
      {
        gl::ActiveTexture(GL_TEXTURE0 + TEXUNIT_TERRAIN_NORMAL_MAP);

        auto terrain = &core::getTerrain();
        assert(terrain);
        if (terrain)
        {
          auto texture = terrain->getNormalMapTexture();
          assert(texture);
          if (texture)
            gl::BindTexture(texture->getTarget(), texture->getID());
        }

        gl::ActiveTexture(GL_TEXTURE0);
      }
      break;
    case core::IL2_PostLandscape:
      onLandscapeFinished();
      break;
  }

  getARBProgramContext()->onRenderPhaseChanged(new_state.render_phase);
}


void Context::Impl::onLandscapeFinished()
{
  using namespace render_util::gl_binding;

  if (core::isFMBActive() || !isEnabled())
    return;

  if (!g_better_shadows)
    return;

  auto viewport_size = getViewportSize();

  gl::BindFramebuffer(GL_FRAMEBUFFER, 0);
  gl::BindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer_id);

  //FIXME - this is possibly slow
  gl::BlitFramebuffer(0, 0, viewport_size.x, viewport_size.y,
                      0, 0, viewport_size.x, viewport_size.y,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

  gl::BindFramebuffer(GL_FRAMEBUFFER, framebuffer_id);
  gl::FramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,0, 0);
  gl::FramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 0, 0);
  gl::FramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, 0, 0);

  gl::BindFramebuffer(GL_FRAMEBUFFER, 0);

  assert(framebuffer_texture1);
  gl::ActiveTexture(GL_TEXTURE0 + TEXUNIT_SHADOW_COLOR);
  gl::BindTexture(GL_TEXTURE_2D, framebuffer_texture1->getID());
  gl::ActiveTexture(GL_TEXTURE0);
}


void Context::Impl::onObjectDraw()
{
  updateARBProgram();
  drawTerrainIfNeccessary();
}


void Context::Impl::drawTerrainIfNeccessary()
{
  if (m_render_state.render_phase == IL2_Landscape0 && !m_was_terrain_drawn)
  {
    drawTerrain();
    m_was_terrain_drawn = true;
  }
}


void Context::Impl::updateARBProgram()
{
  getARBProgramContext()->update();
}


void onRenderPhaseChanged(const core::Il2RenderState &new_state)
{
  getContext()->onRenderPhaseChanged(new_state);
}


#if ENABLE_SHORTCUTS
void toggleEnable()
{
  g_enable = !g_enable;
}

void toggleObjectShaders()
{
  g_enable_object_shaders = !g_enable_object_shaders;
}

void toggleTerrain()
{
  g_enable_terrain = !g_enable_terrain;
}
#endif


void setShader(render_util::ShaderProgramPtr shader)
{
  getContext()->setActiveShader(shader);
}


void updateUniforms(render_util::ShaderProgramPtr shader)
{
  getContext()->updateUniforms(shader);
}


} // namespace core_gl_wrapper
