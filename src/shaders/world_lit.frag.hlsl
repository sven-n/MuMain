// world_lit.frag.hlsl — Enhanced per-pixel point lights for the ground: terrain tiles, their layers,
// grass and terrain decals that take the terrain light. Pairs with world_lit.vert.hlsl.
// The vertex colour is the terrain light without the frame's first dynamicWorldLightCount point
// lights; those are added back here per pixel with the terrain light's own falloff, so the ground
// gets the light it always did, smooth instead of interpolated between grid vertices.
// FogUniforms mirrors FogUniform, SceneLighting mu::SceneLightingUniform and DynamicLights
// mu::DynamicLightsUniform (std140).

Texture2D tex : register(t0, space2);
SamplerState s : register(s0, space2);
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
    float4 diffuseCurve;
    float4 rimParameters;
    float4 specularParameters;
    float4 sceneFogColor;
    float4 sceneFogParameters;
    float4 cameraPosition;
};
static const uint MaxDynamicLights = 32; // mu::kMaxDynamicLights
cbuffer DynamicLights : register(b2, space3)
{
    uint dynamicLightCount;      // lights used
    float dynamicLightWrap;      // models only
    uint dynamicWorldLightCount; // the first lights: the ones the ground's vertex colour leaves out
    uint dynamicLightPadding;
    float4 dynamicLightPositionAndRadius[MaxDynamicLights];
    float4 dynamicLightColor[MaxDynamicLights];
};
struct FSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : TEXCOORD1;
    float fogFactor : TEXCOORD2;
    float eyeDepth : TEXCOORD3;
    float3 worldPosition : TEXCOORD4;
};

// Same curve as basic_textured_fog.frag.hlsl and mu::EnhancedFogAmount() (SceneLighting.cpp).
float EnhancedFogAmount(float eyeDepth)
{
    const float scaledDistance = max(eyeDepth - sceneFogParameters.x, 0.0) * sceneFogParameters.z;
    return saturate(1.0 - exp(-scaledDistance * scaledDistance));
}

// The terrain light's linear falloff over the horizontal distance, as the grid adds it.
float3 WorldDynamicLighting(float3 worldPosition)
{
    float3 light = float3(0.0, 0.0, 0.0);
    const uint count = min(min(dynamicWorldLightCount, dynamicLightCount), MaxDynamicLights);
    [loop]
    for (uint i = 0; i < count; ++i)
    {
        const float4 positionAndRadius = dynamicLightPositionAndRadius[i];
        const float radius = positionAndRadius.w;
        const float horizontalDistance = length(positionAndRadius.xy - worldPosition.xy);
        if (horizontalDistance < radius)
            light += dynamicLightColor[i].rgb * ((radius - horizontalDistance) / radius);
    }
    return light;
}

float4 main(FSInput input) : SV_Target
{
    const float3 light = saturate(input.color.rgb + WorldDynamicLighting(input.worldPosition));
    const float4 texel = tex.Sample(s, input.uv);
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
