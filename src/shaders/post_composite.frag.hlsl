// post_composite.frag.hlsl — the processed 3D scene: the scene image with its bloom screen-blended
// on top, colour graded through the map's lookup table. Draws into the frame's destination before
// the UI.
// CompositeUniforms mirrors mu::SdlGpuCompositeUniform.
Texture2D scene : register(t0, space2);
SamplerState sceneSampler : register(s0, space2);   // point: one scene texel per pixel
Texture2D bloom : register(t1, space2);
SamplerState bloomSampler : register(s1, space2);   // linear: the bloom is half resolution
Texture2D grading : register(t2, space2);
SamplerState gradingSampler : register(s2, space2); // linear: interpolates between table entries
cbuffer CompositeUniforms : register(b0, space3)
{
    float2 inverseTargetSize;
    float bloomScale;     // 0: no bloom
    float gradingStrength; // 0: ungraded; else blend of the graded colour with the ungraded one
};
struct FSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

// The lookup table is a 32x32x32 colour cube stored as 32 slices of 32x32 side by side
// (1024x32): red runs along x inside a slice, green along y, blue picks the slice.
static const float kGradingSize = 32.0;

// Defined after main so the compiled shader declares its textures in binding order (scene,
// bloom, grading): spirv-cross numbers Metal textures in that order.
float3 Grade(float3 color);

float4 main(FSInput input) : SV_Target
{
    const float2 uv = input.pos.xy * inverseTargetSize;
    float4 color = scene.Sample(sceneSampler, uv);
    if (bloomScale > 0.0)
    {
        // Screen blend: a dark pixel gains about the glow's light, a bright one little, so glows
        // light their surroundings without washing bright surfaces out to white.
        const float3 glow = saturate(bloom.Sample(bloomSampler, uv).rgb * bloomScale);
        color.rgb = 1.0 - (1.0 - color.rgb) * (1.0 - glow);
    }
    if (gradingStrength > 0.0)
    {
        // Render::PostProcessing::BlendGrading mirrors this blend.
        color.rgb = lerp(color.rgb, Grade(color.rgb), gradingStrength);
    }
    return color;
}

float3 Grade(float3 color)
{
    const float lastEntry = kGradingSize - 1.0;
    const float blue = color.b * lastEntry;
    const float lowerSlice = floor(blue);
    const float upperSlice = min(lowerSlice + 1.0, lastEntry);
    const float sliceBlend = blue - lowerSlice;

    // Texel centres of the red/green entry inside one slice.
    const float x = (color.r * lastEntry + 0.5) / (kGradingSize * kGradingSize);
    const float y = (color.g * lastEntry + 0.5) / kGradingSize;
    const float sliceWidth = 1.0 / kGradingSize;
    const float3 lower = grading.Sample(gradingSampler, float2(x + lowerSlice * sliceWidth, y)).rgb;
    const float3 upper = grading.Sample(gradingSampler, float2(x + upperSlice * sliceWidth, y)).rgb;
    return lerp(lower, upper, sliceBlend);
}
