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
varying vec3 passObjectPosFlat;
varying vec3 pass_specular_amount;
varying vec4 pass_color;
varying vec4 pass_secondary_color;
varying float pass_ambient_brightness;

const int PARAM_LIGHTING = 9;
const int PARAM_COLOR = 10;
const int PARAM_SPECULAR = 11;
const int PARAM_SECONDARY_COLOR = 13;

void il2_main()
{
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

  vec4 viewPos = gl_ModelViewMatrix * gl_Vertex;
  passObjectPos = (view2WorldMatrix * viewPos).xyz;
  passObjectPosFlat = passObjectPos;

  vec4 normal = (gl_Color * 2) + vec4(-1, -1, -1, 0);
  normal.xyz = normalize(normal.xyz);
  normal.w = 0;

  normal.xyz = gl_NormalMatrix * normal.xyz;
  normal.xyz = (view2WorldMatrix * normal).xyz;

  pass_normal = normal.xyz;
  pass_normal = normalize(pass_normal);

  pass_shinyness = vertex_params[PARAM_SPECULAR].w;
  pass_shinyness = clamp(pass_shinyness, 1, 128);

  pass_specular_amount = vertex_params[PARAM_SPECULAR].xyz;

  float not_shadow = step(0.1, length(vertex_params[PARAM_LIGHTING].xyz));
  pass_color.xyz = vec3(not_shadow);
  pass_color.w = vertex_params[PARAM_COLOR].w * vertex_params[PARAM_SECONDARY_COLOR].w;

  pass_secondary_color = vertex_params[PARAM_SECONDARY_COLOR];

  pass_ambient_brightness = vertex_params[PARAM_LIGHTING].w;

  gl_TexCoord[0].zw = (gl_MultiTexCoord0).zw;
  gl_TexCoord[0].xy = (gl_MultiTexCoord0.xy * vertex_params[12].zw) + vertex_params[12].xy;
}
