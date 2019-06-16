#version 130

#extension GL_ARB_uniform_buffer_object : require

vec3 textureColorCorrection(vec3 color);
vec3 calcLightWithSpecular(vec3 input_color, vec3 normal, float shinyness, vec3 specular_amount,
                           float direct_scale, float ambient_scale, vec3 viewDir);
void apply_fog();

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

const float DIRECT_LIGHT_SCALE = 1.1;
const float AMBIENT_LIGHT_SCALE = 0.8;

void main()
{
  vec3 view_dir = normalize(cameraPosWorld - passObjectPos);

  gl_FragColor = texture2D(sampler_0, (gl_TexCoord[0]).xy);
  gl_FragColor.xyz = textureColorCorrection(gl_FragColor.xyz);

  gl_FragColor.xyz = calcLightWithSpecular(gl_FragColor.xyz, pass_normal, pass_shinyness, pass_specular_amount, DIRECT_LIGHT_SCALE, AMBIENT_LIGHT_SCALE, view_dir);

  if (pass_color.a < 0.99)
  {
    gl_FragColor.xyz = vec3(0.0);
    gl_FragColor.a = 0.3 * smoothstep(-0.02, 0.02, sunDir.z);
  }

  apply_fog();
}
