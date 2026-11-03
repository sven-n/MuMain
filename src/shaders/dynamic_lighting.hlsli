// dynamic_lighting.hlsli — the frame's point lights and the sun shadow map lookup, for the shaders
// that light per pixel (world_lit, skinned_lit). Include after scene_lighting.hlsli and after declaring
// sunShadowMap and sunShadowSampler. DynamicLights mirrors mu::DynamicLightsUniform (std140).
#ifndef MU_DYNAMIC_LIGHTING_HLSLI
#define MU_DYNAMIC_LIGHTING_HLSLI
static const uint MaxDynamicLights = 32; // mu::kMaxDynamicLights
cbuffer DynamicLights : register(b2, space3)
{
    uint dynamicLightCount;      // lights used
    float dynamicLightWrap;      // models only
    uint dynamicWorldLightCount; // the first lights: the ones the ground's vertex colour leaves out
    float groundShadowOpacity;  // how much a shadow darkens the vertex colour
    float4 dynamicLightPositionAndRadius[MaxDynamicLights];
    float4 dynamicLightColor[MaxDynamicLights];
};

// How much sun reaches `worldPosition` (Enhanced sun shadows): 1 lit, 0 in the shadow of what stands
// between it and the sun, softened over sunShadowParameters.y texels. 1 without a shadow map or
// outside it. `worldNormal` pushes the lookup off the surface so that it does not shadow itself.
float SunVisibility(float3 worldPosition, float3 worldNormal)
{
    if (sunShadowParameters.x <= 0.0)
        return 1.0;
    const float3 lifted = worldPosition + worldNormal * (1.5 * sunShadowParameters.w);
    const float4 clip = mul(sunViewProjection, float4(lifted, 1.0));
    const float3 ndc = clip.xyz / clip.w;
    const float2 uv = float2(ndc.x * 0.5 + 0.5, 0.5 - ndc.y * 0.5);
    if (any(uv < 0.0) || any(uv > 1.0) || ndc.z >= 1.0)
        return 1.0;
    const float2 step = sunShadowParameters.zz * sunShadowParameters.y;
    float visible = 0.0;
    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
            visible += sunShadowMap.SampleCmpLevelZero(sunShadowSampler, uv + float2(x, y) * step, ndc.z);
    }
    return visible / 9.0;
}

// How much of a point light reaches a point `toLight` away from it: the terrain light's linear fade
// over the horizontal distance.
float PointLightFalloff(float3 toLight, float radius)
{
    const float horizontal = length(toLight.xy);
    return horizontal < radius ? (radius - horizontal) / radius : 0.0;
}
#endif
