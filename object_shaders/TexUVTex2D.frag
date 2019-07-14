#version 130

#include lighting_definitions.glsl

vec3 textureColorCorrection(vec3 color);
void apply_fog();

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
  gl_FragColor = texture2D(sampler_0, pass_texcoord);
  gl_FragColor .xyz = textureColorCorrection(gl_FragColor .xyz);

  vec3 normal = sampleTerrainNormal();

  vec3 light_direct;
  vec3 light_ambient;
  calcLight(passObjectPos, normal, light_direct, light_ambient);

  light_direct *= DIRECT_LIGHT_SCALE;
  light_ambient *= AMBIENT_LIGHT_SCALE;

  gl_FragColor.xyz *= light_direct + light_ambient;

  apply_fog();
}
