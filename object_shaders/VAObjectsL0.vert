#version 130

#extension GL_ARB_uniform_buffer_object : require

layout(std140) uniform local_vertex_parameters
{
  vec4 vertex_params[14];
};

uniform mat4 view2WorldMatrix;

varying vec3 pass_normal;
varying float pass_shinyness;
varying vec3 passObjectPos;
varying vec3 pass_specular_amount;
varying vec4 pass_color;

const int LIGHTING_PARAMS = 9;
const int ALPHA_PARAMS = 10;
const int SPECULAR_PARAMS = 11;

void il2_main()
{
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

  vec4 viewPos = gl_ModelViewMatrix * gl_Vertex;
  passObjectPos = (view2WorldMatrix * viewPos).xyz;

  vec4 normal = (gl_Color * 2) + vec4(-1, -1, -1, 0);
  normal.xyz = normalize(normal.xyz);
  normal.w = 0;

  normal.xyz = gl_NormalMatrix * normal.xyz;
  normal.xyz = (view2WorldMatrix * normal).xyz;

  pass_normal = normal.xyz;
  pass_normal = normalize(pass_normal);

  pass_shinyness = vertex_params[SPECULAR_PARAMS].w;
  pass_shinyness = clamp(pass_shinyness, 1, 128);

  pass_specular_amount = vertex_params[SPECULAR_PARAMS].xyz;

  float not_shadow = step(0.1, length(vertex_params[LIGHTING_PARAMS].xyz));
  pass_color.xyz = vec3(not_shadow);
  pass_color.w = vertex_params[ALPHA_PARAMS].w;

  gl_TexCoord[0].zw = (gl_MultiTexCoord0).zw;
  gl_TexCoord[0].xy = (gl_MultiTexCoord0.xy * vertex_params[12].zw) + vertex_params[12].xy;
}
