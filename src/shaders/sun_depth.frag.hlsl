// sun_depth.frag.hlsl — a draw into the sun shadow map (depth only). Pairs with
// basic_textured.vert and skinned_textured.vert, whose matrix the sun pass replaces with the sun's.
// FogUniforms mirrors FogUniform, whose fogEnabled slot carries the coverage mode (SunCoverage - 1):
// solid draws let the sun through where the scene's alpha test drops their texels (leaves, fences,
// hair); a character's blended parts (wings, capes, auras) cover a texel with the chance of their
// alpha, or glowing, their brightness, which the shadow's blur turns into a partial shadow.
Texture2D tex : register(t0, space2);
SamplerState s : register(s0, space2);
cbuffer FogUniforms : register(b0, space3)
{
    uint coverageMode; // 0 solid, 1 by alpha, 2 by brightness
    uint alphaDiscardEnabled;
    float alphaThreshold;
    uint alphaToCoverage;
    float fogStart;
    float fogEnd;
    float4 fogColor;
};
struct FSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : TEXCOORD1;
};

// Interleaved gradient noise: a threshold per texel, even over small areas.
float Dither(float2 position)
{
    return frac(52.9829189 * frac(dot(position, float2(0.06711056, 0.00583715))));
}

void main(FSInput input)
{
    const float4 texel = tex.Sample(s, input.uv) * input.color;
    if (coverageMode == 0)
    {
        if ((alphaDiscardEnabled || alphaToCoverage) && texel.a <= alphaThreshold)
            discard;
        return;
    }
    const float coverage = coverageMode == 1 ? texel.a : max(texel.r, max(texel.g, texel.b)) * texel.a;
    if (coverage <= Dither(input.pos.xy))
        discard;
}
