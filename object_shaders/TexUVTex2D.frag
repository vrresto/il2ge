#version 330

#define ENABLE_UNLIT_OUTPUT @enable_unlit_output:0@

#include lighting_definitions.glsl

vec3 textureColorCorrection(vec3 color);
vec3 fogAndToneMap(vec3);
void fogAndToneMap(in vec3 in_color0, in vec3 in_color1,
                   out vec3 out_color0, out vec3 out_color1);


layout(location = 0) out vec4 out_color0;
#if ENABLE_UNLIT_OUTPUT
layout(location = 1) out vec4 out_color1;
#endif

uniform sampler2D sampler_0;
uniform sampler2D sampler_terrain_normal_map;

uniform vec3 sunDir;
uniform vec2 map_size;

varying vec3 passObjectPos;
varying vec3 pass_normal;
varying vec2 pass_texcoord;

#if USE_HDR
const float DIRECT_LIGHT_SCALE = 1.0;
const float AMBIENT_LIGHT_SCALE = 1.0;
#else
const float DIRECT_LIGHT_SCALE = 0.9;
const float AMBIENT_LIGHT_SCALE = 0.9;
#endif


vec3 sampleTerrainNormal()
{
  vec2 normal_map_coord = fract((passObjectPos.xy + vec2(0, 200)) / map_size);
  normal_map_coord.y = 1.0 - normal_map_coord.y;
  vec3 normal = texture2D(sampler_terrain_normal_map, normal_map_coord).xyz;

  normal.y *= -1;

  return normal;
}


void main()
{
  vec4 tex_color = texture2D(sampler_0, pass_texcoord);
  tex_color.xyz = textureColorCorrection(tex_color.xyz);

  vec3 normal = sampleTerrainNormal();

  vec3 light_direct;
  vec3 light_ambient;
  calcLight(passObjectPos, normal, light_direct, light_ambient);

  light_direct *= DIRECT_LIGHT_SCALE;
  light_ambient *= AMBIENT_LIGHT_SCALE;

  out_color0.a = tex_color.a;
#if ENABLE_UNLIT_OUTPUT
  out_color1.a = tex_color.a;
#endif

  out_color0.xyz = tex_color.xyz * (light_direct + light_ambient);
#if ENABLE_UNLIT_OUTPUT
  out_color1.xyz = tex_color.xyz * light_ambient;
  fogAndToneMap(out_color0.xyz, out_color1.xyz, out_color0.xyz, out_color1.xyz);
#else
  out_color0.xyz = fogAndToneMap(out_color0.xyz);
#endif
}
