#version 130

#include lighting_definitions.glsl

vec3 textureColorCorrection(vec3 color);
void apply_fog();

uniform mat4 view2WorldMatrix;
uniform vec3 sunDir = vec3(0,0,1);
uniform vec3 cameraPosWorld;
uniform vec2 map_size;

uniform sampler2D sampler_0;
uniform sampler2D sampler_1;
uniform sampler2D sampler_terrain_normal_map;

varying vec3 pass_quad_center;
varying float pass_shadow;
varying vec2 pass_texcoord_diffuse;
varying vec2 pass_texcoord_normal;
varying vec3 passObjectPos;


vec3 calcTerrainLight(vec3 normal, float direct_scale, float ambient_scale,
                      vec3 light_direct_incoming, vec3 light_ambient_incoming)
{
  vec3 light = direct_scale * getReflectedDirectLight(normal, light_direct_incoming)
            + ambient_scale * getReflectedAmbientLight(normal, light_ambient_incoming);

  return light;
}


vec3 sampleTerrainNormal()
{
  vec2 normal_map_coord = fract((pass_quad_center.xy + vec2(0, 200)) / map_size);
  normal_map_coord.y = 1.0 - normal_map_coord.y;
  vec3 normal = texture2D(sampler_terrain_normal_map, normal_map_coord).xyz;

  normal.y *= -1;

  return normal;
}


vec3 calcLight(vec3 normal)
{
  vec3 terrain_normal = sampleTerrainNormal();

  vec3 light_ambient_incoming;
  vec3 light_direct_incoming;
  getIncomingLight(passObjectPos, light_ambient_incoming, light_direct_incoming);

  vec3 directLightColor = light_direct_incoming * clamp(dot(normal, sunDir), 0.0, 2.0);
  directLightColor *= 1-pass_shadow;
  directLightColor *= 0.6;

  float direct_factor = 0.4 * (1-pass_shadow);
  float ambient_factor = 0.6;
  vec3 ambientLightColor = calcTerrainLight(terrain_normal, direct_factor, ambient_factor,
                                            light_direct_incoming, light_ambient_incoming);

  return directLightColor + ambientLightColor;
}


void main(void)
{
  vec3 normal = (texture2D(sampler_0, pass_texcoord_normal).xyz * 2) - vec3(1);
  normal = -normal;
  normal.z *= -1;
  normal = (view2WorldMatrix * vec4(normal, 0)).xyz;

  vec4 diffuse_color = texture2D(sampler_1, pass_texcoord_diffuse);
  diffuse_color.xyz = textureColorCorrection(diffuse_color.xyz);

  gl_FragColor = diffuse_color;

  gl_FragColor.xyz *= calcLight(normal);

  apply_fog();
}
