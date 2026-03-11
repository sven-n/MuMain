// basic_colored.vert.hlsl — Vertex shader for flat colored geometry (UI lines, debug prims).
// Story 4.3.2: Shader Programs [VS1-RENDER-SHADERS]
// No texture sampling; passes vertex color to fragment stage.
cbuffer ScreenSize : register(b1) { float2 screenSize; float2 pad; };
struct VSInput { float2 pos : TEXCOORD0; float2 uv : TEXCOORD1; float4 color : TEXCOORD2; };
struct VSOutput { float4 pos : SV_Position; float4 color : TEXCOORD0; };
VSOutput main(VSInput input)
{
    VSOutput o;
    o.pos = float4(input.pos / screenSize * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    o.color = input.color;
    return o;
}
