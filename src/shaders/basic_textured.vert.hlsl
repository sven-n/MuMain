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

    // Clip vertices behind or very near the camera.
    // When w is near zero, perspective division produces extreme x/y values that
    // stretch triangles across the entire screen (visible as light streaks).
    // OpenGL's fixed-function pipeline clipped these; SDL_GPU does not.
    // Push the vertex behind the near clip plane so the GPU discards it.
    if (o.pos.w < 0.1)
    {
        o.pos = float4(0.0, 0.0, -1.0, 1.0);
    }

    // Story 7.9.7 (AC-7): Linear fog using eye-space distance.
    float dist = abs(o.pos.w);
    float range = fogEnd - fogStart;
    o.fogFactor = (range > 0.001) ? saturate((fogEnd - dist) / range) : 1.0;
    return o;
}
