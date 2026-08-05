#version 330 core

in vec2 v_UV;
in vec3 v_Light;
in float v_Alpha;

uniform sampler2D u_BaseTex;
uniform sampler2D u_OverlayTex;

out vec4 FragColor;

layout(std140) uniform SceneData {
    vec4  u_FogColor;
    float u_FogStart;
    float u_FogEnd;
    float u_FogEnabled;
    float u_ScenePadding;
};

void main() {
    vec4 texBase = texture(u_BaseTex, v_UV);
    vec4 texOverlay = texture(u_OverlayTex, v_UV);
    vec4 blendedTex = mix(texBase, texOverlay, clamp(v_Alpha, 0.0, 1.0));
    FragColor = vec4(blendedTex.rgb * v_Light, blendedTex.a);

    if (u_FogEnabled > 0.5) {
        float depth = gl_FragCoord.z / gl_FragCoord.w;
        float fogFactor = clamp((u_FogEnd - depth) / (u_FogEnd - u_FogStart), 0.0, 1.0);
        FragColor.rgb = mix(u_FogColor.rgb, FragColor.rgb, fogFactor);
    }
}

