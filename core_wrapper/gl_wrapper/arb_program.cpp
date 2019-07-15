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

#undef NDEBUG

#include "core.h"
#include "misc.h"
#include "gl_wrapper.h"
#include "gl_wrapper_private.h"
#include <wgl_wrapper.h>
#include <render_util/shader.h>
#include <render_util/texunits.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <array>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <GL/gl.h>
#include <GL/glext.h>

#include <render_util/gl_binding/gl_functions.h>

using std::cout;
using std::cerr;
using std::endl;
using std::string;

using namespace core;
using namespace render_util::gl_binding;


namespace
{

bool g_initialized = false;
bool g_enable_object_shaders = false;


struct RenderPhase
{
  enum Enum : unsigned int
  {
    RENDER0 = 0,
    RENDER0_SHADOWS,
    RENDER1,
    COCKPIT,
    DEFAULT,
    MAX
  };
};

std::unordered_set<string> g_required_programs =
{
  "VAObjectsN.ObjectsL0",
  "VAObjectsL0.ObjectsL0",
  "VAObjectsL0.VAObjectsL0_Cockpit",
  "TexUVTex2D.TexUVTex2D",
  "TreeTrunk.TreeTrunk",
  "TreeSprite.TreeSprite",
};

using Context = core_gl_wrapper::arb_program::Context::Impl;

Context &getContext(bool use_main_context = false);


std::unordered_set<std::string> enabled_shaders;

constexpr const bool is_replace_arb_program_string_enabled = true;


const char replacement_path[] = IL2GE_DATA_DIR "/object_shaders";


render_util::ShaderProgramPtr createGLSLProgram(const string &vertex_shader_,
                                                const string &fragment_shader_,
                                                RenderPhase::Enum render_phase)
{
  using namespace std;

  if (render_phase == RenderPhase::DEFAULT)
  {
    return make_shared<render_util::ShaderProgram>();
  }

  // strip "vp" / "fp" prefix
  auto vertex_shader = vertex_shader_.substr(2);
  auto fragment_shader = fragment_shader_.substr(2);

  auto program_name = vertex_shader + '.' + fragment_shader;

  cout<<"creating program: "<<vertex_shader<<", "<<fragment_shader<<endl;

  vector<string> vert;
  vector<string> frag;

  vert.push_back(vertex_shader);
  vert.push_back("main");
  frag.push_back(fragment_shader);
  frag.push_back("atmosphere");
  frag.push_back("functions");
  frag.push_back("lighting");
  frag.push_back("tonemap");
  frag.push_back("mapping");
  frag.push_back("main");

  render_util::ShaderSearchPath paths;
  paths.push_back(replacement_path);
  for (auto &dir : core::getShaderSearchPath())
    paths.push_back(dir);

  auto params = core::getShaderParameters();
  params.set("is_render0", render_phase < RenderPhase::RENDER1);
  params.set("is_shadow", render_phase == RenderPhase::RENDER0_SHADOWS);

  auto program = make_shared<render_util::ShaderProgram>(program_name,
    vert,
    frag,
    vector<string>(),
    vector<string>(),
    paths,
    false,
    std::map<unsigned int, std::string>(),
    params);

  CHECK_GL_ERROR();

  if (!program->isValid())
  {
    assert(g_required_programs.count(program_name) == 0);
    return make_shared<render_util::ShaderProgram>();
  }

  for (int i = 0; i < 20; i++)
  {
    char tmp[100];
    snprintf(tmp, sizeof(tmp), "sampler_%d", i);
    program->setUniformi(tmp, i);
  }

  //FIXME
  program->setUniformi("sampler_atmosphere_thickness_map", core::textureManager().getHighestUnit() + 1);

  program->setUniformi("sampler_terrain_normal_map", core_gl_wrapper::TEXUNIT_TERRAIN_NORMAL_MAP);
  program->setUniformi("sampler_shadow_color", core_gl_wrapper::TEXUNIT_SHADOW_COLOR);

  auto vertex_uniform_block_index =
    gl::GetUniformBlockIndex(program->getId(), "local_vertex_parameters");
  if (vertex_uniform_block_index != GL_INVALID_INDEX)
  {
    gl::UniformBlockBinding(program->getId(), vertex_uniform_block_index, 0);
//     GLint data_size = 0;
//     gl::GetActiveUniformBlock(program->getId(), vertex_uniform_block_index,
//                               GL_UNIFORM_BLOCK_DATA_SIZE, &data_size);
  }


  gl::GetError();

  CHECK_GL_ERROR();


  cout<<"created program: "<<vertex_shader<<", "<<fragment_shader<<endl;

  return program;
}


struct ShaderHashTableEntry
{
  const char *name;
  unsigned long hash_value;
};

ShaderHashTableEntry shader_hashes[] =
{
  {"fpWaterSunLightFast", 2817968272},
  {"fpWaterSunLight", 632849162},
  {"fpWaterSunLightBest", 3531821305},
  {"fpCoastBump", 1124432921},
  {"fpCoastFoam", 2644930622},
  {"fpCoastFoamFast", 1435349844},
  {"fpCoastFoamFarFogTex", 314201521},
  {"fpCausticSimple", 3479977242},
  {"fpCaustic", 2728617329},
  {"fpSprites", 4119988601},
  {"fpObjectsL0", 3801593370},
  {"fpObjectsL0_2L", 991434619},
  {"fpSimpleGL", 3540320514},
  {"fpNearLandFog", 87025808},
  {"fpFarLandFog", 2232994666},
  {"fpRiverCoastAA", 3741666452},
  {"fpWaterDM_CPU", 1594865371},
  {"fpWaterDM_CPULo", 155489676},
  {"fpWaterLFogDM", 281904700},
  {"fpIceWater", 243049986},
  {"fpNearNoBlend", 415116501},
  {"fpNearNoBlendNoise", 1442106534},
  {"fpNearBlend", 1574944565},
  {"fpNearBlendNoise", 356709871},
  {"fpFarBlend", 2313301566},
  {"fpForestPlane", 4206698854},
  {"fpForestPlaneNoise", 1419226297},
  {"fpForestPlaneEdges", 4075197594},
  {"fpForestPlaneEdgesNoise", 109324726},
  {"vpFogFar2Tex2D", 3531779195},
  {"vpFog2Tex2DBlend", 2358603363},
  {"vpFogFar4Tex2D", 892403915},
  {"vpFogFar8Tex2D", 2387820957},
  {"vpFogNoTex", 462566402},
  {"vpFog4Tex2D", 3557282540},
  {"vpFog4Tex2D_UV2", 3947551348},
  {"vp4Tex2D", 3895209446},
  {"vp6Tex2D", 3003880186},
  {"vpTexUVTex2D", 2144006323},
  {"vpWaterGrid_NV", 3771374482},
  {"vpWaterSunLight_NV", 604803814},
  {"vpWaterSunLight_ATI", 2350930377},
  {"vpWaterSunLight_FP", 2803049495},
  {"vpTreeSprite", 2721248406},
  {"vpTreeTrunk", 2545682697},
  {"vpVAObjectsN", 1914100752},
  {"vpVAObjectsL0", 1135272293},
  {"vpSprites", 331927207},
  {"vpSimpleGL", 3662837049},

  // optimized variants
  {"fpWaterSunLightFast", 977067007},
  {"fpWaterSunLight", 2131239042},
  {"fpWaterSunLightBest", 4062392817},
  {"fpCoastBump", 1804904102},
  {"fpCoastFoam", 1083737417},
  {"fpCoastFoamFast", 3738753912},
  {"fpCoastFoamFarFogTex", 3120575906},
  {"fpCausticSimple", 292955408},
  {"fpCaustic", 1454717634},
  {"fpSprites", 1218231756},
  {"fpObjectsL0", 3814157290},
  {"fpObjectsL0_2L", 251922401},
  {"fpNearLandFog", 65822550},
  {"fpFarLandFog", 1339223649},
  {"vpWaterDM_GPU", 2334360762},
  {"vpWaterDM_GPU8800", 3267036497},
  {"fpCoastFoam8800", 1256648043},
  {"fpCoastFoamFarFogTex8800", 527202397},
  {"fpCoastBump8800", 1527700593},
  {"vpWaterDM_CPU", 2612379814},
  {"fpWaterNearDM", 2725810045},
  {"fpWaterMiddleDM", 1160002433},
  {"fpWaterFarDM", 2519986775},
  {"fpWaterDM_CPU", 4087157168},
  {"fpWaterDM_CPULo", 3355252201},
  {"fpWaterNearDM8800", 4056581137},
  {"fpWaterMiddleDM8800", 1621302650},
  {"fpWaterFarDM8800", 645514079},
  {"fpWaterLFogDM8800", 2197871837},
  {"fpIceWater", 1895331007},
  {"fpNearNoBlend", 1157933730},
  {"fpNearNoBlendNoise", 1634443498},
  {"fpNearBlend", 4033623987},
  {"fpNearBlendNoise", 3765832238},
  {"fpFarBlend", 2375977440},
  {"fpForestPlane", 2772098296},
  {"fpForestPlaneNoise", 2365797049},
  {"fpForestPlaneEdges", 1926783243},
  {"fpForestPlaneEdgesNoise", 162196373},
};
enum
{
  NUM_SHADER_HASHES = (sizeof(shader_hashes) / sizeof(ShaderHashTableEntry))
};


std::vector<std::string> g_shader_names;

size_t getShaderNameID(const std::string &name)
{
  for (size_t i = 0; i < g_shader_names.size(); i++)
  {
    if (name == g_shader_names[i])
      return i + 1;
  }
  g_shader_names.push_back(name);
  return g_shader_names.size();
}


class LocalParameters
{
  struct Parameter
  {
    glm::vec4 value;
    bool is_set = false;
  };

