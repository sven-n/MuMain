// BMD body shadow: one bone per vertex.

#include "vertex_location.hlsli"

// The skinned vertex layout.
struct VSInput
{
    MU_VERTEX_LOCATION(0) float3 pos : TEXCOORD0;
    MU_VERTEX_LOCATION(3) float4 color : TEXCOORD3;
    MU_VERTEX_LOCATION(4) int positionBoneIndex : TEXCOORD4;
};

#include "skinned_shadow.hlsli"
