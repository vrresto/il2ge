#version 130

#extension GL_ARB_uniform_buffer_object : require

vec3 calcLightWithSpecular(vec3 input, vec3 normal, float shinyness, vec3 specular_amount, vec3 viewDir);
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


void main()
{
  gl_FragColor = texture2D(sampler_0, (gl_TexCoord[0]).xy);

  vec3 view_dir = normalize(cameraPosWorld - passObjectPos);

  gl_FragColor.xyz = calcLightWithSpecular(gl_FragColor.xyz,
    pass_normal, pass_shinyness, pass_specular_amount, view_dir);

  gl_FragColor *= pass_color;

  apply_fog();
}
