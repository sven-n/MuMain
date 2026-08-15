#version 330 core

#include "global_ubo.glsl"

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in vec4 a_Color;

out vec2 v_UV;
out vec4 v_Color;

void main() {
    gl_Position = u_MVP * vec4(a_Pos, 1.0);
    v_UV    = a_UV;
    v_Color = a_Color;
}
