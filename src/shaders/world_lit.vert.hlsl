// world_lit.vert.hlsl — basic_textured.vert for the ground lit per pixel (world_lit.frag.hlsl).
// The ground is submitted in world space, so its vertex position is also the fragment's world
// position for the point lights.
cbuffer Transform : register(b0, space1) { float4x4 mvp; float fogStart; float fogEnd; float2 fogPad; };
// The normal carries, as an offset from straight up, the darkening the vertex colour cannot hold.
struct VSInput { float3 pos : TEXCOORD0; float2 uv : TEXCOORD1; float4 color : TEXCOORD2; float3 normal : TEXCOORD3; };
struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : TEXCOORD1;
    float fogFactor : TEXCOORD2;
    float eyeDepth : TEXCOORD3;
    float3 worldPosition : TEXCOORD4;
    float3 darkening : TEXCOORD5;
    float clipDist : SV_ClipDistance0;
};
VSOutput main(VSInput input)
{
    VSOutput o;
    o.pos = mul(mvp, float4(input.pos, 1.0));
    o.uv = input.uv;
    o.color = input.color;
    // Near-plane user clip, as in basic_textured.vert.
    o.clipDist = o.pos.w - 1.0;
    const float dist = abs(o.pos.w);
    const float range = fogEnd - fogStart;
    o.fogFactor = (range > 0.001) ? saturate((fogEnd - dist) / range) : 1.0;
    o.eyeDepth = dist;
    o.worldPosition = input.pos;
    o.darkening = input.normal - float3(0.0, 0.0, 1.0);
    return o;
}
