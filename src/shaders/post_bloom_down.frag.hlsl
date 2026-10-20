// post_bloom_down.frag.hlsl — bloom downsample (dual filter): a centre tap and four diagonal taps
// half a source texel away, read through a linear sampler so each tap averages four texels.
// BloomUniforms mirrors mu::SdlGpuBloomUniform.
Texture2D source : register(t0, space2);
SamplerState linearSampler : register(s0, space2);
cbuffer BloomUniforms : register(b0, space3)
{
    float2 sourceTexelSize;
    float levelWeight; // upsample: how much of the smaller level is added
    float padding;
};
struct FSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};
static const float kCentreWeight = 4.0;
static const float kTotalWeight = 8.0;
float4 main(FSInput input) : SV_Target
{
    const float2 halfTexel = sourceTexelSize * 0.5;
    float3 sum = source.Sample(linearSampler, input.uv).rgb * kCentreWeight;
    sum += source.Sample(linearSampler, input.uv - halfTexel).rgb;
    sum += source.Sample(linearSampler, input.uv + halfTexel).rgb;
    sum += source.Sample(linearSampler, input.uv + float2(halfTexel.x, -halfTexel.y)).rgb;
    sum += source.Sample(linearSampler, input.uv - float2(halfTexel.x, -halfTexel.y)).rgb;
    return float4(sum / kTotalWeight, 1.0);
}
