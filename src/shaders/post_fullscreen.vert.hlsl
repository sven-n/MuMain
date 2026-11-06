// post_fullscreen.vert.hlsl — one triangle covering the whole target, for post-processing passes.
// Drawn with three vertices and no vertex buffer. uv is (0,0) at the top-left corner of the
// target and (1,1) at the bottom-right one.
struct VSInput { uint vertexId : SV_VertexID; };
struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};
VSOutput main(VSInput input)
{
    // Vertices 0, 1, 2 get uv (0,0), (2,0), (0,2): the triangle's corners beyond the target
    // are clipped away.
    const float2 uv = float2((input.vertexId << 1) & 2, input.vertexId & 2);
    VSOutput o;
    o.pos = float4(uv.x * 2.0 - 1.0, 1.0 - uv.y * 2.0, 0.0, 1.0);
    o.uv = uv;
    return o;
}
