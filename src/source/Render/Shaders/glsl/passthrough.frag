#version 330 core

uniform sampler2D u_Tex;
uniform int u_UseTexture;
uniform int u_UseFog;

in vec2 v_UV;
in vec4 v_Color;

out vec4 fragColor;

layout(std140) uniform SceneData {
    vec4  u_FogColor;
    float u_FogStart;
    float u_FogEnd;
    float u_FogEnabled;
    float u_ScenePadding;
};

void main() {
    vec4 texSample = (u_UseTexture != 0) ? texture(u_Tex, v_UV) : vec4(1.0);
    fragColor = texSample * v_Color;

    if (u_UseFog != 0 && u_FogEnabled > 0.5) {
        float depth = gl_FragCoord.z / gl_FragCoord.w;
        float fogFactor = clamp((u_FogEnd - depth) / (u_FogEnd - u_FogStart), 0.0, 1.0);
        fragColor.rgb = mix(u_FogColor.rgb, fragColor.rgb, fogFactor);
    }
}

