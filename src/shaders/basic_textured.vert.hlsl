// basic_textured.vert.hlsl — Vertex shader for 2D and 3D textured geometry.
// Story 4.3.2: Shader Programs [VS1-RENDER-SHADERS]
// Story 7.9.7: fogFactor computed from eye-space distance (clip-space w).
// Inputs: pos(float3), uv(float2), color(ubyte4_norm).
//   2D pipeline sends float2 pos — GPU fills z=0 automatically.
//   3D pipeline sends float3 pos — full world-space position.
// MVP matrix + fog params pushed per-draw: ortho for 2D, perspective*view for 3D.
cbuffer Transform : register(b1) { float4x4 mvp; float fogStart; float fogEnd; float2 fogPad; };
struct VSInput { float3 pos : TEXCOORD0; float2 uv : TEXCOORD1; float4 color : TEXCOORD2; };
struct VSOutput { float4 pos : SV_Position; float2 uv : TEXCOORD0; float4 color : TEXCOORD1; float fogFactor : TEXCOORD2; };
VSOutput main(VSInput input)
{
    VSOutput o;
    o.pos = mul(mvp, float4(input.pos, 1.0));
    o.uv = input.uv;
    o.color = input.color;
    // Story 7.9.7 (AC-7): Linear fog using eye-space distance.
    // For perspective projection, clip-space w equals negated eye-space z.
    // For ortho, fogEnd-fogStart=0 → fogFactor=1.0 (no fog).
    float dist = abs(o.pos.w);
    float range = fogEnd - fogStart;
    o.fogFactor = (range > 0.001) ? saturate((fogEnd - dist) / range) : 1.0;
    return o;
}
