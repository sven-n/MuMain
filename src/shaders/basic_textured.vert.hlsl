// basic_textured.vert.hlsl — Vertex shader for 2D textured geometry (Vertex2D path).
// Story 4.3.2: Shader Programs [VS1-RENDER-SHADERS]
// Inputs match Vertex2D layout: pos(float2), uv(float2), color(ubyte4_norm).
// Screen size passed via cbuffer slot b1; fog factor always 0 for 2D path.
// Vertices arrive in window-pixel space with Y pre-flipped (y = WindowHeight - y).
// The Y-flip + this mapping produces the correct orientation on SDL_GPU (Metal/Vulkan).
cbuffer ScreenSize : register(b1) { float2 screenSize; float2 pad; };
struct VSInput { float2 pos : TEXCOORD0; float2 uv : TEXCOORD1; float4 color : TEXCOORD2; };
struct VSOutput { float4 pos : SV_Position; float2 uv : TEXCOORD0; float4 color : TEXCOORD1; float fogFactor : TEXCOORD2; };
VSOutput main(VSInput input)
{
    VSOutput o;
    o.pos = float4(input.pos / screenSize * 2.0 - 1.0, 0.0, 1.0);
    o.uv = input.uv;
    o.color = input.color;
    o.fogFactor = 0.0;
    return o;
}