  struct ParameterValue
  {
    GLfloat x = 0;
    GLfloat y = 0;
    GLfloat z = 0;
    GLfloat w = 0;
  };

  std::vector<Parameter> params;
  bool needs_update = false;
  GLuint uniform_buffer_id = 0;
  std::vector<ParameterValue> uniform_buffer_content;

  Parameter &getParameter(size_t index)
  {
    if (params.size() < index+1)
      params.resize(index+1);
    return params[index];
  }

  ParameterValue &getUniformBufferElement(size_t index)
  {
    if (uniform_buffer_content.size() < index+1)
      uniform_buffer_content.resize(index+1);
    return uniform_buffer_content[index];
  }

  size_t getUniformBufferSize()
  {
    return uniform_buffer_content.size() * sizeof(ParameterValue);
  }

  void *getUniformBufferData()
  {
    return (void*) uniform_buffer_content.data();
  }

public:
  ~LocalParameters()
  {
    if (uniform_buffer_id)
      gl::DeleteBuffers(1, &uniform_buffer_id);
  }

  GLuint getUniformBufferID()
  {
    if (!uniform_buffer_id)
      gl::GenBuffers(1, &uniform_buffer_id);
    assert(uniform_buffer_id);
    return uniform_buffer_id;
  }

  bool needsUpdate() { return needs_update; }

