#version 130

#extension GL_ARB_uniform_buffer_object : require

layout(std140) uniform local_vertex_parameters
{
  vec4 vertex_params[6];
};


void il2_main()
{
  gl_Position = gl_Vertex;
}
