#version 130

#extension GL_ARB_uniform_buffer_object : require

#define USE_HDR @use_hdr:0@

#include lighting_definitions.glsl

vec3 textureColorCorrection(vec3 color);
vec3 fogAndToneMap(vec3);
vec3 deGamma(vec3 color);

uniform sampler2D sampler_0;

layout(std140) uniform local_fragment_parameters
{
  vec4 params[4];
};

uniform vec3 cameraPosWorld;
uniform vec3 sunDir;
uniform mat4 world2ViewMatrix;
uniform mat4 view2WorldMatrix;

varying vec3 passObjectPos;
varying vec3 pass_normal;
varying float pass_shinyness;
varying vec3 pass_specular_amount;
varying vec4 pass_color;
varying vec2 pass_texcoord;

#if USE_HDR
const float DIRECT_LIGHT_SCALE = 1.0;
const float AMBIENT_LIGHT_SCALE = 1.0;
#else
const float DIRECT_LIGHT_SCALE = 1.1;
const float AMBIENT_LIGHT_SCALE = 0.8;
#endif


vec3 getSpecular(vec3 view_dir, vec3 incoming)
{
  vec3 R = reflect(view_dir, pass_normal);
  vec3 lVec = -sunDir;

  vec3 specular_amount = 0.5 * pass_specular_amount * pow(max(dot(R, lVec), 0.0), pass_shinyness);

  return deGamma(specular_amount) * incoming;
}


void main()
{
  vec3 view_dir = normalize(cameraPosWorld - passObjectPos);

  gl_FragColor = texture2D(sampler_0, pass_texcoord);
  gl_FragColor.xyz = textureColorCorrection(gl_FragColor.xyz);

  vec3 light_ambient_incoming;
  vec3 light_direct_incoming;
  getIncomingLight(passObjectPos, light_ambient_incoming, light_direct_incoming);

  vec3 light_direct = getReflectedDirectLight(pass_normal, light_direct_incoming);
  vec3 light_ambient = getReflectedAmbientLight(pass_normal, light_ambient_incoming);

  light_direct *= DIRECT_LIGHT_SCALE;
  light_ambient *= AMBIENT_LIGHT_SCALE;

  vec3 light_specular = getSpecular(view_dir, light_direct_incoming);

  gl_FragColor.xyz = gl_FragColor.xyz * (light_direct + light_ambient) + light_specular;

  if (pass_color.a < 0.99)
  {
    gl_FragColor.xyz = vec3(0.0);
    gl_FragColor.a = 0.3 * smoothstep(-0.02, 0.02, sunDir.z);
  }

  gl_FragColor.xyz = fogAndToneMap(gl_FragColor.xyz);
}
