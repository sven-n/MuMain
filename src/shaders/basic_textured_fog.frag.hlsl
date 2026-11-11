// basic_textured_fog.frag.hlsl — basic_textured.frag with Enhanced per-pixel exponential fog.
// Used by the 3D and GPU-skinned unlit pipelines while Enhanced lighting is on, so terrain, static
// objects and model overlays fade into the fog together with skinned_lit.frag models.
// The shared uniforms and lighting functions: scene_lighting.hlsli.
Texture2D tex : register(t0, space2);
SamplerState s : register(s0, space2);
#include "scene_lighting.hlsli"
struct FSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : TEXCOORD1;
    float fogFactor : TEXCOORD2;
    float eyeDepth : TEXCOORD3;
};

float4 main(FSInput input) : SV_Target
{
    float4 color = tex.Sample(s, input.uv) * input.color;
    if (alphaToCoverage)
    {
        // Anti-aliased cutout, as in basic_textured.frag: coverage keeps the
        // texel's translucency; only the alpha test's step becomes a ramp.
        const float edge = saturate((color.a - alphaThreshold) / max(fwidth(color.a), 0.0001) + 0.5);
        color.a *= edge;
        if (color.a <= 0.0) discard;
    }
    else if (alphaDiscardEnabled && color.a <= alphaThreshold) discard;
    if (fogEnabled && sceneFogColor.w > 0.0)
        color.rgb = lerp(color.rgb, sceneFogColor.rgb, EnhancedFogAmount(input.eyeDepth));
    return color;
}
