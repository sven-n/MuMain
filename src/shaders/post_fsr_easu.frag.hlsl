// post_fsr_easu.frag.hlsl — upscaling of the processed 3D scene from the render size to the
// window's (render scale below 100 %): AMD FidelityFX Super Resolution 1, its edge-adaptive spatial
// upsampling (EASU), the 32-bit path. Each output pixel is a Lanczos-like filter of the 12 nearest
// source texels, stretched along the local edge so edges stay sharp, and clamped to the 2x2 nearest
// so it cannot ring.
// Ported from ffx_fsr1.h (FsrEasuF), with exact reciprocals in place of the approximations.
// Copyright (c) 2021 Advanced Micro Devices, Inc. All rights reserved. MIT licence:
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
// associated documentation files (the "Software"), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge, publish, distribute,
// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions: The above copyright notice and this
// permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.
// EasuUniforms mirrors mu::SdlGpuEasuUniform (Render::PostProcessing::EasuConstants).
Texture2D source : register(t0, space2);
SamplerState sourceSampler : register(s0, space2); // gathers: the filter mode does not matter
cbuffer EasuUniforms : register(b0, space3)
{
    float4 con0; // output pixel to source pixel: scale (xy) and offset (zw)
    float4 con1; // source pixel to texture coordinate (xy), offset to gather centre 0 (zw)
    float4 con2; // offsets to gather centres 1 and 2
    float4 con3; // offset to gather centre 3 (xy)
};
struct FSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

// One tap of the filter: its weight from the distance, rotated and stretched along the edge.
void Tap(inout float3 colour, inout float weight, float2 offset, float2 dir, float2 len, float lob, float clp,
         float3 tapColour)
{
    float2 v;
    v.x = offset.x * dir.x + offset.y * dir.y;
    v.y = offset.x * -dir.y + offset.y * dir.x;
    v *= len;
    const float d2 = min(v.x * v.x + v.y * v.y, clp);
    // Lanczos 2 approximated without sin, rcp or sqrt:
    // (25/16 * (2/5 * x^2 - 1)^2 - (25/16 - 1)) * (lob * x^2 - 1)^2
    float base = 2.0 / 5.0 * d2 - 1.0;
    float window = lob * d2 - 1.0;
    base *= base;
    window *= window;
    base = 25.0 / 16.0 * base - (25.0 / 16.0 - 1.0);
    const float w = base * window;
    colour += tapColour * w;
    weight += w;
}

// Adds the edge direction and length at one of the four texels around the output position, by its
// bilinear weight `w`. a, b, c, d, e are the luma of the texel's plus pattern (c in the middle).
void Direction(inout float2 dir, inout float len, float w, float lA, float lB, float lC, float lD, float lE)
{
    const float dc = lD - lC;
    const float cb = lC - lB;
    float lenX = max(abs(dc), abs(cb));
    lenX = lenX > 0.0 ? 1.0 / lenX : 0.0;
    const float dirX = lD - lB;
    dir.x += dirX * w;
    lenX = saturate(abs(dirX) * lenX);
    lenX *= lenX;
    len += lenX * w;

    const float ec = lE - lC;
    const float ca = lC - lA;
    float lenY = max(abs(ec), abs(ca));
    lenY = lenY > 0.0 ? 1.0 / lenY : 0.0;
    const float dirY = lE - lA;
    dir.y += dirY * w;
    lenY = saturate(abs(dirY) * lenY);
    lenY *= lenY;
    len += lenY * w;
}

