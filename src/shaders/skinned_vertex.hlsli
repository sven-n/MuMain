// The skinning vertex shader shared by skinned_textured.vert (BMD vertices) and
// skinned_weighted.vert (glTF vertices with up to four weighted joints and a tangent). The
// including file declares VSInput first, and defines MU_WEIGHTED_SKINNING for the weighted one.
// palette.w selects the lighting: 0 unlit, 1 Classic per-vertex lighting, 2 Enhanced lighting (the
// vertex colour stays unlit and skinned_lit.frag lights each pixel from the normal, view vector and
// light direction outputs), 3 Enhanced lighting plus the frame's point lights (world-space
// outputs). Other fragment shaders ignore the Enhanced outputs.
// Normal maps (Enhanced lighting only): a weighted vertex with a tangent (w = handedness) under a
// draw with a normal map (textureCoordinateParameters.w = the map's scale, 0: none) passes its
// tangent skinned like its normal, in draw and world space, for skinned_lit.frag; every other
// vertex passes zero tangents and scale 0.
#ifdef MU_D3D12_RAW_STORAGE_BUFFER
ByteAddressBuffer boneRows : register(t0, space0);
static const uint BoneRowSizeBytes = 16;
#else
StructuredBuffer<float4> boneRows : register(t0, space0);
#endif

cbuffer Skinning : register(b0, space1)
{
    float4x4 mvp;
    float4 bodyOriginAndScale;
    float4 boneScaleAndRestPoseScale;
    uint4 palette;
    float4 lightDirection; // xyz: the model light vector; w: the highlight knee
    float4 fogParameters;
    uint4 textureCoordinateMode;
    float4 chromeParameters;
    float4 textureCoordinateParameters;
    // xyz: camera position in this draw's vertex space; w: specular strength.
    float4 eyePositionAndSpecular;
    // Enhanced lighting: rows of the affine transform from this draw's vertex space to world space.
    float4 vertexToWorldRows[3];
};

static const uint LightingClassic = 1;
static const uint LightingEnhanced = 2;
static const uint LightingEnhancedDynamic = 3;

// The light mood's highlight shoulder (Render::Lighting::LightMood::RollOffHighlight): a lit
// colour channel above the knee approaches 1 smoothly instead of being cut off there. Only a knee
// between 0 and 1 rolls off; the callers keep the Classic cut-off expression verbatim otherwise,
// so that the driver compiles the Classic path to the same arithmetic as before.
bool RollsOffHighlight(float knee)
{
    return knee > 0.0 && knee < 1.0;
}

float RollOffChannel(float value, float knee)
{
    const float range = 1.0 - knee;
    return value > knee ? knee + range * (1.0 - exp(-max(value - knee, 0.0) / range)) : value;
}

float3 RollOffHighlight(float3 color, float knee)
{
    return saturate(float3(RollOffChannel(color.r, knee), RollOffChannel(color.g, knee),
                           RollOffChannel(color.b, knee)));
}


struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : TEXCOORD1;
    float fogFactor : TEXCOORD2;
    float eyeDepth : TEXCOORD3;
    float3 normal : TEXCOORD4;
    float3 viewVector : TEXCOORD5;
    nointerpolation float4 lightDirectionAndSpecular : TEXCOORD6;
    float4 worldPositionAndDynamicLights : TEXCOORD7; // w: 1 when the point lights apply
    float3 worldNormal : TEXCOORD8;
    float4 tangent : TEXCOORD9;                       // w: handedness, 0 without a normal map
    float4 worldTangentAndNormalMapScale : TEXCOORD10; // w: the map's scale, 0 without one
    float clipDist : SV_ClipDistance0;
};

#ifdef MU_D3D12_RAW_STORAGE_BUFFER
float4 loadBoneRow(uint row)
{
    return asfloat(boneRows.Load4(row * BoneRowSizeBytes));
}
#endif

void loadBone(uint boneIndex, out float4 row0, out float4 row1, out float4 row2)
{
    const uint row = palette.x + boneIndex * 3;
#ifdef MU_D3D12_RAW_STORAGE_BUFFER
    row0 = loadBoneRow(row);
    row1 = loadBoneRow(row + 1);
    row2 = loadBoneRow(row + 2);
#else
    row0 = boneRows[row];
    row1 = boneRows[row + 1];
    row2 = boneRows[row + 2];
#endif
}

float3 placeBody(float3 transformed)
{
    if (palette.z != 0)
    {
        transformed = bodyOriginAndScale.xyz + transformed * bodyOriginAndScale.w;
    }
    return transformed;
}