  void set(size_t index, const glm::vec4 &value)
  {
    auto &p = getParameter(index);
    if (!p.is_set || (value != p.value))
      needs_update = true;
    p.value = value;
    p.is_set = true;

    getUniformBufferElement(index) = {value.x, value.y, value.z, value.w};
  }

  void updateUniformBuffer()
  {
    gl::BindBuffer(GL_UNIFORM_BUFFER, getUniformBufferID());
    gl::BufferData(GL_UNIFORM_BUFFER, getUniformBufferSize(), nullptr, GL_STREAM_DRAW);
    gl::BufferData(GL_UNIFORM_BUFFER, getUniformBufferSize(), getUniformBufferData(), GL_STREAM_DRAW);
    gl::BindBuffer(GL_UNIFORM_BUFFER, 0);
    needs_update = false;
  }

};


enum ProgramState
{
  PROGRAM_STATE_NEW,
  PROGRAM_STATE_SOURCE_SET,
};


struct ProgramBase
{
  ProgramState state = PROGRAM_STATE_NEW;
  GLenum shader_stage = 0;
  GLuint real_id = 0;
  GLenum target = 0;
  Context *context = 0;
  size_t name_id = 0;

  std::string name;
  std::string file_extension;

  std::array<render_util::ShaderProgramPtr, RenderPhase::MAX> default_glsl_program;

