// post_fxaa.frag.hlsl — fast approximate anti-aliasing of the processed 3D scene, before the UI is
// drawn on it. It finds edges by the contrast in luma, follows each edge along its length to find
// where it ends, and blends across it by how far the pixel is from that end; thin features and
// single bright pixels are softened by their contrast with their neighbours. It smooths what
// multisampling cannot: highlights and edges inside a surface's own shading and texture.
// After Timothy Lottes' FXAA 3.11 (the "quality" variant), written for this client.
// FxaaUniforms mirrors mu::SdlGpuFxaaUniform.
Texture2D image : register(t0, space2);
SamplerState linearSampler : register(s0, space2);
cbuffer FxaaUniforms : register(b0, space3)
{
    float2 texelSize;
    float subpixelStrength; // 0: edges only; 1: full softening of single bright pixels
    float padding;
};
struct FSInput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
};

// Edges are smoothed only where the local contrast is at least this share of the brightest
// neighbour, and above an absolute floor so dark areas keep their texture.
static const float kEdgeThreshold = 0.125;
static const float kEdgeThresholdMin = 0.0312;
// The steps of the search along the edge, in texels, and how many there are.
static const int kSearchSteps = 10;
static const float kSearchStep[kSearchSteps] = {1.0, 1.0, 1.0, 1.0, 1.5, 2.0, 2.0, 2.0, 4.0, 8.0};
// The share of an edge's local gradient at which the search counts the edge as ended.
static const float kGradientScale = 0.25;

float Luma(float3 color)
{
    return dot(color, float3(0.299, 0.587, 0.114));
}

float LumaAt(float2 uv)
{
    return Luma(image.SampleLevel(linearSampler, uv, 0.0).rgb);
}

float LumaOffset(float2 uv, float2 offset)
{
    return LumaAt(uv + offset * texelSize);
}

// How much of a single bright or dark pixel is blended with its neighbours, from its contrast
// with their average.
float SubpixelBlend(float lumaCentre, float lumaRange, float neighbourAverage)
{
    const float contrast = saturate(abs(neighbourAverage - lumaCentre) / lumaRange);
    const float smoothed = smoothstep(0.0, 1.0, contrast);
    return smoothed * smoothed * subpixelStrength;
}

float4 main(FSInput input) : SV_Target
{
    const float2 uv = input.uv;
    const float4 centre = image.SampleLevel(linearSampler, uv, 0.0);
    const float lumaCentre = Luma(centre.rgb);
    const float lumaNorth = LumaOffset(uv, float2(0.0, -1.0));
    const float lumaSouth = LumaOffset(uv, float2(0.0, 1.0));
    const float lumaWest = LumaOffset(uv, float2(-1.0, 0.0));
    const float lumaEast = LumaOffset(uv, float2(1.0, 0.0));

    const float lumaMax = max(lumaCentre, max(max(lumaNorth, lumaSouth), max(lumaWest, lumaEast)));
    const float lumaMin = min(lumaCentre, min(min(lumaNorth, lumaSouth), min(lumaWest, lumaEast)));
    const float lumaRange = lumaMax - lumaMin;
    if (lumaRange < max(kEdgeThresholdMin, lumaMax * kEdgeThreshold))
        return centre;

    const float lumaNorthWest = LumaOffset(uv, float2(-1.0, -1.0));
    const float lumaNorthEast = LumaOffset(uv, float2(1.0, -1.0));
    const float lumaSouthWest = LumaOffset(uv, float2(-1.0, 1.0));
    const float lumaSouthEast = LumaOffset(uv, float2(1.0, 1.0));

    // Is the edge horizontal (luma changes from north to south) or vertical?
    const float edgeHorizontal = abs(lumaNorthWest + lumaSouthWest - 2.0 * lumaWest) +
                                 2.0 * abs(lumaNorth + lumaSouth - 2.0 * lumaCentre) +
                                 abs(lumaNorthEast + lumaSouthEast - 2.0 * lumaEast);
    const float edgeVertical = abs(lumaNorthWest + lumaNorthEast - 2.0 * lumaNorth) +
                               2.0 * abs(lumaWest + lumaEast - 2.0 * lumaCentre) +
                               abs(lumaSouthWest + lumaSouthEast - 2.0 * lumaSouth);
    const bool horizontal = edgeHorizontal >= edgeVertical;

    // The side of the pixel the edge lies on: the neighbour across it with the larger gradient.
    const float lumaBefore = horizontal ? lumaNorth : lumaWest;
    const float lumaAfter = horizontal ? lumaSouth : lumaEast;
    const float gradientBefore = abs(lumaBefore - lumaCentre);
    const float gradientAfter = abs(lumaAfter - lumaCentre);
    const bool before = gradientBefore >= gradientAfter;
    const float gradient = max(gradientBefore, gradientAfter);
    float stepLength = horizontal ? texelSize.y : texelSize.x;
    const float lumaOther = before ? lumaBefore : lumaAfter;
    if (before)
        stepLength = -stepLength;
    const float lumaEdge = 0.5 * (lumaCentre + lumaOther);

    // Search both ways along the edge, half a texel onto it, until the luma leaves the edge's.
    float2 edgeUv = uv;
    if (horizontal)
        edgeUv.y += stepLength * 0.5;
    else
        edgeUv.x += stepLength * 0.5;
    const float2 along = horizontal ? float2(texelSize.x, 0.0) : float2(0.0, texelSize.y);
    const float gradientScaled = gradient * kGradientScale;
    float2 uvNegative = edgeUv;
    float2 uvPositive = edgeUv;
    float deltaNegative = 0.0;
    float deltaPositive = 0.0;
    bool doneNegative = false;
    bool donePositive = false;
    [unroll] for (int step = 0; step < kSearchSteps; ++step)
    {
        if (!doneNegative)
        {
            uvNegative -= along * kSearchStep[step];
            deltaNegative = LumaAt(uvNegative) - lumaEdge;
            doneNegative = abs(deltaNegative) >= gradientScaled;
        }
        if (!donePositive)
        {
            uvPositive += along * kSearchStep[step];
            deltaPositive = LumaAt(uvPositive) - lumaEdge;
            donePositive = abs(deltaPositive) >= gradientScaled;
        }
    }

    // The nearer end decides the blend; none where the edge bends away from this pixel.
    const float distanceNegative = horizontal ? uv.x - uvNegative.x : uv.y - uvNegative.y;
    const float distancePositive = horizontal ? uvPositive.x - uv.x : uvPositive.y - uv.y;
    const bool negativeNearer = distanceNegative < distancePositive;
    const float distanceNearer = min(distanceNegative, distancePositive);
    const float edgeLength = distanceNegative + distancePositive;
    const bool centreBelowEdge = lumaCentre < lumaEdge;
    const bool endBelowEdge = (negativeNearer ? deltaNegative : deltaPositive) < 0.0;
    const float edgeBlend = endBelowEdge != centreBelowEdge ? 0.5 - distanceNearer / edgeLength : 0.0;

    const float neighbourAverage = (2.0 * (lumaNorth + lumaSouth + lumaWest + lumaEast) + lumaNorthWest +
                                    lumaNorthEast + lumaSouthWest + lumaSouthEast) / 12.0;
    const float blend = max(edgeBlend, SubpixelBlend(lumaCentre, lumaRange, neighbourAverage));

    float2 blendUv = uv;
    if (horizontal)
        blendUv.y += blend * stepLength;
    else
        blendUv.x += blend * stepLength;
    return float4(image.SampleLevel(linearSampler, blendUv, 0.0).rgb, centre.a);
}
