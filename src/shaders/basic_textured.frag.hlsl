// basic_textured.frag.hlsl — Fragment shader with texture, alpha discard, alpha-to-coverage and linear fog.
// Story 4.3.2: Shader Programs [VS1-RENDER-SHADERS]
// FogUniforms cbuffer mirrors FogUniform struct (std140): see MuRendererSDLGpu.cpp.
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
struct FSInput { float4 pos : SV_Position; float2 uv : TEXCOORD0; float4 color : TEXCOORD1; float fogFactor : TEXCOORD2; };
float4 main(FSInput input) : SV_Target
{
    float4 color = tex.Sample(s, input.uv) * input.color;
    if (alphaToCoverage)
    {
        // Anti-aliased cutout. Alpha becomes multisample coverage, so a texel's
        // own translucency still shows as it did blended. Only the alpha test's
        // hard step at the threshold is replaced by a one-pixel ramp.
        const float edge = saturate((color.a - alphaThreshold) / max(fwidth(color.a), 0.0001) + 0.5);
        color.a *= edge;
        if (color.a <= 0.0) discard;
    }
    else if (alphaDiscardEnabled && color.a <= alphaThreshold) discard;
    // Story 7.9.7: fogFactor=1.0 → no fog (close), fogFactor=0.0 → full fog (far).
    if (fogEnabled) color.rgb = lerp(fogColor.rgb, color.rgb, input.fogFactor);
    return color;
}
