#version 130

#define USE_HDR @use_hdr:0@
#define USE_LUMINANCE @use_luminance:0@

vec3 calcLight(vec3 pos, float direct_scale, float ambient_scale);
vec3 deGamma(vec3 color);
vec3 fogAndToneMap(vec3 color, bool no_inscattering);

uniform sampler2D sampler_0;
uniform bool blend_add;
uniform bool alpha_texture;

varying vec2 pass_texcoord;
varying vec4 pass_color;
varying vec4 pass_secondary_color;
varying vec3 passObjectPos;

const float DIRECT_SCALE = 0.8;
const float AMBIENT_SCALE = 0.6;

#if USE_HDR
  const float MAX_ALBEDO = 0.3;
  #if USE_LUMINANCE
    const float MAX_FIRE_RADIANCE = 8000;
  #else
    const float MAX_FIRE_RADIANCE = 0.2;
  #endif
#else
  const float MAX_ALBEDO = 0.9;
  const float MAX_FIRE_RADIANCE = 1.0;
#endif


void main(void)
{
  vec4 tex_color = texture(sampler_0, pass_texcoord);

  if (alpha_texture)
  {
    gl_FragColor.xyz = pass_color.xyz;
    gl_FragColor.a = tex_color.r;
  }
  else
  {
    gl_FragColor = pass_color * tex_color;
  }

  gl_FragColor.xyz = deGamma(gl_FragColor.xyz);

#if USE_HDR
  if (!blend_add)
    gl_FragColor.xyz = pow(gl_FragColor.xyz, vec3(1.5));
#endif

  vec3 light = MAX_ALBEDO * calcLight(passObjectPos, DIRECT_SCALE, AMBIENT_SCALE);

  if (blend_add)
    gl_FragColor.xyz *= MAX_FIRE_RADIANCE;
  else
    gl_FragColor.xyz *= light;

  gl_FragColor.xyz = fogAndToneMap(gl_FragColor.xyz, blend_add);
}
