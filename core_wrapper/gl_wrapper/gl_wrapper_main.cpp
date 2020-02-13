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
#include <render_util/state.h>

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


#if ENABLE_SHORTCUTS
  #define SHORTCUT_OPTION
#else
  #define SHORTCUT_OPTION constexpr
#endif


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
#if ENABLE_CONFIGURABLE_SHADOWS
bool g_better_shadows = false;
#else
constexpr bool g_better_shadows = false;
#endif
bool g_enable_cirrus_clouds = false;
bool g_enable_transparent_shader = true;

SHORTCUT_OPTION bool g_enable = true;
SHORTCUT_OPTION bool g_enable_object_shaders = true;
SHORTCUT_OPTION bool g_enable_terrain = true;

bool isActive() { return g_enable && !core::isFMBActive(); }
bool isTerrainEnabled() { return isActive() && g_enable_terrain; }
bool isObjectShadersEnabled() { return isActive() && g_enable_object_shaders; }
bool isTransparentShaderEnabled() { return isActive() && g_enable_transparent_shader; }


bool isCameraAboveCirrusClouds()
{
  auto *cirrus_clouds = core::getCirrusClouds();

  if (!cirrus_clouds)
    return false;
  else
    return core::getCamera()->getPos().z > cirrus_clouds->getHeight();
}


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


render_util::ShaderProgramPtr getTransparentProgram()
{
  auto ctx = core_gl_wrapper::getContext();
  if (!ctx->transparent_program)
  {
    ctx->transparent_program =
      render_util::createShaderProgram("transparent", core::textureManager(),
                                       core::getShaderSearchPath(), {}, core::getShaderParameters());
    ctx->transparent_program->setUniformi("sampler_0", 0);
  }
  return ctx->transparent_program;
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



void GLAPIENTRY wrap_glBlendFunc(GLenum sfactor, GLenum dfactor)
{
  gl::BlendFunc(sfactor, dfactor);

  if (wgl_wrapper::isMainContextCurrent())
  {
    getContext()->onBlendFuncChanged(sfactor, dfactor);
  }
}


void GLAPIENTRY wrap_glClear(GLbitfield mask)
{
  assert(wgl_wrapper::isMainThread());

  if (wgl_wrapper::isMainContextCurrent())
  {
    auto ctx = getContext();

    if (ctx->getRenderState().render_phase == IL2_Landscape0)
    {
      return;
    }
  }

  gl::Clear(mask);
}


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

  if (!wgl_wrapper::isMainContextCurrent() || !isActive())
  {
    return gl::Begin(mode);
  }

  auto ctx = getContext();
  ctx->updateARBProgram();
  ctx->unbindFrameBuffer();

  if (ctx->is_arb_program_active)
  {
    ctx->setActiveShader(getRedProgram());
  }

  assert(!ctx->isFrameBufferBound());

  auto &state = ctx->getRenderState();

  if (state.isRender3D1Flushing())
  {
    if (isTransparentShaderEnabled())
    {
      if (ctx->active_shader != getTransparentProgram())
      {
        assert(!ctx->active_shader);
        ctx->setActiveShader(getTransparentProgram());
      }

      ctx->active_shader->setUniform<bool>("texture_enabled", gl::IsEnabled(GL_TEXTURE_2D));
      ctx->active_shader->setUniform<bool>("is_quad", mode == GL_QUADS);
      ctx->active_shader->assertUniformsAreSet();
    }
  }
  else if (state.render_phase == IL2_SpritesFog)
  {
    assert(!ctx->active_shader);
    ctx->setActiveShader(getInvisibleProgram());
  }
  else if (!g_better_shadows)
  {
    if (state.render_phase == IL2_Landscape0)
    {
      if (ctx->active_shader != getInvisibleProgram())
      {
        assert(!ctx->active_shader);
        ctx->setActiveShader(getInvisibleProgram());
      }
    }
  }

  gl::Begin(mode);
}