  LocalParameters params;

  ProgramBase(Context *ctx) : context(ctx) {}

  bool isFragmentProgram() const
  {
    return target == GL_FRAGMENT_PROGRAM_ARB;
  }

  size_t getNameID()
  {
    if (!name_id)
    {
      assert(!name.empty());
      name_id = getShaderNameID(name);
    }
    return name_id;
  }

  render_util::ShaderProgramPtr getDefaultGLSLProgram(RenderPhase::Enum render_phase)
  {
    auto &program = default_glsl_program.at(render_phase);

    if (!program)
    {
      if (!isFragmentProgram())
      {
        assert(!name.empty());

        string frag_name;

        if (name == "vpTreeSprite")
          frag_name = "fpTreeSprite";
        else if (name == "vpTreeTrunk")
          frag_name = "fpTreeTrunk";
        else if (name == "vpTexUVTex2D")
          frag_name = "fpTexUVTex2D";
        else if (name == "vp4Tex2D")
          frag_name = "fp4Tex2D";
        else if (name == "vpFogFar2Tex2D")
          frag_name = "fpFogFar2Tex2D";
        else if (name == "vpVAObjectsL0")
          frag_name = "fpVAObjectsL0";
        else
        {
          cout<<"no default program for "<<name<<endl;
          program = std::make_shared<render_util::ShaderProgram>();
        }

        if (!frag_name.empty())
        {
          if (render_phase == RenderPhase::COCKPIT)
            frag_name += "_cockpit";
          program = createGLSLProgram(name, frag_name, render_phase);
        }
      }
      else
      {
        program = std::make_shared<render_util::ShaderProgram>();
      }
    }

    assert(program);

    return program;
  }
};


struct FragmentProgram : public ProgramBase
{
  using ProgramList = std::vector<render_util::ShaderProgramPtr>;

  std::array<ProgramList, RenderPhase::MAX> glsl_program_for_vertex_shader;

  bool is_object_program = false;

  FragmentProgram(Context *ctx) : ProgramBase(ctx) {}

  render_util::ShaderProgramPtr getGLSLProgram(ProgramBase *vertex_program,
                                               RenderPhase::Enum render_phase)
  {
    CHECK_GL_ERROR();

    auto &programs = glsl_program_for_vertex_shader.at(render_phase);

    if (programs.size() <= vertex_program->getNameID())
    {
      programs.resize(vertex_program->getNameID() + 100);
    }

    auto program = programs.at(vertex_program->getNameID());

    if (!program)
    {
      if (render_phase == RenderPhase::COCKPIT)
        program = createGLSLProgram(vertex_program->name + "_cockpit", name + "_cockpit",
                                    render_phase);
      else
        program = createGLSLProgram(vertex_program->name, name, render_phase);
      programs.at(vertex_program->getNameID()) = program;
    }

    return program;
  }
};


unsigned long myHashFunc(const void *str, size_t len)
{
  unsigned long hash = 5381;

  for (int i = 0; i < len; i++)
  {
    unsigned char c = ((const unsigned char*)str)[i];
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
  }

  return hash;
}


const char * getShaderName(const void *source, size_t len)
{
  unsigned long hash_value = myHashFunc(source, len);

  const char *name = 0;

  for (int i = 0; i < NUM_SHADER_HASHES; i++)
  {
    if (shader_hashes[i].hash_value != hash_value)
      continue;

    name = shader_hashes[i].name;
    break;
  }

  if (!name)
  {
    printf("unknown shader: %lu\n", hash_value);
    assert(0);
    abort();
  }

  return name;
}


} // namespace


