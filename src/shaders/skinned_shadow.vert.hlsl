// BMD body shadow: one bone per vertex.

// The skinned vertex layout; glslang would otherwise number these 0, 1, 2 (dxc ignores vk::location).
struct VSInput
{
    [[vk::location(0)]] float3 pos : TEXCOORD0;
    [[vk::location(3)]] float4 color : TEXCOORD3;
    [[vk::location(4)]] int positionBoneIndex : TEXCOORD4;
};

#include "skinned_shadow.hlsli"
