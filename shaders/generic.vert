#version 130

// varying vec3 passViewPos;

// uniform mat4 projectionMatrixFar;
// uniform mat4 world2ViewMatrix;

void main(void)
{
//   passViewPos = (gl_ModelViewMatrix * gl_Vertex).xyz;
  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;

//   mat4 mvp = projectionMatrixFar * world2ViewMatrix;
//   mat4 mvp = projectionMatrixFar;


//   vec4 pos = gl_Vertex;
//   pos.xyz *= 1000;

//   gl_Position = mvp * pos;
}