float3 transformPosition(float3 position, int boneIndex)
{
    if (boneIndex < 0 || boneIndex >= int(palette.y))
    {
        return position;
    }

    float4 row0;
    float4 row1;
    float4 row2;
    loadBone(uint(boneIndex), row0, row1, row2);
    float3 transformed;

    if (boneScaleAndRestPoseScale.x == 1.0)
    {
        const float restScale = boneScaleAndRestPoseScale.y != 0.0 ? boneScaleAndRestPoseScale.y : 1.0;
        const float4 restPosition = float4(position * restScale, 1.0);
        transformed = float3(dot(row0, restPosition), dot(row1, restPosition), dot(row2, restPosition));
    }
    else
    {
        transformed = float3(dot(row0.xyz, position), dot(row1.xyz, position), dot(row2.xyz, position));
        transformed = transformed * boneScaleAndRestPoseScale.x + float3(row0.w, row1.w, row2.w);
    }

    return placeBody(transformed);
}

// The palette of a weighted draw already holds each joint's bone matrix times its inverse bind
// matrix with the bone and rest-pose scales applied (Render::Models::BuildWeightedSkinPalette).
float3 skinWeightedPosition(float3 position, uint4 joints, float4 weights)
{
    const float4 bindPosition = float4(position, 1.0);
    float3 transformed = float3(0.0, 0.0, 0.0);
    [unroll]
    for (uint k = 0; k < 4; ++k)
    {
        if (weights[k] > 0.0 && joints[k] < palette.y)
        {
            float4 row0;
            float4 row1;
            float4 row2;
            loadBone(joints[k], row0, row1, row2);
            transformed += weights[k] * float3(dot(row0, bindPosition), dot(row1, bindPosition), dot(row2, bindPosition));
        }
    }
    return placeBody(transformed);
}

float3 skinWeightedNormal(float3 normal, uint4 joints, float4 weights)
{
    float3 transformed = float3(0.0, 0.0, 0.0);
    [unroll]
    for (uint k = 0; k < 4; ++k)
    {
        if (weights[k] > 0.0 && joints[k] < palette.y)
        {
            float4 row0;
            float4 row1;
            float4 row2;
            loadBone(joints[k], row0, row1, row2);
            transformed += weights[k] * float3(dot(row0.xyz, normal), dot(row1.xyz, normal), dot(row2.xyz, normal));
        }
    }
    // The palette carries the skinning scale, which a normal does not take; a blend of joints
    // is normalised, a single joint keeps the length its bone matrix gives (as for a BMD).
    const float restScale = boneScaleAndRestPoseScale.y != 0.0 ? boneScaleAndRestPoseScale.y : 1.0;
    const float skinScale = boneScaleAndRestPoseScale.x == 1.0 ? restScale : boneScaleAndRestPoseScale.x;
    transformed /= skinScale;
    if (weights.y > 0.0)
    {
        const float lengthSquared = dot(transformed, transformed);
        transformed = lengthSquared > 1e-24 ? transformed * rsqrt(lengthSquared) : transformed;
    }
    return transformed;
}

float3 transformNormal(float3 normal, int boneIndex)
{
    if (boneIndex < 0 || boneIndex >= int(palette.y))
    {
        return normal;
    }

    float4 row0;
    float4 row1;
    float4 row2;
    loadBone(uint(boneIndex), row0, row1, row2);
    return float3(dot(row0.xyz, normal), dot(row1.xyz, normal), dot(row2.xyz, normal));
}

