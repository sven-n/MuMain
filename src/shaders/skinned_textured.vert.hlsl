// GPU bone skinning for BMD rest-pose geometry.
StructuredBuffer<float4> boneRows : register(t0, space0);

cbuffer Skinning : register(b1)
{
    float4x4 mvp;
    float4 bodyOriginAndScale;
    float4 boneScaleAndRestPoseScale;
    uint4 palette;
    float4 lightDirection;
    float4 fogParameters;
    uint4 textureCoordinateMode;
    float4 chromeParameters;
    float4 textureCoordinateParameters;
};

struct VSInput
{
    float3 pos : TEXCOORD0;
    float3 normal : TEXCOORD1;
    float2 uv : TEXCOORD2;
    float4 color : TEXCOORD3;
    int positionBoneIndex : TEXCOORD4;
    int normalBoneIndex : TEXCOORD5;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : TEXCOORD1;
    float fogFactor : TEXCOORD2;
    float clipDist : SV_ClipDistance0;
};

float3 transformPosition(float3 position, int boneIndex)
{
    if (boneIndex < 0 || boneIndex >= int(palette.y))
    {
        return position;
    }

    const uint row = palette.x + uint(boneIndex) * 3;
    const float4 row0 = boneRows[row];
    const float4 row1 = boneRows[row + 1];
    const float4 row2 = boneRows[row + 2];
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

    if (palette.z != 0)
    {
        transformed = bodyOriginAndScale.xyz + transformed * bodyOriginAndScale.w;
    }
    return transformed;
}

float3 transformNormal(float3 normal, int boneIndex)
{
    if (boneIndex < 0 || boneIndex >= int(palette.y))
    {
        return normal;
    }

    const uint row = palette.x + uint(boneIndex) * 3;
    return float3(dot(boneRows[row].xyz, normal),
                  dot(boneRows[row + 1].xyz, normal),
                  dot(boneRows[row + 2].xyz, normal));
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
    const float3 skinnedPosition = transformPosition(input.pos, input.positionBoneIndex);
    const float3 skinnedNormal = transformNormal(input.normal, input.normalBoneIndex);
    output.pos = mul(mvp, float4(skinnedPosition, 1.0));
    output.uv = getTextureCoordinates(input.uv, skinnedNormal);
    output.color = input.color;

    if (palette.w != 0)
    {
        const float luminosity = max(dot(skinnedNormal, lightDirection.xyz) * 0.8 + 0.4, 0.2);
        output.color.rgb = saturate(output.color.rgb * luminosity);
    }

    output.clipDist = output.pos.w - 1.0;
    const float range = fogParameters.y - fogParameters.x;
    output.fogFactor = range > 0.001
        ? saturate((fogParameters.y - abs(output.pos.w)) / range)
        : 1.0;
    return output;
}
