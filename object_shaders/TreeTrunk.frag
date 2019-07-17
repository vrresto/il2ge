#version 330

#define ENABLE_UNLIT_OUTPUT @enable_unlit_output:0@
#define IS_RENDER0 @is_render0@
#define IS_SHADOW IS_RENDER0 && @is_blend_enabled:0@

#include lighting_definitions.glsl

vec3 textureColorCorrection(vec3 color);
vec3 apply_fog(vec3);

layout(location = 0) out vec4 out_color0;
#if ENABLE_UNLIT_OUTPUT && IS_RENDER0
layout(location = 1) out vec4 out_color1;
#endif

uniform sampler2D sampler_0;
#if ENABLE_UNLIT_OUTPUT
uniform sampler2D sampler_shadow_color;
#endif

uniform vec3 sunDir;

varying vec4 pass_texcoord;
varying vec3 passObjectPos;


#if ENABLE_UNLIT_OUTPUT

void main()
{
  out_color0 = texture2D(sampler_0, pass_texcoord.xy);

  #if IS_SHADOW
  {
    out_color0.xyz = texelFetch(sampler_shadow_color, ivec2(gl_FragCoord.xy), 0).xyz;
    out_color1 = vec4(1,0,1,0);
  }
  #else
  {
    vec3 light_ambient_incoming;
    vec3 light_direct_incoming;
    getIncomingLight(passObjectPos, light_ambient_incoming, light_direct_incoming);

    #if IS_RENDER0
    {
      out_color0.xyz = textureColorCorrection(out_color0 .xyz);
      out_color0.xyz *= 0.8 * getReflectedAmbientLight(vec3(0,0,1), light_ambient_incoming);
      out_color0.xyz = apply_fog(out_color0.xyz);
      out_color1 = out_color0;
    }
    #else
    {
      #error This shouldn't happen!
    }
    #endif
  }
  #endif

}

#else

void main()
{
  out_color0 = texture2D(sampler_0, pass_texcoord.xy);

#if IS_RENDER0
  out_color0.xyz = vec3(0);
  out_color0.a *= 0.5 * smoothstep(-0.02, 0.02, sunDir.z);
#else
  out_color0.xyz = textureColorCorrection(out_color0.xyz);

  vec3 light_ambient_incoming;
  vec3 light_direct_incoming;
  getIncomingLight(passObjectPos, light_ambient_incoming, light_direct_incoming);

  out_color0.xyz *= 0.8 * getReflectedAmbientLight(vec3(0,0,1), light_ambient_incoming);
#endif

  out_color0.xyz = apply_fog(out_color0.xyz);
}

#endif
