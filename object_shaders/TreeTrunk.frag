#version 130

#define IS_RENDER0 @is_render0@
vec3 textureColorCorrection(vec3 color);
void apply_fog();
vec3 calcLight(vec3 pos, vec3 normal, float direct_scale, float ambient_scale);

uniform sampler2D sampler_0;

uniform vec3 sunDir;

varying vec4 pass_texcoord;

void main()
{
  gl_FragColor = texture2D(sampler_0, pass_texcoord.xy);

#if IS_RENDER0
  gl_FragColor.xyz = vec3(0);
  gl_FragColor.a *= 0.5 * smoothstep(-0.02, 0.02, sunDir.z);
#else
  gl_FragColor.xyz = textureColorCorrection(gl_FragColor .xyz);
  gl_FragColor.xyz *= calcLight(vec3(0), vec3(0,0,1), 0.0, 0.8);
#endif

  apply_fog();
}
