// basic_colored.frag.hlsl — Fragment shader outputting vertex color directly.
// Story 4.3.2: Shader Programs [VS1-RENDER-SHADERS]
// Flat shading: no texture sampling, no fog.
struct FSInput { float4 pos : SV_Position; float4 color : TEXCOORD0; };
float4 main(FSInput input) : SV_Target
{
    return input.color;
}
