#version 330 core

layout(std140) uniform GlobalMatrices {
    mat4 u_View;
    mat4 u_Proj;
    mat4 u_Model;
    mat4 u_MVP;
    vec4 u_Time;
};

// Bone matrix palette UBO at binding slot 2
layout(std140) uniform BoneMatrices {
    mat4 u_Bones[200];
};

layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec2 a_UV;
layout(location = 2) in vec4 a_Color;
layout(location = 3) in vec3 a_Normal;
layout(location = 4) in int  a_BoneIndex;

uniform mat4  u_MVPDraw;     // per-draw MVP from current GL matrix state
uniform int   u_UseGPUSkin;  // 1 = skin via u_Bones[a_BoneIndex], 0 = pass-through (dynamic VBO)
uniform int   u_RenderMode;
uniform float u_WaveOffsetU;
uniform float u_WaveOffsetV;
uniform vec3  u_BodyOrigin;  // world-space placement of model root (applied after bone transform)
uniform float u_BodyScale;   // uniform body scale (applied after bone transform)

out vec2 v_UV;
out vec4 v_Color;

void main() {
    if (u_UseGPUSkin == 1 && a_BoneIndex >= 0 && a_BoneIndex < 200) {
        // u_Bones[a_BoneIndex] transforms rest-pose position to skeleton-local space.
        // u_BodyOrigin + u_BodyScale * bonePos replicates BMD::Transform VectorMA:
        //   VectorMA(BodyOrigin, BodyScale, bonePos, VertexTransform)
        // This produces the correct world-space position that u_MVPDraw (camera VP) expects.
        vec4 bonePos = u_Bones[a_BoneIndex] * vec4(a_Pos, 1.0);
        vec3 worldPos = u_BodyOrigin + u_BodyScale * bonePos.xyz;
        gl_Position = u_MVPDraw * vec4(worldPos, 1.0);
    } else {
        // CPU pre-transformed position (dynamic VBO fallback / stream mesh)
        // VertexTransform[] already has BodyOrigin+BodyScale applied on CPU side.
        gl_Position = u_MVPDraw * vec4(a_Pos, 1.0);
    }
    v_Color = a_Color;
    if (u_RenderMode == 0) {
        v_UV = a_UV + vec2(u_WaveOffsetU, u_WaveOffsetV);
    } else {
        v_UV = a_UV;
    }
}