void GLAPIENTRY wrap_glEnd()
{
  assert(wgl_wrapper::isMainThread());

  gl::End();

  if (wgl_wrapper::isMainContextCurrent())
  {
    auto ctx = getContext();

    if (ctx->is_arb_program_active)
      ctx->setActiveShader(nullptr);

    if (ctx->getRenderState().render_phase == IL2_SpritesFog)
      ctx->setActiveShader(nullptr);
  }
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
    auto ctx = getContext();
    ctx->onObjectDraw(GeometryType::TREES);
    assert(ctx->is_arb_program_active);
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
    auto ctx = getContext();
    ctx->onObjectDraw(GeometryType::TREES);
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

  ctx->onObjectDraw(GeometryType::OTHER);

  if (!isActive()
      || !isTerrainEnabled()
      || state.is_mirror
      || state.camera_mode == IL2_CAMERA_MODE_2D
      || state.render_phase < IL2_Landscape0
      || state.render_phase >= IL2_PostLandscape
//         || state.render_phase < IL2_Landscape0_Finished
//       || true
    )
  {
    assert(!ctx->isFrameBufferBound());

    if (state.render_phase == IL2_Cockpit && ctx->active_shader)
    {
      //FIXME - see Context::Impl::onBlendFuncChanged() / Context::Impl::onRender3D1Flushing()
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

  }

}
#endif


void drawCirrus(Context::Impl *ctx, StateModifier &state,
                const render_util::Camera &camera, bool is_far_camera)
{
  if (!g_enable_cirrus_clouds)
    return;

  auto *cirrus_clouds = core::getCirrusClouds();

  assert(cirrus_clouds);

  if (!cirrus_clouds)
    return;

  state.enableBlend(true);

  if (!is_far_camera)
    state.setDepthMask(false);

  assert(cirrus_clouds->getProgram()->isValid());

  ctx->setActiveShader(cirrus_clouds->getProgram());
  ctx->updateUniforms(cirrus_clouds->getProgram(), camera, is_far_camera);
  cirrus_clouds->getProgram()->setUniform("is_far_camera", is_far_camera);

  cirrus_clouds->draw(state, camera);

  ctx->setActiveShader(nullptr);

  state.enableBlend(false);
}


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

  auto ctx = getContext();

  auto z_far = core::getCamera()->getZFar();

  render_util::Camera far_camera(*core::getCamera());
  far_camera.setProjection(far_camera.getFov(), z_far - 4000, 1500000);

  render_util::Camera far_camera_cirrus(*core::getCamera());
  far_camera_cirrus.setProjection(far_camera.getFov(), 20000, 1500000);

  state.setFrontFace(GL_CCW);

  doDrawTerrain(terrain, far_camera, true);
  drawCirrus(ctx, state, far_camera_cirrus, true);

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

  ctx->setActiveShader(nullptr);
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

  if (!isTerrainEnabled() || ctx->getRenderState().is_mirror)
    return;

  const auto original_state = State::fromCurrent();

  StateModifier state(original_state);
  state.setDefaults();

  assert(gl::IsEnabled(GL_COLOR_LOGIC_OP) == GL_FALSE);

  ctx->updateFramebufferTextureSize();

  texture_state::freeze();
  core::textureManager().setActive(true);

  ctx->updateFrameBufferBinding(GeometryType::TERRAIN);

  gl::Clear(GL_DEPTH_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

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

  ctx->unbindFrameBuffer();

  if (g_better_shadows)
  {
    gl::ActiveTexture(GL_TEXTURE0 + TEXUNIT_SHADOW_COLOR);
    gl::BindTexture(GL_TEXTURE_2D, ctx->getFrameBuffer().getTexture(1)->getID());
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

  g_enable_cirrus_clouds = il2ge::core_wrapper::getConfig().enable_cirrus_clouds;
  g_enable_transparent_shader = il2ge::core_wrapper::getConfig().enable_transparent_shader;

#if ENABLE_CONFIGURABLE_SHADOWS
  g_better_shadows = il2ge::core_wrapper::getConfig().better_shadows;
#endif

  #if 1
//     setProc("glOrtho", (void*) &wrap_glOrtho);
//     setProc("glCallList", (void*) &wrap_glCallList);
//     setProc("glCallLists", (void*) &wrap_glCallLists);

  SET_PROC(glBlendFunc);

  setProc("glBegin", (void*) &wrap_glBegin);
  setProc("glEnd", (void*) &wrap_glEnd);
  setProc("glClear", (void*) &wrap_glClear);
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
  else if (isObjectShadersEnabled() && is_arb_program_active)
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
}


Context::Impl::~Impl()
{
}


void Context::Impl::onRenderPhaseChanged(const core::Il2RenderState &new_state)
{
  assert(wgl_wrapper::isMainContextCurrent());

  bool was_mirror = m_render_state.is_mirror;

  m_render_state = new_state;

  getARBProgramContext()->onRenderPhaseChanged(new_state.render_phase);
  getARBProgramContext()->update();

  assert(!is_arb_program_active);

  unbindFrameBuffer();

  switch (new_state.render_phase)
  {
    case core::IL2_PrePreRenders:
      m_frame_nr++;
      break;
    case core::IL2_Landscape0:
      if (isActive())
      {
        // bind terrain normal map
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

        drawTerrain();
      }
      break;
    case core::IL2_PostLandscape:
      onLandscapeFinished(was_mirror);
      break;
    case IL2_Render3D1_Flushing:
      onRender3D1Flushing();
      break;
    case IL2_Render3D1_Finished:
      onRender3D1Finished();
      break;
  }
}


void Context::Impl::onLandscapeFinished(bool was_mirror)
{
  using namespace render_util::gl_binding;

  setActiveShader(nullptr);

  if (!isActive() || was_mirror)
    return;

  if (!g_better_shadows || !isTerrainEnabled())
    return;

  auto viewport_size = getViewportSize();

  assert(!isFrameBufferBound());

  gl::BindFramebuffer(GL_READ_FRAMEBUFFER, m_framebuffer->getID());

  //FIXME - this is possibly slow
  gl::BlitFramebuffer(0, 0, viewport_size.x, viewport_size.y,
                      0, 0, viewport_size.x, viewport_size.y,
                      GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);

  gl::BindFramebuffer(GL_READ_FRAMEBUFFER, 0);

  assert(m_framebuffer->getTexture(1));
  gl::ActiveTexture(GL_TEXTURE0 + TEXUNIT_SHADOW_COLOR);
  gl::BindTexture(GL_TEXTURE_2D, m_framebuffer->getTexture(1)->getID());
  gl::ActiveTexture(GL_TEXTURE0);
}


void Context::Impl::onRender3D1Flushing()
{
  if (!isActive())
    return;

  assert(!is_arb_program_active);

  const auto original_state = State::fromCurrent();

  // cirrus
  {
    StateModifier state(original_state);
    state.setDefaults();

    drawCirrus(this, state, *core::getCamera(), false);
  }

  //FIXME - see Context::Impl::onBlendFuncChanged() / wrap_glDrawRangeElements()
  if (isTransparentShaderEnabled())
  {
    auto program = getTransparentProgram();

    updateUniforms(program);

    bool blend_add = false;
    bool alpha_texture = false;
    if (m_blend_dfactor == GL_ONE)
      blend_add = true;

    program->setUniform("blend_add", blend_add);
    program->setUniform("alpha_texture", alpha_texture);
  }
}


void Context::Impl::onRender3D1Finished()
{
  if (!isActive())
    return;

  setActiveShader(nullptr);

  const auto original_state = State::fromCurrent();

  StateModifier state(original_state);
  state.setDefaults();

  core::renderEffects();

  gl::Clear(GL_DEPTH_BUFFER_BIT);
}


void Context::Impl::onObjectDraw(GeometryType geometry)
{
  if (active_shader == getInvisibleProgram())
  {
    setActiveShader(nullptr);
  }

  updateARBProgram();
  updateFrameBufferBinding(geometry);
}


void Context::Impl::updateARBProgram()
{
  getARBProgramContext()->update();
}


void Context::Impl::updateFramebufferTextureSize()
{
  if (!g_better_shadows)
    return;

  createFrameBuffer();

  assert(m_framebuffer);

  glm::ivec2 viewport_size(m_viewport_w, m_viewport_h);
  assert(viewport_size != glm::ivec2(0));

  m_framebuffer->setSize(viewport_size);
}


void Context::Impl::createFrameBuffer()
{
  assert(g_better_shadows);

  if (m_framebuffer)
    return;

  glm::ivec2 viewport_size(m_viewport_w, m_viewport_h);
  assert(viewport_size != glm::ivec2(0));

  m_framebuffer = make_unique<FrameBuffer>(viewport_size, 2);
}


bool Context::Impl::shouldBindFrameBuffer(GeometryType geometry)
{
  if (!g_better_shadows)
    return false;

  auto &state = getRenderState();

  if (!isActive() || !isTerrainEnabled() || state.is_mirror
      || state.render_phase < IL2_Landscape0 || state.render_phase >= IL2_PostLandscape)
  {
    return false;
  }
  else if (geometry == GeometryType::TERRAIN || geometry == GeometryType::TREES
           || (is_arb_program_active && getARBProgramContext()->isObjectProgramActive()))
  {
    return true;
  }
  else
  {
    return false;
  }
}


void Context::Impl::updateFrameBufferBinding(GeometryType geometry)
{
  if (shouldBindFrameBuffer(geometry))
    bindFrameBuffer();
  else
    unbindFrameBuffer();
}


void Context::Impl::bindFrameBuffer()
{
  assert(g_better_shadows);

  if (is_framebuffer_bound)
    return;

  createFrameBuffer();

  assert(m_framebuffer);

  gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, m_framebuffer->getID());

  is_framebuffer_bound = true;
}


void Context::Impl::unbindFrameBuffer()
{

  if (!is_framebuffer_bound)
    return;

  gl::BindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  is_framebuffer_bound = false;
}


void Context::Impl::onBlendFuncChanged(GLenum sfactor, GLenum dfactor)
{
  //FIXME - see wrap_glDrawRangeElements()
  if (m_blend_sfactor != sfactor || m_blend_dfactor != dfactor)
  {
    auto &state = getRenderState();

    if (state.isRender3D1Flushing())
    {
      if (isTransparentShaderEnabled())
      {
        bool blend_add = false;
        bool alpha_texture = false;
        if (dfactor == GL_ONE)
          blend_add = true;

        auto program = getTransparentProgram();

        program->setUniform("blend_add", blend_add);
        program->setUniform("alpha_texture", alpha_texture);
      }
    }
  }

  m_blend_sfactor = sfactor;
  m_blend_dfactor = dfactor;
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

void toggleTransparentShader()
{
  g_enable_transparent_shader = !g_enable_transparent_shader;
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
