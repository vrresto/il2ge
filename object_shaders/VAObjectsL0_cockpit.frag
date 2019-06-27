#version 130

#extension GL_ARB_uniform_buffer_object : require

#define USE_LUMINANCE @use_luminance:0@
#define USE_HDR @use_hdr:0@

vec3 textureColorCorrection(vec3 color);
vec3 calcLightWithSpecular(vec3 input_color, vec3 normal, float shinyness, vec3 specular_amount,
                           float direct_scale, float ambient_scale, vec3 viewDir);
void apply_fog();
vec3 toneMap(vec3 color);
vec3 deGamma(vec3 color);

uniform sampler2D sampler_0;
uniform sampler2D sampler_1;

uniform vec3 cameraPosWorld;
uniform vec3 sunDir;
uniform mat4 world2ViewMatrix;
uniform mat4 view2WorldMatrix;

uniform bool blend_add = false;
uniform bool blend = false;

varying vec3 passObjectPos;
varying vec3 pass_normal;
varying float pass_shinyness;
varying vec3 pass_specular_amount;
varying vec4 pass_color;
varying vec4 pass_secondary_color;
varying float pass_ambient_brightness;


const float DIRECT_LIGHT_SCALE = 1.0;
const float AMBIENT_LIGHT_SCALE = 1.0;

#if USE_HDR
  #if USE_LUMINANCE
    const float MAX_RETICLE_RADIANCE = 4000;
  #else
    const float MAX_RETICLE_RADIANCE = 0.2;
  #endif
#else
  const float MAX_RETICLE_RADIANCE = 1.0;
#endif


void main()
{
  vec3 view_dir = normalize(cameraPosWorld - passObjectPos);

  //EVIL HACK FIXME
  bool is_instrument_light = false;
  if (blend && !blend_add
        && (distance(pass_secondary_color.xyz, vec3(0.0, 1.0, 0.5)) < 0.1
              || pass_ambient_brightness > 0.4))
  {
    is_instrument_light = true;
  }

  //EVIL HACK FIXME
  bool is_light_source = !is_instrument_light && blend_add && pass_ambient_brightness > 0.3;

  vec4 texture_color = texture2D(sampler_0, (gl_TexCoord[0]).xy);

  vec3 texture_color_corrected = textureColorCorrection(texture_color.xyz);

  texture_color.xyz = deGamma(texture_color.xyz);

  vec3 radiance = vec3(0);

  float alpha = texture_color.a * pass_color.a;

  vec3 normal = pass_normal;

  if (!is_light_source)
  {
    vec3 specular_amount = pass_specular_amount;
    specular_amount = vec3(0);

    radiance += calcLightWithSpecular(texture_color_corrected, normal, pass_shinyness,
        specular_amount, DIRECT_LIGHT_SCALE, AMBIENT_LIGHT_SCALE, view_dir);
  }

  if (is_instrument_light || is_light_source)
  {
    radiance += MAX_RETICLE_RADIANCE * texture_color.xyz * pass_secondary_color.xyz;
  }

  gl_FragColor.xyz = toneMap(radiance);
  gl_FragColor.a = alpha;
}
