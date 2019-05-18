#version 130

#extension GL_ARB_uniform_buffer_object : require

varying vec4 pass_texcoord;

layout(std140) uniform local_vertex_parameters
{
  vec4 vertex_params[6];
};


void il2_main()
{
  vec4 pos = gl_ModelViewProjectionMatrix * (vec4(gl_Vertex.xyz * 0.0156, 1) + vertex_params[5]);

  gl_Position = float(vertex_params[0].x < gl_SecondaryColor.z) * pos;

  pass_texcoord = (gl_Color * vec4(1,1,0,0)) + vec4(0,0,0,1);
}