struct core_gl_wrapper::arb_program::Context::Impl
{
  core_gl_wrapper::Context::Impl &main_context;

  ProgramBase *active_vertex_program = 0;
  FragmentProgram *active_fragment_program = 0;
  bool is_vertex_program_enabled = false;
  bool is_fragment_program_enabled = false;
  bool is_stencil_test_enabled = false;
  bool program_needs_update = false;
  core::Il2RenderPhase render_phase = core::IL2_PrePreRenders;

  std::vector<std::unique_ptr<ProgramBase>> programs;

  Impl(core_gl_wrapper::Context::Impl &main_context) : main_context(main_context)
  {
    programs.resize(500);
    programs[0] = std::make_unique<ProgramBase>(this);
  }

  RenderPhase::Enum getRenderPhase()
  {
    if (render_phase >= core::IL2_Landscape0 && render_phase < core::IL2_PostLandscape)
    {
      if (is_stencil_test_enabled)
        return RenderPhase::RENDER0_SHADOWS;
      else
        return RenderPhase::RENDER0;
    }
    else if(render_phase < core::IL2_Cockpit)
    {
      return RenderPhase::RENDER1;
    }
    else if(render_phase == core::IL2_Cockpit)
    {
      return RenderPhase::COCKPIT;
    }
    else
    {
      return RenderPhase::DEFAULT;
    }
  }

  void getFreeIDs(GLsizei n, GLuint *ids)
  {
    GLsizei num_found = 0;
    for (size_t i = 0; i < programs.size() && num_found < n; i++)
    {
      if (!programs[i])
      {
        num_found++;
        *ids = i;
        ids++;
      }
    }
    assert(num_found == n);
  }

  void enableVertexProgram(int enable)
  {
    if (is_vertex_program_enabled == enable)
      return;

    is_vertex_program_enabled = enable;

    program_needs_update = true;
  }

  void enableFragmentProgram(int enable)
  {
    if (is_fragment_program_enabled == enable)
      return;

    is_fragment_program_enabled = enable;

    program_needs_update = true;
  }

  void enableStencilTest(bool enable)
  {
    if (is_stencil_test_enabled == enable)
      return;

    is_stencil_test_enabled = enable;
    program_needs_update = true;
  }

  void createProgram(GLint id, GLenum target)
  {
    printf("createProgram: %d, %d\n", id, target);

    assert(!programs.at(id));

    std::unique_ptr<ProgramBase> p;
    if (target == GL_VERTEX_PROGRAM_ARB) {
      p = std::make_unique<ProgramBase>(this);
      p->shader_stage = GL_VERTEX_SHADER;
      p->file_extension = "vert";
    }
    else if(target == GL_FRAGMENT_PROGRAM_ARB) {
      p = std::make_unique<FragmentProgram>(this);
      p->shader_stage = GL_FRAGMENT_SHADER;
      p->file_extension = "frag";
    }
    else {
      assert(0);
      abort();
    }

    p->target = target;

    programs.at(id) = std::move(p);
  }

  ProgramBase *programForID(GLint id, GLenum target)
  {
    assert(id > 0);
    assert(id < programs.size());

    ProgramBase *p = programs.at(id).get();

    if (!p)
    {
      createProgram(id, target);
      p = programs.at(id).get();
    }

    assert(!target || p->target == target);

    return p;
  }

  ProgramBase *getActiveProgram(GLenum target)
  {
    ProgramBase *p = 0;

    if (target == GL_VERTEX_PROGRAM_ARB)
      p = active_vertex_program;
    else if (target == GL_FRAGMENT_PROGRAM_ARB)
      p = active_fragment_program;
    else {
      printf("invalid target.\n");
      exit(0);
    }

    assert(p);

    return p;
  }

