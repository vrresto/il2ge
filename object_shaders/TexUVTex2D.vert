#version 130

#extension GL_ARB_uniform_buffer_object : require

layout(std140) uniform local_vertex_parameters
{
  vec4 vertex_params[16];
};

varying vec3 pass_normal;
uniform mat4 view2WorldMatrix;


void il2_main()
{
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

  gl_TexCoord[0] = gl_MultiTexCoord0;

  vec3 normal = gl_NormalMatrix * gl_Normal;
  normal = (view2WorldMatrix * vec4(normal, 0)).xyz;

  pass_normal = normalize(normal);
}
