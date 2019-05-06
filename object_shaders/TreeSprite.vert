#version 130

#extension GL_ARB_uniform_buffer_object : require

uniform mat4 view2WorldMatrix;

varying vec3 passObjectPos;
varying vec3 passObjectPosFlat;

layout(std140) uniform local_vertex_parameters
{
  vec4 params[21];
};


void il2_main()
{
  vec3 quad_center_model = gl_Vertex.xyz * 0.0156;

  vec4 quad_center_view = gl_ModelViewMatrix * vec4(quad_center_model + params[19].xyz, 1);

  vec4 view_pos = quad_center_view;

  float scale = gl_Color.y * params[4].y;
  float distance_scale = 1;

  {
    // distance scale

    int idx = int(floor((gl_SecondaryColor.x * 8)));

    vec4 scale_func_coefficients0 = params[idx+5];
    vec4 scale_func_coefficients1 = params[idx+6];

    float dist_from_camera = length(quad_center_view.xyz);

    float scale_func_arg = pow(dist_from_camera, 2) * gl_SecondaryColor.y * 4;

    distance_scale = scale_func_coefficients1.x;
    distance_scale += scale_func_arg * scale_func_coefficients0.w;
    distance_scale += pow(scale_func_arg, 2) * scale_func_coefficients0.z;
    distance_scale += pow(scale_func_arg, 3) * scale_func_coefficients0.y;
    distance_scale += pow(scale_func_arg, 4) * scale_func_coefficients0.x;

    distance_scale = clamp(distance_scale, 0, 4);
  }

  vec4 vertex_params = params[int(floor(gl_Color.x * params[4].x))];

  view_pos.xy = (vertex_params.xy * scale * distance_scale) + quad_center_view.xy;

  passObjectPos = (view2WorldMatrix * view_pos).xyz;
  passObjectPosFlat = passObjectPos;

  gl_Position = gl_ProjectionMatrix * view_pos;

  {
    // texcoords

    vec4 texture_params = params[int(floor((fract(scale + pow(length(quad_center_model), 2))) * 4))];

    gl_TexCoord[0].xy = vec2(0.5) + 0.25 * vertex_params.zw + 0.25 * texture_params.xy;
    gl_TexCoord[0].zw = vec2(0, 1);

    gl_TexCoord[1].xy = gl_Color.zw;
    gl_TexCoord[1].zw = vec2(0, 1);

    gl_TexCoord[2] = gl_TexCoord[1];
  }

}
