// post_composite.frag.hlsl — the processed 3D scene: the scene image darkened by its ambient
// occlusion (an HDR scene then tone mapped),
// with its bloom screen-blended on top, colour graded through the map's lookup table and dithered.
// Draws into the frame's destination before the UI.
// CompositeUniforms mirrors mu::SdlGpuCompositeUniform.
Texture2D scene : register(t0, space2);
SamplerState sceneSampler : register(s0, space2);   // point: one scene texel per pixel
Texture2D bloom : register(t1, space2);
SamplerState bloomSampler : register(s1, space2);   // linear: the bloom is half resolution
Texture2D grading : register(t2, space2);
SamplerState gradingSampler : register(s2, space2); // linear: interpolates between table entries
Texture2D ambientOcclusion : register(t3, space2);
SamplerState ambientOcclusionSampler : register(s3, space2); // linear: half resolution
Texture2D bloomSource : register(t4, space2);
SamplerState bloomSourceSampler : register(s4, space2); // point
cbuffer CompositeUniforms : register(b0, space3)
{
    float2 inverseTargetSize;
    float bloomScale;     // 0: no bloom
    float gradingStrength; // 0: ungraded; else blend of the graded colour with the ungraded one
    float toneMapKnee;     // channels above it roll off towards 1; 1: none
    float ditherAmount;    // 0: no dither
    float ambientOcclusionStrength; // 0: no ambient occlusion
    float ambientOcclusionEffects;  // 1: the bloom source holds this frame's additive light
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
float AmbientOcclusion(float2 uv);

// The HDR scene's tone map: unchanged up to the knee, then an exponential shoulder towards 1, so
// stacked light rolls off instead of clipping. The same curve as skinned_lit.frag's highlight knee.
float ToneMapChannel(float value)
{
    const float range = 1.0 - toneMapKnee;
    return value > toneMapKnee ? toneMapKnee + range * (1.0 - exp(-(value - toneMapKnee) / range)) : value;
}

// Interleaved gradient noise in [0, 1): an ordered dither pattern without a texture.
float DitherNoise(float2 pixel)
{
    return frac(52.9829189 * frac(dot(pixel, float2(0.06711056, 0.00583715))));
}

float4 main(FSInput input) : SV_Target
{
    const float2 uv = input.pos.xy * inverseTargetSize;
    float4 color = scene.Sample(sceneSampler, uv);
    if (ambientOcclusionStrength > 0.0)
        color.rgb *= AmbientOcclusion(uv);
    if (toneMapKnee < 1.0)
    {
        // Blending over light above 1 (inverse colour) can leave negative values: black.
        color.rgb = max(color.rgb, 0.0);
        color.rgb = float3(ToneMapChannel(color.r), ToneMapChannel(color.g), ToneMapChannel(color.b));
    }
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
    if (ditherAmount > 0.0)
        color.rgb += (DitherNoise(input.pos.xy) - 0.5) * ditherAmount;
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

// The share of the scene's light its ambient occlusion leaves. Additive light (effects, glows) is
// light, not surface: where the bloom source holds it, the occlusion fades out.
float AmbientOcclusion(float2 uv)
{
    float visibility = ambientOcclusion.Sample(ambientOcclusionSampler, uv).r;
    if (ambientOcclusionEffects > 0.0)
    {
        const float effectLight = dot(bloomSource.Sample(bloomSourceSampler, uv).rgb, float3(0.2126, 0.7152, 0.0722));
        visibility = lerp(visibility, 1.0, saturate(effectLight * 4.0));
    }
    return max(1.0 - (1.0 - visibility) * ambientOcclusionStrength, 0.0);
}
