// post_ao.frag.hlsl — screen-space ambient occlusion of the 3D scene, at half resolution: how much
// of the hemisphere above each surface nearby geometry hides, from the scene's depth alone.
// post_ao_blur.frag.hlsl then removes its 4x4 rotation pattern.
// AmbientOcclusion mirrors mu::SdlGpuAmbientOcclusionUniform.
Texture2D depthTexture : register(t0, space2);
SamplerState depthSampler : register(s0, space2); // point
cbuffer AmbientOcclusion : register(b0, space3)
{
    float4x4 projection;
    float4x4 inverseProjection;
    float2 depthTexelSize; // 1 / the depth's size
    float radius;          // view-space units
    float padding;
};
struct FSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

static const uint SampleCount = 12;
// Sine of the angle an occluder must rise above the surface's plane before it counts: depth
// precision and slightly bumpy ground stay below it.
static const float PlaneRise = 0.15;
// Points in the unit hemisphere around +z, more of them near the centre.
static const float3 Kernel[SampleCount] = {
    float3(0.21, 0.05, 0.10),  float3(-0.10, 0.25, 0.15), float3(-0.28, -0.12, 0.08), float3(0.14, -0.30, 0.20),
    float3(0.42, 0.22, 0.25),  float3(-0.35, 0.40, 0.12), float3(-0.30, -0.45, 0.35), float3(0.38, -0.36, 0.42),
    float3(0.70, 0.10, 0.30),  float3(-0.15, 0.72, 0.45), float3(-0.66, -0.20, 0.55), float3(0.20, -0.62, 0.70),
};
// 4x4 ordered rotations: the blur's 4x4 average cancels the pattern.
static const float Rotations[16] = {0.0 / 16.0, 8.0 / 16.0, 2.0 / 16.0, 10.0 / 16.0, 12.0 / 16.0, 4.0 / 16.0,
                                    14.0 / 16.0, 6.0 / 16.0, 3.0 / 16.0, 11.0 / 16.0, 1.0 / 16.0, 9.0 / 16.0,
                                    15.0 / 16.0, 7.0 / 16.0, 13.0 / 16.0, 5.0 / 16.0};

float SceneDepth(float2 uv)
{
    return depthTexture.SampleLevel(depthSampler, uv, 0).r;
}

float3 ViewPosition(float2 uv, float depth)
{
    const float4 view = mul(inverseProjection, float4(uv.x * 2.0 - 1.0, 1.0 - uv.y * 2.0, depth, 1.0));
    return view.xyz / view.w;
}

// At a depth texel's centre: half-resolution pixels sit on texel corners, where point sampling rounds
// either way from row to row (horizontal lines); a quarter texel past the corner picks one.
float3 ViewPositionAt(float2 uv)
{
    const float2 centre = (floor(uv / depthTexelSize + 0.25) + 0.5) * depthTexelSize;
    return ViewPosition(centre, SceneDepth(centre));
}

// The surface's direction from its nearer neighbour on each axis, so silhouettes do not bend it.
float3 SurfaceNormal(float2 uv, float3 position)
{
    const float3 right = ViewPositionAt(uv + float2(depthTexelSize.x, 0.0)) - position;
    const float3 left = position - ViewPositionAt(uv - float2(depthTexelSize.x, 0.0));
    const float3 down = ViewPositionAt(uv + float2(0.0, depthTexelSize.y)) - position;
    const float3 up = position - ViewPositionAt(uv - float2(0.0, depthTexelSize.y));
    const float3 alongX = abs(right.z) < abs(left.z) ? right : left;
    const float3 alongY = abs(down.z) < abs(up.z) ? down : up;
    float3 normal = normalize(cross(alongX, alongY));
    return dot(normal, position) > 0.0 ? -normal : normal; // towards the camera
}

float4 main(FSInput input) : SV_Target
{
    if (SceneDepth(input.uv) >= 1.0)
        return float4(1.0, 1.0, 1.0, 1.0); // sky
    const float3 position = ViewPositionAt(input.uv);
    const float3 normal = SurfaceNormal(input.uv, position);

    const uint2 cell = uint2(input.pos.xy) & 3u;
    const float angle = 6.2831853 * Rotations[cell.y * 4u + cell.x];
    const float3 turn = float3(cos(angle), sin(angle), 0.0);
    const float3 tangent = normalize(turn - normal * dot(turn, normal) + float3(1e-4, 0.0, 0.0));
    const float3 bitangent = cross(normal, tangent);

    float occlusion = 0.0;
    [unroll]
    for (uint i = 0; i < SampleCount; ++i)
    {
        const float3 offset = tangent * Kernel[i].x + bitangent * Kernel[i].y + normal * Kernel[i].z;
        const float3 samplePosition = position + offset * radius;
        const float4 clip = mul(projection, float4(samplePosition, 1.0));
        const float2 sampleUv = float2(clip.x / clip.w * 0.5 + 0.5, 0.5 - clip.y / clip.w * 0.5);
        // The surface seen where the sample lands occludes by how far it rises above this surface's
        // plane: the same flat ground, wherever its pixel rows put it, does not (no banding).
        const float3 toOccluder = ViewPositionAt(sampleUv) - position;
        const float distanceSquared = dot(toOccluder, toOccluder);
        const float rise = dot(toOccluder, normal) * rsqrt(max(distanceSquared, 1e-6));
        // Only geometry within the radius counts: a far wall behind the sample does not.
        const float inRange = saturate(1.0 - distanceSquared / (radius * radius));
        occlusion += saturate(rise - PlaneRise) * inRange;
    }
    const float visibility = 1.0 - occlusion / SampleCount;
    return float4(visibility, visibility, visibility, visibility);
}