float4 main(FSInput input) : SV_Target
{
    const float2 ip = floor(input.pos.xy);
    float2 pp = ip * con0.xy + con0.zw;
    const float2 fp = floor(pp);
    pp -= fp;

    // The 12 taps, gathered around four centres:
    //    b c
    //  e f g h
    //  i j k l
    //    n o
    const float2 p0 = fp * con1.xy + con1.zw;
    const float2 p1 = p0 + con2.xy;
    const float2 p2 = p0 + con2.zw;
    const float2 p3 = p0 + con3.xy;
    const float4 bczzR = source.GatherRed(sourceSampler, p0);
    const float4 bczzG = source.GatherGreen(sourceSampler, p0);
    const float4 bczzB = source.GatherBlue(sourceSampler, p0);
    const float4 ijfeR = source.GatherRed(sourceSampler, p1);
    const float4 ijfeG = source.GatherGreen(sourceSampler, p1);
    const float4 ijfeB = source.GatherBlue(sourceSampler, p1);
    const float4 klhgR = source.GatherRed(sourceSampler, p2);
    const float4 klhgG = source.GatherGreen(sourceSampler, p2);
    const float4 klhgB = source.GatherBlue(sourceSampler, p2);
    const float4 zzonR = source.GatherRed(sourceSampler, p3);
    const float4 zzonG = source.GatherGreen(sourceSampler, p3);
    const float4 zzonB = source.GatherBlue(sourceSampler, p3);

    // Luma times 2.
    const float4 bczzL = bczzB * 0.5 + (bczzR * 0.5 + bczzG);
    const float4 ijfeL = ijfeB * 0.5 + (ijfeR * 0.5 + ijfeG);
    const float4 klhgL = klhgB * 0.5 + (klhgR * 0.5 + klhgG);
    const float4 zzonL = zzonB * 0.5 + (zzonR * 0.5 + zzonG);
    const float bL = bczzL.x;
    const float cL = bczzL.y;
    const float iL = ijfeL.x;
    const float jL = ijfeL.y;
    const float fL = ijfeL.z;
    const float eL = ijfeL.w;
    const float kL = klhgL.x;
    const float lL = klhgL.y;
    const float hL = klhgL.z;
    const float gL = klhgL.w;
    const float oL = zzonL.z;
    const float nL = zzonL.w;

    float2 dir = float2(0.0, 0.0);
    float len = 0.0;
    Direction(dir, len, (1.0 - pp.x) * (1.0 - pp.y), bL, eL, fL, gL, jL);
    Direction(dir, len, pp.x * (1.0 - pp.y), cL, fL, gL, hL, kL);
    Direction(dir, len, (1.0 - pp.x) * pp.y, fL, iL, jL, kL, nL);
    Direction(dir, len, pp.x * pp.y, gL, jL, kL, lL, oL);

    // Normalise the direction; none (a flat area) points along x.
    const float2 dir2 = dir * dir;
    float dirR = dir2.x + dir2.y;
    const bool flat = dirR < 1.0 / 32768.0;
    dirR = flat ? 1.0 : rsqrt(dirR);
    dir.x = flat ? 1.0 : dir.x;
    dir *= dirR;
    len = len * 0.5;
    len *= len;
    // Stretch from 1 along an axis to sqrt(2) on a diagonal; longer along an edge, shorter across.
    const float stretch = (dir.x * dir.x + dir.y * dir.y) / max(abs(dir.x), abs(dir.y));
    const float2 len2 = float2(1.0 + (stretch - 1.0) * len, 1.0 - 0.5 * len);
    const float lob = 0.5 + ((1.0 / 4.0 - 0.04) - 0.5) * len;
    const float clp = 1.0 / lob;

    // The 2x2 nearest bound the result, so the negative lobe cannot ring.
    const float3 f = float3(ijfeR.z, ijfeG.z, ijfeB.z);
    const float3 g = float3(klhgR.w, klhgG.w, klhgB.w);
    const float3 j = float3(ijfeR.y, ijfeG.y, ijfeB.y);
    const float3 k = float3(klhgR.x, klhgG.x, klhgB.x);
    const float3 min4 = min(min(f, g), min(j, k));
    const float3 max4 = max(max(f, g), max(j, k));

    float3 colour = float3(0.0, 0.0, 0.0);
    float weight = 0.0;
    Tap(colour, weight, float2(0.0, -1.0) - pp, dir, len2, lob, clp, float3(bczzR.x, bczzG.x, bczzB.x)); // b
    Tap(colour, weight, float2(1.0, -1.0) - pp, dir, len2, lob, clp, float3(bczzR.y, bczzG.y, bczzB.y)); // c
    Tap(colour, weight, float2(-1.0, 1.0) - pp, dir, len2, lob, clp, float3(ijfeR.x, ijfeG.x, ijfeB.x)); // i
    Tap(colour, weight, float2(0.0, 1.0) - pp, dir, len2, lob, clp, j);                                  // j
    Tap(colour, weight, float2(0.0, 0.0) - pp, dir, len2, lob, clp, f);                                  // f
    Tap(colour, weight, float2(-1.0, 0.0) - pp, dir, len2, lob, clp, float3(ijfeR.w, ijfeG.w, ijfeB.w)); // e
    Tap(colour, weight, float2(1.0, 1.0) - pp, dir, len2, lob, clp, k);                                  // k
    Tap(colour, weight, float2(2.0, 1.0) - pp, dir, len2, lob, clp, float3(klhgR.y, klhgG.y, klhgB.y)); // l
    Tap(colour, weight, float2(2.0, 0.0) - pp, dir, len2, lob, clp, float3(klhgR.z, klhgG.z, klhgB.z)); // h
    Tap(colour, weight, float2(1.0, 0.0) - pp, dir, len2, lob, clp, g);                                  // g
    Tap(colour, weight, float2(1.0, 2.0) - pp, dir, len2, lob, clp, float3(zzonR.z, zzonG.z, zzonB.z)); // o
    Tap(colour, weight, float2(0.0, 2.0) - pp, dir, len2, lob, clp, float3(zzonR.w, zzonG.w, zzonB.w)); // n

    return float4(min(max4, max(min4, colour / weight)), 1.0);
}
