#version 130

#extension GL_ARB_uniform_buffer_object : require

uniform mat4 view2WorldMatrix;

varying vec4 pass_texcoord;
varying vec3 passObjectPos;
varying vec3 passObjectPosFlat;

layout(std140) uniform local_vertex_parameters
{
  vec4 vertex_params[6];
};


void il2_main()
{
  vec4 view_pos = gl_ModelViewMatrix * (vec4(gl_Vertex.xyz * 0.0156, 1) + vertex_params[5]);

  passObjectPos = (view2WorldMatrix * view_pos).xyz;
  passObjectPosFlat = passObjectPos;

  gl_Position = gl_ProjectionMatrix * view_pos;
  gl_Position *= float(vertex_params[0].x < gl_SecondaryColor.z);

  pass_texcoord = (gl_Color * vec4(1,1,0,0)) + vec4(0,0,0,1);
}
