#version 130

uniform mat4 view2WorldMatrix;

varying vec2 pass_texcoord;
varying vec3 passViewPos;
varying vec3 passObjectPos;
varying vec3 passObjectPosFlat;
varying vec4 pass_color;
varying vec4 pass_secondary_color;

void main(void)
{
  vec4 viewPos = gl_ModelViewMatrix * gl_Vertex;
  passViewPos = viewPos.xyz;
  passObjectPos = (view2WorldMatrix * viewPos).xyz;
  passObjectPosFlat = passObjectPos;

  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

  pass_color = gl_Color;
  pass_secondary_color = gl_SecondaryColor;

  pass_texcoord = (gl_TextureMatrix[0] * gl_MultiTexCoord0).xy;
}
