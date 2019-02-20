#version 130

uniform sampler2D sampler_0;
uniform bool is_alpha_texture = false;

varying vec2 pass_texcoord;

void main(void)
{
  vec4 color = texture(sampler_0, pass_texcoord);

  gl_FragColor = gl_Color;

  if (is_alpha_texture)
  {
    gl_FragColor.a *= color.x;
  }
  else
  {
    gl_FragColor *= color;
  }
}
