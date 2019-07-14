#version 130

#define IS_RENDER0 @is_render0@

#include lighting_definitions.glsl

vec3 textureColorCorrection(vec3 color);
vec3 apply_fog(vec3);

uniform sampler2D sampler_0;

uniform vec3 sunDir;

varying vec4 pass_texcoord;
varying vec3 passObjectPos;


void main()
{
  gl_FragColor = texture2D(sampler_0, pass_texcoord.xy);

#if IS_RENDER0
  gl_FragColor.xyz = vec3(0);
  gl_FragColor.a *= 0.5 * smoothstep(-0.02, 0.02, sunDir.z);
#else
  gl_FragColor.xyz = textureColorCorrection(gl_FragColor.xyz);

  vec3 light_ambient_incoming;
  vec3 light_direct_incoming;
  getIncomingLight(passObjectPos, light_ambient_incoming, light_direct_incoming);

  gl_FragColor.xyz *= 0.8 * getReflectedAmbientLight(vec3(0,0,1), light_ambient_incoming);
#endif

  gl_FragColor.xyz = apply_fog(gl_FragColor.xyz);
}
