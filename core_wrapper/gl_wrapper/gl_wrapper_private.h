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
#include <render_util/shader.h>

#include <string>
#include <unordered_map>

namespace core_gl_wrapper
{
  struct Context : public Module
  {
    render_util::ShaderProgramPtr sky_program;
    render_util::ShaderProgramPtr terrain_program;
    render_util::ShaderProgramPtr forest_program;
    render_util::ShaderProgramPtr invisible_program;
    render_util::ShaderProgramPtr red_program;
    render_util::ShaderProgramPtr tree_program;
  };

  Context *getContext();

  const std::string &getFragmentProgramName();

  void updateARBProgram();

  void updateUniforms(render_util::ShaderProgramPtr program);

  void setProc(const char *name, void *func);

  namespace texture_state
  {
    void init();
    void freeze();
    void restore();
  }

}
