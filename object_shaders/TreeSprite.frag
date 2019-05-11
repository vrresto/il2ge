#version 130

vec3 calcIncomingDirectLight();
void calcLightParams(vec3 normal, out vec3 ambientLightColor, out vec3 directLightColor);
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


vec3 calcTerrainLight(vec3 pos, vec3 normal, float direct_scale, float ambient_scale)
{
  vec3 ambientLightColor;
  vec3 directLightColor;
  calcLightParams(normal, ambientLightColor, directLightColor);

  vec3 light = direct_scale * directLightColor + ambient_scale * ambientLightColor;

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

  float directLight = clamp(dot(normal, sunDir), 0.0, 2.0);

  directLight *= 1-pass_shadow;

  vec3 directLightColor = calcIncomingDirectLight() * directLight;
  directLightColor *= 0.7;

  float direct_factor = 0.5 * (1-pass_shadow);
  float ambient_factor = 0.8;
  vec3 ambientLightColor = calcTerrainLight(vec3(0), terrain_normal, direct_factor, ambient_factor);

  return directLightColor + ambientLightColor;
}


void main(void)
{
  vec3 normal = (texture2D(sampler_0, gl_TexCoord[0].xy).xyz * 2) - vec3(1);
  normal = -normal;
  normal.z *= -1;
  normal = (view2WorldMatrix * vec4(normal, 0)).xyz;

  vec4 diffuse_color = texture2D(sampler_1, gl_TexCoord[1].xy);

  gl_FragColor = diffuse_color;

  gl_FragColor.xyz *= calcLight(normal);

  apply_fog();
}
