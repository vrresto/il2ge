#version 130

uniform mat4 view2WorldMatrix;

varying vec3 passViewPos;
varying vec3 passObjectPos;
varying vec3 passObjectPosFlat;

void il2_main();

void main(void)
{
    vec4 viewPos = gl_ModelViewMatrix * gl_Vertex;
    passViewPos = viewPos.xyz;
    passObjectPos = (view2WorldMatrix * viewPos).xyz;
    passObjectPosFlat = passObjectPos;

    il2_main();
}
