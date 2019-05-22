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

#include <misc.h>
#include <core.h>
#include <gl_wrapper.h>
#include <wgl_wrapper.h>
#include <render_util/render_util.h>

#include <string>

namespace core_gl_wrapper
{
  namespace arb_program
  {
    struct Context
    {
      struct Impl;

      std::unique_ptr<Impl> impl;

      void update();
      bool isObjectProgramActive();

      Context(core_gl_wrapper::Context::Impl&);
      ~Context();
    };

    void init();
  }

  namespace texture_state
  {
    enum { MAX_UNITS = render_util::MAX_GL_TEXUNITS };

    struct Unit
    {
      // target, texture
      std::map<unsigned int, unsigned int> bindings;
    };

    struct TextureState
    {
      bool is_frozen = false;
      unsigned int active_unit = 0;
      std::array<Unit, MAX_UNITS> units;
    };

    void init();
    void freeze();
    void restore();
  }

  struct Context::Impl
  {
    render_util::ShaderProgramPtr sky_program;
    render_util::ShaderProgramPtr terrain_program;
    render_util::ShaderProgramPtr forest_program;
    render_util::ShaderProgramPtr invisible_program;
    render_util::ShaderProgramPtr red_program;
    render_util::ShaderProgramPtr tree_program;

    bool is_arb_program_active = false;
    render_util::ShaderProgramPtr current_shader;
    render_util::ShaderProgramPtr current_arb_program;
    render_util::ShaderProgramPtr active_shader;

    std::shared_ptr<render_util::GLContext> render_util_gl_context;


    Impl();
    ~Impl();

    texture_state::TextureState *getTextureState();

    arb_program::Context *getARBProgramContext()
    {
      if (!m_arb_program_context)
        m_arb_program_context = std::make_unique<arb_program::Context>(*this);
      return m_arb_program_context.get();
    }

    void onRenderPhaseChanged(const core::Il2RenderState &state)
    {
      if (state.render_phase == core::IL2_PrePreRenders)
        m_frame_nr++;
      m_was_terrain_drawn = false;
    }

    bool isRenderingCubeMap()
    {
      auto vp_size = core::getCamera()->getViewportSize();
      return m_viewport_w != vp_size.x || m_viewport_h != vp_size.y;
    }

    bool wasTerrainDrawn() { return m_was_terrain_drawn; }
    void onTerrainDrawn() { m_was_terrain_drawn = true; }

    void setViewport(int w, int h)
    {
//       std::cout<<"setViewport: "<<w<<","<<h<<"\n";
      m_viewport_w = w;
      m_viewport_h = h;
    }

    unsigned long long getFrameNumber() { return m_frame_nr; }

    void updateUniforms(render_util::ShaderProgramPtr program,
                        const render_util::Camera &camera,
                        bool is_far_camera)
    {
      if (program->frame_nr != getFrameNumber() || program->is_far_camera != is_far_camera)
      {
        core::updateUniforms(program);
        render_util::updateUniforms(program, camera);
        program->setUniform("is_shadow", false);

        program->frame_nr = getFrameNumber();
        program->is_far_camera = is_far_camera;
      }
    }

    void updateUniforms(render_util::ShaderProgramPtr program)
    {
      updateUniforms(program, *core::getCamera(), false);
    }

    void setActiveShader(render_util::ShaderProgramPtr shader)
    {
      current_shader = shader;
      updateShaderState();
    }

    void setActiveARBProgram(render_util::ShaderProgramPtr prog)
    {
      current_arb_program = prog;
      updateShaderState();
    }

    void setIsARBProgramActive(bool active)
    {
      is_arb_program_active = active;
    }

    void updateShaderState();

  private:
    std::unique_ptr<texture_state::TextureState> m_texture_state;
    std::unique_ptr<arb_program::Context> m_arb_program_context;
    bool m_was_terrain_drawn = false;
    int m_viewport_w = 0;
    int m_viewport_h = 0;
    unsigned long long m_frame_nr = 0;
  };


  inline Context::Impl *getContext()
  {
    return wgl_wrapper::getContext()->getGLWrapperContext()->getImpl();
  }


  void setProc(const char *name, void *func);

}
