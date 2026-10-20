// BMD body shadow: skins rest-pose geometry, projects it onto the terrain as CalcShadowPosition() does.
#ifdef MU_D3D12_RAW_STORAGE_BUFFER
ByteAddressBuffer boneRows : register(t0, space0);
ByteAddressBuffer terrainWords : register(t1, space0);
static const uint BoneRowSizeBytes = 16;
#else
StructuredBuffer<float4> boneRows : register(t0, space0);
StructuredBuffer<uint> terrainWords : register(t1, space0);
#endif

// Same layout as skinned_textured.vert.hlsl.
cbuffer Skinning : register(b0, space1)
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

cbuffer Shadow : register(b1, space1)
{
    float4 shadowOrigin;     // xyz: BodyOrigin, w: height of TW_HEIGHT cells
    float4 shadowProjection; // x, y: sx, sy; z: 0 where RequestTerrainHeight() returns 0
};

// The skinned vertex layout; glslang would otherwise number these 0, 1, 2 (dxc ignores vk::location).
struct VSInput
{
    [[vk::location(0)]] float3 pos : TEXCOORD0;
    [[vk::location(3)]] float4 color : TEXCOORD3;
    [[vk::location(4)]] int positionBoneIndex : TEXCOORD4;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD0;
    float4 color : TEXCOORD1;
    float fogFactor : TEXCOORD2;
    float clipDist : SV_ClipDistance0;
};

// Render::TerrainHeightGrid: 256 * 256 heights, then one TW_HEIGHT bit per cell.
static const uint TerrainSize = 256;
static const uint TerrainCells = TerrainSize * TerrainSize;

// These two come first: spirv-cross numbers MSL buffers in order of first use, and SDL wants the uniform
// buffers (Skinning, then Shadow) ahead of the storage buffers.
float4 clipPosition(float3 position)
{
    return mul(mvp, float4(position, 1.0));
}

float3 shearShadow(float3 position)
{
    float3 relative = position - shadowOrigin.xyz;
    relative.x += relative.z * (relative.x + shadowProjection.x) / (relative.z - shadowProjection.y);
    return relative + shadowOrigin.xyz;
}

#ifdef MU_D3D12_RAW_STORAGE_BUFFER
float4 loadBoneRow(uint row)
{
    return asfloat(boneRows.Load4(row * BoneRowSizeBytes));
}

uint loadTerrainWord(uint word)
{
    return terrainWords.Load(word * 4);
}
#else
float4 loadBoneRow(uint row)
{
    return boneRows[row];
}

uint loadTerrainWord(uint word)
{
    return terrainWords[word];
}
#endif

// skinned_textured.vert.hlsl's transformPosition().
float3 transformPosition(float3 position, int boneIndex)
{
    if (boneIndex < 0 || boneIndex >= int(palette.y))
    {
        return position;
    }

    const uint row = palette.x + uint(boneIndex) * 3;
    const float4 row0 = loadBoneRow(row);
    const float4 row1 = loadBoneRow(row + 1);
    const float4 row2 = loadBoneRow(row + 2);
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

float terrainCornerHeight(int x, int y)
{
    return asfloat(loadTerrainWord(uint(y & 255) * TerrainSize + uint(x & 255)));
}

// Render::SampleTerrainHeightGrid().
float terrainHeight(float x, float y)
{
    if (shadowProjection.z == 0.0 || x < 0.0 || y < 0.0)
    {
        return 0.0;
    }

    x /= 100.0;
    y /= 100.0;
    const int xi = int(x);
    const int yi = int(y);

    // An x past the last column runs into the next row; only the corners wrap.
    const uint index = uint(yi * int(TerrainSize) + xi);
    if (index >= TerrainCells || ((loadTerrainWord(TerrainCells + (index >> 5)) >> (index & 31)) & 1) != 0)
    {
        return shadowOrigin.w;
    }

    const float xd = x - float(xi);
    const float yd = y - float(yi);
    const float h1 = terrainCornerHeight(xi, yi);
    const float h2 = terrainCornerHeight(xi, yi + 1);
    const float h3 = terrainCornerHeight(xi + 1, yi);
    const float h4 = terrainCornerHeight(xi + 1, yi + 1);

    const float left = h1 + (h2 - h1) * yd;
    const float right = h3 + (h4 - h3) * yd;
    return left + (right - left) * xd;
}

VSOutput main(VSInput input)
{
    VSOutput output;

    float3 position = shearShadow(transformPosition(input.pos, input.positionBoneIndex));
    position.z = terrainHeight(position.x, position.y) + 5.0;

    output.pos = clipPosition(position);
    output.uv = float2(0.0, 0.0);
    output.color = input.color;
    output.clipDist = output.pos.w - 1.0;
    const float range = fogParameters.y - fogParameters.x;
    output.fogFactor = range > 0.001
        ? saturate((fogParameters.y - abs(output.pos.w)) / range)
        : 1.0;
    return output;
}
