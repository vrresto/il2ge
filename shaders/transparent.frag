#version 130

#define USE_HDR @use_hdr:0@
#define USE_LUMINANCE @use_luminance:0@

#define ENABLE_WINDOW_GLARE 1


vec3 calcLight(vec3 pos, float direct_scale, float ambient_scale);
vec3 deGamma(vec3 color);
float deGamma(float color);
vec3 fogAndToneMap(vec3 color, bool no_inscattering);
void getIncomingLight(vec3 pos, out vec3 ambientLight, out vec3 directLight);
vec3 textureColorCorrection(vec3 color);

uniform sampler2D sampler_0;
uniform bool blend_add;
uniform bool texture_enabled;
uniform bool is_quad;

varying vec2 pass_texcoord;
varying vec4 pass_color;
varying vec3 passObjectPos;

#if USE_HDR
  const float MAX_ALBEDO = 0.5;
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
  bool is_sprite = is_quad;

  vec4 tex_color = texture(sampler_0, pass_texcoord);

  gl_FragColor.rgb = tex_color.rgb;
  gl_FragColor.a = pass_color.a * tex_color.a;

  vec3 incoming_direct_light;
  vec3 incoming_ambient_light;
  getIncomingLight(passObjectPos, incoming_ambient_light, incoming_direct_light);

//   if (!gl_FrontFacing)
//   {
//     // smoketrails
//     gl_FragColor.rgb = vec3(1,1,0);
//     gl_FragColor.a = 0.3;
//     return;
//   }

  if (!texture_enabled && blend_add)
  {
#if ENABLE_WINDOW_GLARE
   // window glare
    gl_FragColor.a = 1;

    float glare_intensity = length(pass_color.rgb) / length(vec3(1));

    gl_FragColor.rgb = incoming_direct_light * clamp(deGamma(vec3(glare_intensity)), 0, 1);

    gl_FragColor.rgb = fogAndToneMap(gl_FragColor.rgb, true);

    return;
#else
    discard;
#endif
  }

  float intensity = length(pass_color.rgb) / length(vec3(1));
  float direct_intensity = pass_color.r;

  float ambient_albedo_factor = deGamma(intensity);
  float direct_albedo_factor = deGamma(pass_color.r);

  vec3 tex_color_corrected = vec3(0);
  vec3 direct_albedo = vec3(0);
  vec3 ambient_albedo = vec3(0);

  if (is_sprite)
  {
    tex_color_corrected = deGamma(tex_color.rgb);

    #if USE_HDR
      direct_albedo_factor = pow(direct_albedo_factor, 3);
      ambient_albedo_factor = pow(ambient_albedo_factor, 3);
    #else
      direct_albedo_factor = pow(direct_albedo_factor, 2);
      ambient_albedo_factor = pow(ambient_albedo_factor, 2);
    #endif

    direct_albedo = MAX_ALBEDO * direct_albedo_factor * tex_color_corrected;
    ambient_albedo = MAX_ALBEDO * ambient_albedo_factor * tex_color_corrected;
  }
  else
  {
    tex_color_corrected = textureColorCorrection(tex_color.rgb);
    direct_albedo = tex_color_corrected * direct_albedo_factor;
    ambient_albedo = tex_color_corrected * ambient_albedo_factor;
  }

  vec3 ambient_reflection = ambient_albedo * incoming_ambient_light;
  vec3 direct_reflection = direct_albedo * incoming_direct_light;

  if (blend_add)
    gl_FragColor.rgb = MAX_FIRE_RADIANCE *  deGamma(pass_color.rgb) * tex_color_corrected;
  else
    gl_FragColor.rgb = direct_reflection + ambient_reflection;

  gl_FragColor.rgb = fogAndToneMap(gl_FragColor.rgb, blend_add);
}
