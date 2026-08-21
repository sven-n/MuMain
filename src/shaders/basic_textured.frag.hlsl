// basic_textured.frag.hlsl — Fragment shader with texture, alpha discard, and linear fog.
// Story 4.3.2: Shader Programs [VS1-RENDER-SHADERS]
// FogUniforms cbuffer mirrors FogUniform struct (std140): see MuRendererSDLGpu.cpp.
Texture2D tex : register(t0);
SamplerState s : register(s0);
cbuffer FogUniforms : register(b0)
{
    uint fogEnabled;
    uint alphaDiscardEnabled;
    float alphaThreshold;
    float pad0;
    float fogStart;
    float fogEnd;
    float4 fogColor;
};
struct FSInput { float4 pos : SV_Position; float2 uv : TEXCOORD0; float4 color : TEXCOORD1; float fogFactor : TEXCOORD2; };
float4 main(FSInput input) : SV_Target
{
    float4 color = tex.Sample(s, input.uv) * input.color;
    if (alphaDiscardEnabled && color.a <= alphaThreshold) discard;
    // Story 7.9.7: fogFactor=1.0 → no fog (close), fogFactor=0.0 → full fog (far).
    if (fogEnabled) color.rgb = lerp(fogColor.rgb, color.rgb, input.fogFactor);
    return color;
}