float2 getTextureCoordinates(float2 restCoordinates, float3 normal)
{
    const uint mode = textureCoordinateMode.x;
    const float chromeWave = chromeParameters.x;
    const float chromeWave2 = chromeParameters.y;
    const float2 chromeLight = chromeParameters.zw;
    const float2 offset = textureCoordinateParameters.xy;

    if (mode == 1)
    {
        return float2(normal.z * 0.5 + chromeWave, normal.y * 0.5 + chromeWave * 2.0);
    }
    if (mode == 2)
    {
        return float2((normal.z + normal.x) * 0.8 + chromeWave2 * 2.0,
                      normal.y + normal.x + chromeWave2 * 3.0);
    }
    if (mode == 3)
    {
        const float u = dot(normal, float3(0.0, -0.1, -0.8));
        return float2(u, 1.0 - u);
    }
    if (mode == 4 || mode == 5)
    {
        const float lightDot = dot(normal, float3(chromeLight, 1.0));
        const float scale = mode == 4 ? 0.5 : 2.5;
        const float uScale = mode == 4 ? 0.5 : 3.0;
        const float lightScale = mode == 4 ? 3.0 : 5.0;
        const float waveScale = mode == 4 ? 3.0 : 1.0;
        return float2(lightDot + normal.y * uScale + chromeLight.y * lightScale,
                      1.0 - lightDot - normal.z * scale - chromeWave * waveScale) + offset;
    }
    if (mode == 6)
    {
        const float coordinate = (normal.z + normal.x) * 0.8 + chromeWave2 * 2.0;
        return coordinate.xx;
    }
    if (mode == 7)
    {
        const float coordinate = (normal.z + normal.x) * 0.8 + textureCoordinateParameters.z;
        return coordinate.xx;
    }
    if (mode == 8)
    {
        return normal.xy * restCoordinates + offset;
    }
    if (mode == 9)
    {
        return float2(normal.z * 0.5 + 0.2, normal.y * 0.5 + 0.5);
    }
    return restCoordinates;
}

VSOutput main(VSInput input)
{
    VSOutput output;
    float3 skinnedPosition;
    float3 skinnedNormal;
#ifdef MU_WEIGHTED_SKINNING
    if (input.weights.x > 0.0)
    {
        skinnedPosition = skinWeightedPosition(input.pos, input.joints, input.weights);
        skinnedNormal = skinWeightedNormal(input.normal, input.joints, input.weights);
    }
    else
#endif
    {
        skinnedPosition = transformPosition(input.pos, input.positionBoneIndex);
        skinnedNormal = transformNormal(input.normal, input.normalBoneIndex);
    }
    output.pos = mul(mvp, float4(skinnedPosition, 1.0));
    output.uv = getTextureCoordinates(input.uv, skinnedNormal);
    output.color = input.color;

    if (palette.w == LightingClassic)
    {
        const float luminosity = max(dot(skinnedNormal, lightDirection.xyz) * 0.8 + 0.4, 0.2);
        output.color.rgb = saturate(output.color.rgb * luminosity);
        if (RollsOffHighlight(lightDirection.w))
            output.color.rgb = RollOffHighlight(input.color.rgb * luminosity, lightDirection.w);
    }

    output.clipDist = output.pos.w - 1.0;
    const float range = fogParameters.y - fogParameters.x;
    output.fogFactor = range > 0.001
        ? saturate((fogParameters.y - abs(output.pos.w)) / range)
        : 1.0;
    output.eyeDepth = abs(output.pos.w);
    output.normal = skinnedNormal;
    output.viewVector = eyePositionAndSpecular.xyz - skinnedPosition;
    output.lightDirectionAndSpecular = float4(lightDirection.xyz, eyePositionAndSpecular.w);
    const float4 position = float4(skinnedPosition, 1.0);
    output.worldPositionAndDynamicLights =
        float4(dot(vertexToWorldRows[0], position), dot(vertexToWorldRows[1], position),
               dot(vertexToWorldRows[2], position), palette.w == LightingEnhancedDynamic ? 1.0 : 0.0);
    output.worldNormal = float3(dot(vertexToWorldRows[0].xyz, skinnedNormal),
                                dot(vertexToWorldRows[1].xyz, skinnedNormal),
                                dot(vertexToWorldRows[2].xyz, skinnedNormal));
#ifdef MU_WEIGHTED_SKINNING
    const bool normalMapped = palette.w >= LightingEnhanced && input.weights.x > 0.0 && input.tangent.w != 0.0 &&
                              textureCoordinateParameters.w != 0.0;
    float3 skinnedTangent = float3(0.0, 0.0, 0.0);
    if (normalMapped)
    {
        skinnedTangent = skinWeightedNormal(input.tangent.xyz, input.joints, input.weights);
    }
    output.tangent = float4(skinnedTangent, normalMapped ? sign(input.tangent.w) : 0.0);
#else
    const bool normalMapped = false;
    const float3 skinnedTangent = float3(0.0, 0.0, 0.0);
    output.tangent = float4(0.0, 0.0, 0.0, 0.0);
#endif
    output.worldTangentAndNormalMapScale =
        float4(dot(vertexToWorldRows[0].xyz, skinnedTangent), dot(vertexToWorldRows[1].xyz, skinnedTangent),
               dot(vertexToWorldRows[2].xyz, skinnedTangent), normalMapped ? textureCoordinateParameters.w : 0.0);
    return output;
}