  void deleteProgram(GLuint id)
  {
    ProgramBase *p = programForID(id, 0);
    assert(p);

    if (p->target == GL_VERTEX_PROGRAM_ARB) {
      if (active_vertex_program == p)
        active_vertex_program = 0;
    }
    else if(p->target == GL_FRAGMENT_PROGRAM_ARB) {
      if (active_fragment_program == p)
        active_fragment_program = 0;
    }
    else {
      assert(0);
      abort();
    }

    gl::DeleteProgramsARB(1, &p->real_id);

    programs.at(id).reset();

    program_needs_update = true;
  }

  void bindProgram(GLenum target, GLuint id)
  {
    bool is_main_context = wgl_wrapper::isMainContextCurrent();

    ProgramBase *p = programForID(id, target);
    if (!p)
    {
      assert(0);
      abort();
    }

    assert(p->target == target);

    if (is_main_context)
    {
      if (target == GL_VERTEX_PROGRAM_ARB)
      {
        if (active_vertex_program == p)
          return;
        active_vertex_program = p;
      }
      else if(target == GL_FRAGMENT_PROGRAM_ARB)
      {
        if (active_fragment_program == p)
          return;
        active_fragment_program = static_cast<FragmentProgram*>(p);
      }
      else
      {
        assert(0);
        abort();
      }

      if (!p->real_id)
        gl::GenProgramsARB(1, &p->real_id);
    }

    assert(p->real_id);

    gl::BindProgramARB(target, p->real_id);

    if (is_main_context)
      program_needs_update = true;
  }

  void updateLocalParameters(ProgramBase *program)
  {
    if (program->params.needsUpdate())
      program->params.updateUniformBuffer();
  }

  void updateLocalParameters()
  {
    if (active_vertex_program)
      updateLocalParameters(active_vertex_program);
    if (active_fragment_program)
      updateLocalParameters(active_fragment_program);
  }

  void bindUniformBuffers()
  {
    if (active_vertex_program)
      gl::BindBufferBase(GL_UNIFORM_BUFFER, 0, active_vertex_program->params.getUniformBufferID());
    if (active_fragment_program)
      gl::BindBufferBase(GL_UNIFORM_BUFFER, 1, active_fragment_program->params.getUniformBufferID());
    gl::BindBuffer(GL_UNIFORM_BUFFER, 0);
  }

  void updateProgram(core::Il2RenderPhase render_phase_new)
  {
    if (!program_needs_update && render_phase_new == render_phase)
    {
      return;
    }

    render_phase = render_phase_new;

    auto phase = getRenderPhase();

    bool is_arb_program_active = false;

    render_util::ShaderProgramPtr glsl_program;

    if (is_fragment_program_enabled && active_fragment_program ||
        is_vertex_program_enabled && active_vertex_program)
    {
      is_arb_program_active = true;

      if (g_enable_object_shaders)
      {
        if (is_fragment_program_enabled && active_fragment_program)
        {
          if (is_vertex_program_enabled && active_vertex_program)
          {
            glsl_program = active_fragment_program->getGLSLProgram(active_vertex_program, phase);
          }
          else
          {
            glsl_program = active_fragment_program->getDefaultGLSLProgram(phase);
          }
        }
        else if (is_vertex_program_enabled && active_vertex_program)
        {
          glsl_program = active_vertex_program->getDefaultGLSLProgram(phase);
        }
      }
    }

    main_context.setIsARBProgramActive(is_arb_program_active);

    if (g_enable_object_shaders)
    {
      if (glsl_program && glsl_program->isValid())
      {
        main_context.setActiveARBProgram(glsl_program);
        main_context.updateUniforms(glsl_program);
        glsl_program->assertUniformsAreSet();
        bindUniformBuffers();
      }
      else
      {
        main_context.setActiveARBProgram(nullptr);
      }
    }

    program_needs_update = false;
  }

