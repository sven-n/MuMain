// shadow_volume.vert.hlsl — Vertex-only shader for shadow stencil passes.
// Story 4.3.2: Shader Programs [VS1-RENDER-SHADERS]
// No fragment output; used with SDL_GPU_COLORCOMPONENT_NONE color mask.
// MVP matrix passed via cbuffer slot b0 (identity for this story).
cbuffer MVP : register(b0) { float4x4 mvp; };
struct VSInput { float3 pos : TEXCOORD0; };
float4 main(VSInput input) : SV_Position
{
    return mul(mvp, float4(input.pos, 1.0));
}
