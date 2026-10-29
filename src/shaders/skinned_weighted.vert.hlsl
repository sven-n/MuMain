// Four-weight skinning for glTF models: a vertex with weights (weights.x > 0: the largest weight
// comes first) blends up to four palette joints by weight from bind space; one without weights
// follows its position and normal bone, as a BMD vertex.
#define MU_WEIGHTED_SKINNING 1

struct VSInput
{
    float3 pos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float2 uv : TEXCOORD2;
    float4 color : TEXCOORD3;
    int positionBoneIndex : TEXCOORD4;
    int normalBoneIndex : TEXCOORD5;
    uint4 joints : TEXCOORD6;
    float4 weights : TEXCOORD7;
    float4 tangent : TEXCOORD8; // snorm16; w = +-1, 0 without a normal map
};

#include "skinned_vertex.hlsli"
