// GPU bone skinning for BMD rest-pose geometry: each vertex follows its position and normal bone.
struct VSInput
{
    float3 pos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float2 uv : TEXCOORD2;
    float4 color : TEXCOORD3;
    int positionBoneIndex : TEXCOORD4;
    int normalBoneIndex : TEXCOORD5;
};

#include "skinned_vertex.hlsli"
