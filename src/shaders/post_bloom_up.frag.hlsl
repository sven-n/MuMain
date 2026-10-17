// post_bloom_up.frag.hlsl — bloom upsample (dual filter): eight taps on a ring around the pixel,
// the diagonal ones weighted double. The pipeline adds the result, scaled by levelWeight, to the
// level it draws into: every level contributes to the final glow, the wider ones less, so the glow
// stays close to its effect instead of veiling the frame.
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
static const float kDiagonalWeight = 2.0;
static const float kTotalWeight = 12.0;
float4 main(FSInput input) : SV_Target
{
    const float2 halfTexel = sourceTexelSize * 0.5;
    const float2 uv = input.uv;
    float3 sum = source.Sample(linearSampler, uv + float2(-halfTexel.x * 2.0, 0.0)).rgb;
    sum += source.Sample(linearSampler, uv + float2(halfTexel.x * 2.0, 0.0)).rgb;
    sum += source.Sample(linearSampler, uv + float2(0.0, -halfTexel.y * 2.0)).rgb;
    sum += source.Sample(linearSampler, uv + float2(0.0, halfTexel.y * 2.0)).rgb;
    sum += source.Sample(linearSampler, uv + float2(-halfTexel.x, -halfTexel.y)).rgb * kDiagonalWeight;
    sum += source.Sample(linearSampler, uv + float2(halfTexel.x, -halfTexel.y)).rgb * kDiagonalWeight;
    sum += source.Sample(linearSampler, uv + float2(-halfTexel.x, halfTexel.y)).rgb * kDiagonalWeight;
    sum += source.Sample(linearSampler, uv + float2(halfTexel.x, halfTexel.y)).rgb * kDiagonalWeight;
    return float4(sum / kTotalWeight * levelWeight, 1.0);
}
