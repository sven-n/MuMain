// scene_lighting.hlsli — the fog and scene lighting uniforms every Enhanced lit shader shares, and the
// Enhanced fog curve. FogUniforms mirrors FogUniform, SceneLighting mu::SceneLightingUniform (std140).
#ifndef MU_SCENE_LIGHTING_HLSLI
#define MU_SCENE_LIGHTING_HLSLI
cbuffer FogUniforms : register(b0, space3)
{
    uint fogEnabled;
    uint alphaDiscardEnabled;
    float alphaThreshold;
    uint alphaToCoverage; // 1: the pipeline turns alpha into multisample coverage
    float fogStart;
    float fogEnd;
    float4 fogColor;
};
cbuffer SceneLighting : register(b1, space3)
{
    float4 sceneLightDirection;
    float4 sceneLightColor;
    float4 diffuseCurve; // scale, ambient, floor, highlight knee
    float4 rimParameters;
    float4 specularParameters;
    float4 sceneFogColor;
    float4 sceneFogParameters;
    float4 cameraPosition;
    float4x4 sunViewProjection; // world to the sun shadow map's clip space
    float4 sunShadowParameters; // darkness (0: no shadow map), softness in texels, 1 / map size, texel in world units
};

// The Enhanced exponential fog: the same curve as mu::EnhancedFogAmount() (SceneLighting.cpp).
float EnhancedFogAmount(float eyeDepth)
{
    const float scaledDistance = max(eyeDepth - sceneFogParameters.x, 0.0) * sceneFogParameters.z;
    return saturate(1.0 - exp(-scaledDistance * scaledDistance));
}
#endif
