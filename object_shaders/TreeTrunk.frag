#version 130

void apply_fog();
vec3 calcLight(vec3 pos, vec3 normal, float direct_scale, float ambient_scale);

uniform sampler2D sampler_0;

uniform vec3 sunDir;
uniform bool is_shadow = false;

varying vec4 pass_texcoord;
varying vec3 pass_normal;

void main()
{
  gl_FragColor = texture2D(sampler_0, pass_texcoord.xy);

  if (is_shadow)
  {
    gl_FragColor.xyz = vec3(0);
    gl_FragColor.a *= 0.5 * smoothstep(-0.02, 0.02, sunDir.z);
  }
  else
  {
    gl_FragColor.xyz *= calcLight(vec3(0), pass_normal, 0.0, 0.8);
  }

  apply_fog();
}
