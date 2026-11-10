// world_lit.frag.hlsl — Enhanced per-pixel point lights for the ground: terrain tiles, their layers,
// grass and terrain decals that take the terrain light. Pairs with world_lit.vert.hlsl.
// The vertex colour is the terrain light without the frame's first dynamicWorldLightCount point
// lights; those are added back here per pixel with the terrain light's own falloff, so the ground
// gets the light it always did, smooth instead of interpolated between grid vertices.
// The shared uniforms and lighting functions: scene_lighting.hlsli, dynamic_lighting.hlsli.

Texture2D tex : register(t0, space2);
SamplerState s : register(s0, space2);
// The sun shadow map's depth (Enhanced sun shadows; a far 1x1 map without them).
Texture2D sunShadowMap : register(t2, space2);
SamplerComparisonState sunShadowSampler : register(s2, space2);
// Per pixel, 1 minus the darkest ground shadow there: red the sun's, green the point lights' (white
// without shadows).
Texture2D groundShadowMask : register(t1, space2);
SamplerState groundShadowSampler : register(s1, space2);
// Per terrain cell, the direct sunlight inside the vertex colour (the rest is the painted ambient light
// and point lights): what a cloud's shadow takes away. Used while lightingParameters.y is 1.
Texture2D terrainSunLight : register(t3, space2);
SamplerState terrainSunSampler : register(s3, space2);
#include "scene_lighting.hlsli"
#include "dynamic_lighting.hlsli"
#include "clouds.hlsli"
struct FSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : TEXCOORD1;
    float fogFactor : TEXCOORD2;
    float eyeDepth : TEXCOORD3;
    float3 worldPosition : TEXCOORD4;
    float3 darkening : TEXCOORD5;
};

// The frame's first point lights, which the ground's vertex colour leaves out.
float3 WorldDynamicLighting(float3 worldPosition)
{
    float3 light = float3(0.0, 0.0, 0.0);
    const uint count = min(min(dynamicWorldLightCount, dynamicLightCount), MaxDynamicLights);
    [loop]
    for (uint i = 0; i < count; ++i)
    {
        const float4 positionAndRadius = dynamicLightPositionAndRadius[i];
        light += dynamicLightColor[i].rgb * PointLightFalloff(positionAndRadius.xyz - worldPosition, positionAndRadius.w);
    }
    return light;
}

float4 main(FSInput input) : SV_Target
{
    // Sampled before the mask: spirv-cross numbers the MSL textures in the order the source uses them.
    const float4 texel = tex.Sample(s, input.uv);
    // The mask covers the render target; a 1x1 white mask clamps to white everywhere.
    uint maskWidth;
    uint maskHeight;
    groundShadowMask.GetDimensions(maskWidth, maskHeight);
    const float2 visibility =
        groundShadowMask.Sample(groundShadowSampler, input.pos.xy / float2(maskWidth, maskHeight)).rg;
    // The sun's shadows dim the ground's own light, the point lights' shadows the point lights.
    float2 shadow = 1.0 - groundShadowOpacity * (1.0 - visibility);
    const bool sunShadows = sunShadowParameters.x > 0.0;
    const float sunVisible = sunShadows ? SunVisibility(input.worldPosition, float3(0.0, 0.0, 1.0)) : 1.0;
    shadow.x *= 1.0 - sunShadowParameters.x * (1.0 - sunVisible);
    // A cloud takes away direct sunlight only, where the sun reaches.
    const float cloud = sunShadows ? CloudLight(input.worldPosition.xy, cloudParameters, cloudOffset.xy) : 1.0;
    float3 ground = input.color.rgb * shadow.x;
    if (cloud < 1.0 && lightingParameters.y > 0.0)
    {
        // Cell centres: the terrain's 256 cells of 100 world units (TERRAIN_SIZE, TERRAIN_SCALE).
        const float2 cellUv = (input.worldPosition.xy / 100.0 + 0.5) / 256.0;
        const float3 direct = terrainSunLight.Sample(terrainSunSampler, cellUv).rgb;
        ground = max(ground - direct * sunVisible * (1.0 - cloud), 0.0);
    }
    // Shade, from shadows and clouds, turns towards the sky's blue.
    const float3 skyShade = lerp(float3(1.0, 1.0, 1.0), skyFill.rgb, skyFill.w * (1.0 - sunVisible * cloud));
    // Darkening that outweighed the ground's own light takes from the point lights it was added after.
    const float3 pointLight = max(WorldDynamicLighting(input.worldPosition) * shadow.y + input.darkening, 0.0);
    const float3 light = saturate(ground * skyShade + pointLight);
    float4 color = float4(texel.rgb * light, texel.a * input.color.a);
    if (alphaToCoverage)
    {
        // Anti-aliased cutout, as in basic_textured.frag: coverage keeps the
        // texel's translucency; only the alpha test's step becomes a ramp.
        const float edge = saturate((color.a - alphaThreshold) / max(fwidth(color.a), 0.0001) + 0.5);
        color.a *= edge;
        if (color.a <= 0.0) discard;
    }
    else if (alphaDiscardEnabled && color.a <= alphaThreshold) discard;
    if (fogEnabled && sceneFogColor.w > 0.0)
        color.rgb = lerp(color.rgb, sceneFogColor.rgb, EnhancedFogAmount(input.eyeDepth));
    return color;
}
