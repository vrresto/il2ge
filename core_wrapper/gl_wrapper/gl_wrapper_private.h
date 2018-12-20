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
#include <gl_wrapper.h>

#include <string>

namespace core_gl_wrapper
{
  Context::Impl *getContext();

  namespace arb_program
  {
    struct Context
    {
      struct Impl;

      std::unique_ptr<Impl> impl;

      Context();
      ~Context();
    };

    void init();
    void update();
    bool isObjectProgramActive();
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

    texture_state::TextureState *getTextureState();
    arb_program::Context *getARBProgramContext();

    Impl();
    ~Impl();

  private:
    std::unique_ptr<texture_state::TextureState> m_texture_state;
    std::unique_ptr<arb_program::Context> m_arb_program_context;
  };


  void updateUniforms(render_util::ShaderProgramPtr program);

  void setProc(const char *name, void *func);

  render_util::ShaderProgramPtr activeShader();
  void setActiveShader(render_util::ShaderProgramPtr);
  render_util::ShaderProgramPtr activeARBProgram();
  void setActiveARBProgram(render_util::ShaderProgramPtr);
  void setIsARBProgramActive(bool active);
  bool isARBProgramActive();

}
