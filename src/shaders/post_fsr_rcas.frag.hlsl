// post_fsr_rcas.frag.hlsl — sharpening of the processed 3D scene, after its upscaling or alone:
// AMD FidelityFX Super Resolution 1, its robust contrast adaptive sharpening (RCAS), the 32-bit
// path. A five-tap cross whose negative lobe is as strong as it can be without the result leaving
// the range of its neighbours, times the sharpness.
// Ported from ffx_fsr1.h (FsrRcasF), with exact reciprocals in place of the approximations.
// Copyright (c) 2021 Advanced Micro Devices, Inc. All rights reserved. MIT licence:
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
// associated documentation files (the "Software"), to deal in the Software without restriction,
// including without limitation the rights to use, copy, modify, merge, publish, distribute,
// sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions: The above copyright notice and this
// permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED.
// RcasUniforms mirrors mu::SdlGpuRcasUniform.
Texture2D source : register(t0, space2);
SamplerState sourceSampler : register(s0, space2); // the centre tap; the others are loads
cbuffer RcasUniforms : register(b0, space3)
{
    float sharpness; // linear: 1 is the most, each halving one stop less
    float maxX;      // the last texel of the source, for clamping the taps
    float maxY;
    float padding;
};
struct FSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

// The strongest lobe that does not give unnatural results.
static const float kLobeLimit = 0.25 - 1.0 / 16.0;

float3 Load(int2 position)
{
    const int2 clamped = clamp(position, int2(0, 0), int2(maxX, maxY));
    return source.Load(int3(clamped, 0)).rgb;
}

float Max3(float a, float b, float c)
{
    return max(a, max(b, c));
}

float3 Max3(float3 a, float3 b, float3 c)
{
    return max(a, max(b, c));
}

float3 Min3(float3 a, float3 b, float3 c)
{
    return min(a, min(b, c));
}

float4 main(FSInput input) : SV_Target
{
    //    b
    //  d e f
    //    h
    const int2 sp = int2(input.pos.xy);
    const float3 b = Load(sp + int2(0, -1));
    const float3 d = Load(sp + int2(-1, 0));
    // At its own texel's centre, so the sample is that texel exactly.
    const float3 e = source.SampleLevel(sourceSampler, input.uv, 0.0).rgb;
    const float3 f = Load(sp + int2(1, 0));
    const float3 h = Load(sp + int2(0, 1));

    const float3 min4 = min(Min3(b, d, f), h);
    const float3 max4 = max(Max3(b, d, f), h);
    // How far the lobe may go before the result would clip at 0 or at 1, per channel.
    const float3 hitMin = min(min4, e) / (4.0 * max4);
    const float3 hitMax = (1.0 - max(max4, e)) / (4.0 * min4 - 4.0);
    const float3 lobeRgb = max(-hitMin, hitMax);
    const float lobe = max(-kLobeLimit, min(Max3(lobeRgb.r, lobeRgb.g, lobeRgb.b), 0.0)) * sharpness;

    return float4((lobe * (b + d + f + h) + e) / (4.0 * lobe + 1.0), 1.0);
}
