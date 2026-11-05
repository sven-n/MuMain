// glTF body shadow: up to four weighted joints per vertex (weights.x > 0), else one bone as a BMD vertex.
#define MU_WEIGHTED_SKINNING 1

#include "vertex_location.hlsli"

// The weighted skinned vertex layout.
struct VSInput
{
    MU_VERTEX_LOCATION(0) float3 pos : TEXCOORD0;
    MU_VERTEX_LOCATION(3) float4 color : TEXCOORD3;
    MU_VERTEX_LOCATION(4) int positionBoneIndex : TEXCOORD4;
    MU_VERTEX_LOCATION(6) uint4 joints : TEXCOORD6;
    MU_VERTEX_LOCATION(7) float4 weights : TEXCOORD7;
};

#include "skinned_shadow.hlsli"