  void update(core::Il2RenderPhase render_phase)
  {
    if (!g_initialized)
      return;

    updateProgram(render_phase);

    if (g_enable_object_shaders)
      updateLocalParameters();
  }

  bool isObjectProgramActive()
  {
    if (!g_initialized)
      return false;

    if (is_fragment_program_enabled && active_fragment_program)
        return active_fragment_program->is_object_program ;
    else
      return false;
  }

};


namespace
{


Context &getContext(bool use_main_context)
{
  if (use_main_context)
  {
    return *wgl_wrapper::getMainContext()->
      getGLWrapperContext()->getImpl()->getARBProgramContext()->impl.get();
  }
  else
    return *core_gl_wrapper::getContext()->getARBProgramContext()->impl.get();
}


ProgramBase *getActiveProgram(GLenum target)
{
  return getContext().getActiveProgram(target);
}


////////////////////////////////////////////////////////////////////////////


void GLAPIENTRY
wrap_GenProgramsARB(GLsizei n, GLuint *ids)
{
  getContext().getFreeIDs(n, ids);
}

void GLAPIENTRY
wrap_DeleteProgramsARB(GLsizei n, const GLuint *ids)
{
  for (GLsizei i = 0; i < n; i++)
  {
    getContext().deleteProgram(ids[i]);
  }
}

GLboolean GLAPIENTRY
wrap_IsProgramARB(GLuint id)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_BindProgramARB(GLenum target, GLuint id)
{
  getContext(true).bindProgram(target, id);
}

void GLAPIENTRY
wrap_ProgramStringARB(GLenum target, GLenum format, GLsizei len,
                       const GLvoid *string)
{
  ProgramBase *p = getActiveProgram(target);
  assert(p);

  if (p->state != PROGRAM_STATE_NEW)
  {
    printf("state %d\n", p->state);
    exit(0);
  }

  assert(p->state == PROGRAM_STATE_NEW);
  assert(p->target == target);

  p->state = PROGRAM_STATE_SOURCE_SET;

  const void *replacement_string = string;
  size_t replacement_len = len;

  gl::ProgramStringARB(target, format, replacement_len, replacement_string);

  const char *name = getShaderName(string, len);

  if (name)
  {
    p->name = name;
    if (p->isFragmentProgram())
    {
      static_cast<FragmentProgram*>(p)->is_object_program =
        (p->name == "fpObjectsL0_2L" || p->name == "fpObjectsL0");
    }
  }
}

void GLAPIENTRY
wrap_ProgramLocalParameter4fARB(GLenum target, GLuint index,
                                 GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  gl::ProgramLocalParameter4fARB(target, index, x, y, z, w);

  if (g_enable_object_shaders && wgl_wrapper::isMainContextCurrent())
  {
    ProgramBase *p = getActiveProgram(target);
    assert(p);

    glm::vec4 value(x,y,z,w);

    p->params.set(index, value);
  }
}


void GLAPIENTRY
wrap_ProgramLocalParameter4fvARB(GLenum target, GLuint index,
                                  const GLfloat *params)
{
  wrap_ProgramLocalParameter4fARB(target, index, params[0], params[1], params[2], params[3]);
}

void GLAPIENTRY
wrap_ProgramEnvParameter4dARB(GLenum target, GLuint index,
                               GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_ProgramEnvParameter4dvARB(GLenum target, GLuint index,
                                const GLdouble *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_ProgramEnvParameter4fARB(GLenum target, GLuint index,
                               GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_ProgramEnvParameter4fvARB(GLenum target, GLuint index,
                                const GLfloat *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_ProgramEnvParameters4fvEXT(GLenum target, GLuint index, GLsizei count,
                                 const GLfloat *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_ProgramLocalParameter4dARB(GLenum target, GLuint index,
                                 GLdouble x, GLdouble y,
                                 GLdouble z, GLdouble w)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_ProgramLocalParameter4dvARB(GLenum target, GLuint index,
                                  const GLdouble *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_ProgramLocalParameters4fvEXT(GLenum target, GLuint index, GLsizei count,
                                   const GLfloat *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_GetProgramEnvParameterdvARB(GLenum target, GLuint index,
                                  GLdouble *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_GetProgramEnvParameterfvARB(GLenum target, GLuint index,
                                  GLfloat *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_GetProgramLocalParameterdvARB(GLenum target, GLuint index,
                                    GLdouble *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_GetProgramLocalParameterfvARB(GLenum target, GLuint index,
                                    GLfloat *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_GetProgramivARB(GLenum target, GLenum pname, GLint *params)
{
  assert(0);
  abort();
}

void GLAPIENTRY
wrap_GetProgramStringARB(GLenum target, GLenum pname, GLvoid *string)
{
  assert(0);
  abort();
}


void setEnabled(GLenum cap, bool enable)
{
  if (cap == GL_FRAGMENT_PROGRAM_ARB)
  {
    getContext().enableFragmentProgram(enable);
  }
  else if (cap == GL_VERTEX_PROGRAM_ARB)
  {
    getContext().enableVertexProgram(enable);
  }
  else if (cap == GL_STENCIL_TEST)
  {
    getContext().enableStencilTest(enable);
  }
}

void GLAPIENTRY wrap_Enable(GLenum cap)
{
  assert(wgl_wrapper::isMainThread());

  gl::Enable(cap);

  if (wgl_wrapper::isMainContextCurrent())
    setEnabled(cap, true);
}

void GLAPIENTRY wrap_Disable(GLenum cap)
{
  assert(wgl_wrapper::isMainThread());

  gl::Disable(cap);

  if (wgl_wrapper::isMainContextCurrent())
    setEnabled(cap, false);
}


} // namespace


namespace core_gl_wrapper::arb_program
{

  Context::Context(core_gl_wrapper::Context::Impl &main_context) :
    impl(std::make_unique<core_gl_wrapper::arb_program::Context::Impl>(main_context))
  {
  }


  Context::~Context() {}


  void Context::update(core::Il2RenderPhase render_phase)
  {
    impl->update(render_phase);
  }


  bool Context::isObjectProgramActive()
  {
    return impl->isObjectProgramActive();
  }


  #define SET_OVERRIDE(func) core_gl_wrapper::setProc("gl"#func, (void*) wrap_##func);

  void init()
  {
    g_enable_object_shaders = il2ge::core_wrapper::getConfig().enable_object_shaders;

    SET_OVERRIDE(Enable)
    SET_OVERRIDE(Disable)

    SET_OVERRIDE(BindProgramARB)
    SET_OVERRIDE(DeleteProgramsARB)
    SET_OVERRIDE(GenProgramsARB)
    SET_OVERRIDE(IsProgramARB)
    SET_OVERRIDE(ProgramStringARB)
    SET_OVERRIDE(ProgramEnvParameter4dARB)
    SET_OVERRIDE(ProgramEnvParameter4dvARB)
    SET_OVERRIDE(ProgramEnvParameter4fARB)
    SET_OVERRIDE(ProgramEnvParameter4fvARB)
    SET_OVERRIDE(ProgramEnvParameters4fvEXT)
    SET_OVERRIDE(ProgramLocalParameter4dARB)
    SET_OVERRIDE(ProgramLocalParameter4dvARB)
    SET_OVERRIDE(ProgramLocalParameter4fARB)
    SET_OVERRIDE(ProgramLocalParameter4fvARB)
    SET_OVERRIDE(ProgramLocalParameters4fvEXT)
    SET_OVERRIDE(GetProgramEnvParameterdvARB)
    SET_OVERRIDE(GetProgramEnvParameterfvARB)
    SET_OVERRIDE(GetProgramLocalParameterdvARB)
    SET_OVERRIDE(GetProgramLocalParameterfvARB)
    SET_OVERRIDE(GetProgramivARB)
    SET_OVERRIDE(GetProgramStringARB)

    g_initialized = true;
  }

}
