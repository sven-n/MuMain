#version 330 core

uniform sampler2D u_Tex;
uniform int       u_RenderMode;  // 0=TEXTURE, 1=CHROME, 2=BRIGHT

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
    if (u_RenderMode == 2) {
        fragColor = v_Color;
    } else {
        fragColor = texture(u_Tex, v_UV) * v_Color;
    }
    if (fragColor.a < 0.01) discard;

    if (u_FogEnabled > 0.5) {
        float depth = gl_FragCoord.z / gl_FragCoord.w;
        float fogFactor = clamp((u_FogEnd - depth) / (u_FogEnd - u_FogStart), 0.0, 1.0);
        fragColor.rgb = mix(u_FogColor.rgb, fragColor.rgb, fogFactor);
    }
}

