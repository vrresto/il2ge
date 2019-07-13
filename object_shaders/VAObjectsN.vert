#version 130

#extension GL_ARB_uniform_buffer_object : require

uniform mat4 view2WorldMatrix;

varying vec3 passObjectPos;
varying vec3 passObjectPosFlat;

layout(std140) uniform local_vertex_parameters
{
  vec4 vertex_params[90];
};

varying vec3 pass_normal;
varying float pass_shinyness;
varying vec3 pass_specular_amount;
varying vec4 pass_color;
varying vec2 pass_texcoord;

const int LIGHTING_PARAMS = 9;
const int ALPHA_PARAMS = 10;
const int INSTANCE_DATA_START = 18;
const int INSTANCE_DATA_SIZE = 3;

void il2_main()
{
  pass_texcoord = gl_MultiTexCoord0.xy;

  int instance_offset = INSTANCE_DATA_START + (int(floor(gl_Color.w * 32)) * INSTANCE_DATA_SIZE);

  mat4 model_rotation =
    transpose(mat4(vertex_params[instance_offset+0],
                   vertex_params[instance_offset+1],
                   vertex_params[instance_offset+2],
                   vec4(0,0,0,1)));

  vec4 model_pos = model_rotation * gl_Vertex;

  vec4 viewPos = gl_ModelViewMatrix * model_pos;

  passObjectPos = (view2WorldMatrix * viewPos).xyz;
  passObjectPosFlat = passObjectPos;

  gl_Position = gl_ModelViewProjectionMatrix * model_pos;

  vec4 normal = (gl_Color * 2) + vec4(-1, -1, -1, 0);
  normal.xyz = normalize(normal.xyz);
  normal.w = 0;

  normal = model_rotation * normal;

  pass_normal = normal.xyz;
  pass_normal = normalize(pass_normal);

  pass_shinyness = 1;
  pass_specular_amount = vec3(0);

  float not_shadow = step(0.1, length(vertex_params[LIGHTING_PARAMS].xyz));
  pass_color.xyz = vec3(not_shadow);
  pass_color.w = vertex_params[ALPHA_PARAMS].w;
}
