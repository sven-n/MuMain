// post_ao_blur.frag.hlsl — post_ao.frag's occlusion averaged over 4x4 texels, the size of its
// rotation pattern, leaving out texels off the centre's surface (an edge) so edges keep no halo.
// Texels are weighed by their distance from the centre's plane, not by depth: ground far away, whose
// depth changes fast down the screen, still blends fully and the pattern cancels there too.
// AmbientOcclusion mirrors mu::SdlGpuAmbientOcclusionUniform (only some of it is read here).
Texture2D depthTexture : register(t0, space2);
SamplerState depthSampler : register(s0, space2);         // point
Texture2D occlusionTexture : register(t1, space2);
SamplerState occlusionSampler : register(s1, space2);     // point
cbuffer AmbientOcclusion : register(b0, space3)
{
    float4x4 projection;
    float4x4 inverseProjection;
    float2 depthTexelSize;
    float radius;
    float padding;
};
struct FSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

// At a depth texel's centre, as post_ao.frag's ViewPositionAt.
float3 ViewPosition(float2 uv)
{
    uv = (floor(uv / depthTexelSize + 0.25) + 0.5) * depthTexelSize;
    const float depth = depthTexture.SampleLevel(depthSampler, uv, 0).r;
    const float4 view = mul(inverseProjection, float4(uv.x * 2.0 - 1.0, 1.0 - uv.y * 2.0, depth, 1.0));
    return view.xyz / view.w;
}

// As post_ao.frag's SurfaceNormal: from the nearer neighbour on each axis.
float3 SurfaceNormal(float2 uv, float3 position)
{
    const float3 right = ViewPosition(uv + float2(depthTexelSize.x, 0.0)) - position;
    const float3 left = position - ViewPosition(uv - float2(depthTexelSize.x, 0.0));
    const float3 down = ViewPosition(uv + float2(0.0, depthTexelSize.y)) - position;
    const float3 up = position - ViewPosition(uv - float2(0.0, depthTexelSize.y));
    const float3 alongX = abs(right.z) < abs(left.z) ? right : left;
    const float3 alongY = abs(down.z) < abs(up.z) ? down : up;
    return normalize(cross(alongX, alongY));
}

float4 main(FSInput input) : SV_Target
{
    const float3 centre = ViewPosition(input.uv);
    const float3 normal = SurfaceNormal(input.uv, centre);
    // The occlusion is at half the depth's resolution.
    const float2 occlusionTexelSize = depthTexelSize * 2.0;
    // Texels further than this from the centre's plane are another surface.
    const float planeTolerance = radius * 0.25;
    float sum = 0.0;
    float weightSum = 0.0;
    [unroll]
    for (int y = -2; y < 2; ++y)
    {
        [unroll]
        for (int x = -2; x < 2; ++x)
        {
            // Whole texels from the centre: four in a row cover all four of the pattern's columns.
            const float2 uv = input.uv + float2(x, y) * occlusionTexelSize;
            const float offPlane = abs(dot(ViewPosition(uv) - centre, normal));
            const float weight = saturate(1.0 - offPlane / planeTolerance) + 1e-3;
            sum += occlusionTexture.SampleLevel(occlusionSampler, uv, 0).r * weight;
            weightSum += weight;
        }
    }
    const float visibility = sum / weightSum;
    return float4(visibility, visibility, visibility, visibility);
}
