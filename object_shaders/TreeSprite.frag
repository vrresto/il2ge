#version 130

void calcLightParams(vec3 normal, out vec3 ambientLightColor, out vec3 directLightColor);
void apply_fog();

uniform mat4 view2WorldMatrix;
uniform vec3 sunDir = vec3(0,0,1);

uniform sampler2D sampler_0;
uniform sampler2D sampler_1;


vec3 calcLight(vec3 normal)
{
  vec3 ambientLightColor;
  vec3 directLightColor;
  calcLightParams(normal, ambientLightColor, directLightColor);

  directLightColor *= 0.5;
  ambientLightColor *= 1.0;

  return directLightColor + ambientLightColor;
}


void main(void)
{
  vec3 normal = (texture2D(sampler_0, gl_TexCoord[0].xy).xyz * 2) - vec3(1);
  normal = -normal;
  normal.z *= -1;
  normal = (view2WorldMatrix * vec4(normal, 0)).xyz;

  vec4 diffuse_color = texture2D(sampler_1, gl_TexCoord[1].xy);

  gl_FragColor.xyz = diffuse_color.xyz;
  gl_FragColor.a = diffuse_color.a;

  gl_FragColor.xyz *= calcLight(normal);

  apply_fog();
}
