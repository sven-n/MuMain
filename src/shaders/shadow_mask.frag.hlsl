// shadow_mask.frag.hlsl — a ground shadow into the ground shadow mask (R8, MIN blend): 1 minus the
// shadow's alpha, so the darkest shadow at a pixel wins and overlapping shadows count once.
// world_lit.frag.hlsl reads the mask. Pairs with skinned_shadow.vert and basic_textured.vert.
struct FSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : TEXCOORD1;
};

float4 main(FSInput input) : SV_Target
{
    return (1.0 - input.color.a).xxxx;
}
