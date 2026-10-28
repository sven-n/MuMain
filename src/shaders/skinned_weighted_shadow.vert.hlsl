// glTF body shadow: up to four weighted joints per vertex (weights.x > 0), else one bone as a BMD vertex.
#define MU_WEIGHTED_SKINNING 1

// The weighted skinned vertex layout; glslang would otherwise number these 0, 1, 2 (dxc ignores vk::location).
struct VSInput
{
    [[vk::location(0)]] float3 pos : TEXCOORD0;
    [[vk::location(3)]] float4 color : TEXCOORD3;
    [[vk::location(4)]] int positionBoneIndex : TEXCOORD4;
    [[vk::location(6)]] uint4 joints : TEXCOORD6;
    [[vk::location(7)]] float4 weights : TEXCOORD7;
};

#include "skinned_shadow.hlsli"
