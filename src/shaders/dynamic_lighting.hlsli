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

// Twelve points spread evenly over the unit disc (a Poisson disc).
static const float2 SunShadowDisc[12] = {
    float2(-0.326, -0.406), float2(-0.840, -0.074), float2(-0.696, 0.457), float2(-0.203, 0.621),
    float2(0.962, -0.195), float2(0.473, -0.480), float2(0.519, 0.767), float2(0.185, -0.893),
    float2(0.507, 0.064), float2(0.896, 0.412), float2(-0.322, -0.933), float2(-0.792, -0.598),
};

// How much sun reaches `worldPosition` (Enhanced sun shadows): 1 lit, 0 in the shadow of what stands
// between it and the sun, softened over sunShadowParameters.y texels. 1 without a shadow map or
// outside it. `worldNormal` pushes the lookup off the surface so that it does not shadow itself,
// further on faces turned away from the sun, where a texel covers more of the surface.
float SunVisibility(float3 worldPosition, float3 worldNormal)
{
    if (sunShadowParameters.x <= 0.0)
        return 1.0;
    const float slope = 1.0 - saturate(dot(worldNormal, normalize(sunTowards.xyz)));
    const float3 lifted = worldPosition + worldNormal * ((1.5 + 2.0 * slope) * sunShadowParameters.w);
    const float4 clip = mul(sunViewProjection, float4(lifted, 1.0));
    const float3 ndc = clip.xyz / clip.w;
    const float2 uv = float2(ndc.x * 0.5 + 0.5, 0.5 - ndc.y * 0.5);
    if (any(uv < 0.0) || any(uv > 1.0) || ndc.z >= 1.0)
        return 1.0;
    // The disc turned by a per-place angle, so the samples' pattern becomes fine noise instead of
    // bands; nearer samples weigh more (Gaussian).
    const float turn = 6.2831853 * frac(52.9829189 * frac(dot(worldPosition.xy, float2(0.06711056, 0.00583715))));
    const float2x2 rotation = float2x2(cos(turn), -sin(turn), sin(turn), cos(turn));
    const float radius = sunShadowParameters.z * max(sunShadowParameters.y, 0.25) * 1.5;
    float visible = 0.0;
    float total = 0.0;
    [unroll]
    for (int i = 0; i < 12; ++i)
    {
        const float2 offset = mul(rotation, SunShadowDisc[i]);
        const float weight = exp(-2.0 * dot(offset, offset));
        visible += weight * sunShadowMap.SampleCmpLevelZero(sunShadowSampler, uv + offset * radius, ndc.z);
        total += weight;
    }
    return visible / total;
}

// How much of a point light reaches a point `toLight` away from it: the terrain light's linear fade
// over the horizontal distance.
float PointLightFalloff(float3 toLight, float radius)
{
    const float horizontal = length(toLight.xy);
    return horizontal < radius ? (radius - horizontal) / radius : 0.0;
}
#endif
