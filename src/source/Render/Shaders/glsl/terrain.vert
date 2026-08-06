#version 330 core

layout(std140) uniform GlobalMatrices {
    mat4 u_View;
    mat4 u_Proj;
    mat4 u_Model;
    mat4 u_MVP;
    vec4 u_Time;
};

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Light;
layout(location = 2) in float a_Alpha;

out vec2 v_UV;
out vec3 v_Light;
out float v_Alpha;

void main() {
    v_UV = a_Pos.xy * 0.0025;
    v_Light = a_Light;
    v_Alpha = a_Alpha;
    gl_Position = u_MVP * vec4(a_Pos, 1.0);
}
