#version 130

vec3 calcLight(vec3 pos, vec3 normal, float direct_scale, float ambient_scale);
void apply_fog();

uniform sampler2D sampler_0;
uniform sampler2D sampler_terrain_normal_map;

uniform vec3 sunDir;
uniform vec2 map_size;

varying vec3 passObjectPos;
varying vec3 pass_normal;


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
  gl_FragColor = texture2D(sampler_0, (gl_TexCoord[0]).xy);

  vec3 normal = sampleTerrainNormal();
  gl_FragColor.xyz *= calcLight(vec3(0), normal, 1, 1);

  apply_fog();
}
