//////////////////////////////////////////////////////////////////////////
//  CSWaterTerrain.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>

#include "Random.h"
#include "ZzzOpenglUtil.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzTexture.h"
#include "ZzzCharacter.h"
#include "Scenes/SceneCore.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "ZzzLodTerrain.h"
#include "CSWaterTerrain.h"
#include "MapManager.h"

extern  double   WorldTime;
extern  float   TerrainMappingAlpha[TERRAIN_SIZE * TERRAIN_SIZE];
extern  float   g_chrome[MAX_VERTICES][2];

namespace
{
    constexpr double kAutoWaveIntervalMs = (40.0 / 25.0) * 1000.0; // 1600 ms
    constexpr float kDefaultWaterHeight = 350.0f;
    constexpr float kTileHeightOffset = 400.0f;
}

void CSWaterTerrain::Init(void)
{
    Vector(1.f, -1.f, 1.f, m_vLightVector);
}

void CSWaterTerrain::Update(void)
{
    if (!gMapManager.InHellas(m_iMapIndex))
    {
        // We're not in Kalima, so we don't need to update.
        return;
    }

    SpawnAmbientWave(WorldTime);

    m_iWaterPage ^= 1;
    calcWave();
    calcBaseWave();
}

void    CSWaterTerrain::Render(void)
{
    if (!gMapManager.InHellas(m_iMapIndex)) return;

    CreateTerrain((Hero->PositionX) * 2, (Hero->PositionY) * 2);

    float alpha;
    int   offset;
    int   i, j;
    for (i = 0; i < MAX_WATER_GRID * MAX_WATER_GRID; i++)
    {
        float* Normal = m_Normals[i];
        g_chrome[i][0] = Normal[2] * 0.5f + 0.1f;
        g_chrome[i][1] = Normal[1] * 0.5f + 0.5f;
    }

    EnableAlphaTest();
    BindTexture(BITMAP_MAPTILE);
    glBegin(GL_TRIANGLES);
    glColor3f(0.2f, 0.5f, 0.65f);
    for (j = 0; j < m_iTriangleListNum; j++)
    {
        offset = m_iTriangleList[j];
        glTexCoord2f(g_chrome[offset][1], g_chrome[offset][0]);
        glVertex3fv(m_Vertices[offset]);
    }
    glEnd();
    EnableAlphaBlend();
    BindTexture(BITMAP_MAPTILE + 1);
    glBegin(GL_TRIANGLES);
    for (j = 0; j < m_iTriangleListNum; j++)
    {
        offset = m_iTriangleList[j];
        alpha = 1.f - DotProduct(m_Normals[offset], m_vLightVector);
        glColor3f(alpha, alpha * 2.5f, alpha * 3.f);//, alpha );
        glTexCoord2f(g_chrome[offset][1], g_chrome[offset][0]);
        glVertex3fv(m_Vertices[offset]);
    }
    glEnd();
}

void CSWaterTerrain::SpawnAmbientWave(double currentTimeMs)
{
    if ((currentTimeMs - m_lastAutoWaveTime) < kAutoWaveIntervalMs)
    {
        return;
    }

    const int heroWaveX = static_cast<int>(Hero->PositionX * 2.0f);
    const int waveX = heroWaveX + Random::RangeInt(-15, 14);
    const int waveY = static_cast<int>((Hero->PositionY) * 2) + 25;
    addSineWave(waveX, waveY, 20, 2, 2000);
    m_lastAutoWaveTime = currentTimeMs;
}

void    CSWaterTerrain::CreateTerrain(int x, int y)
{
    float   fHeight, fHeight1;
    int     offset;
    constexpr int grid = MAX_WATER_GRID / 2;

    m_iTriangleListNum = 0;
    for (int offY = 0, i = y - grid + 6; offY < MAX_WATER_GRID; ++i, offY++)
    {
        for (int offX = 0, j = x - grid - 4; offX < MAX_WATER_GRID; ++j, offX++)
        {
            if (i < 0 || j < 0) fHeight = kDefaultWaterHeight;
            else if (i >= WATER_TERRAIN_SIZE || j >= WATER_TERRAIN_SIZE) fHeight = kDefaultWaterHeight;
            else
            {
                offset = j + (i * WATER_TERRAIN_SIZE);

                fHeight = m_iWaveHeight[m_iWaterPage][offset] + kDefaultWaterHeight;

                fHeight1 = m_iWaveHeight[2][offset] + kDefaultWaterHeight;
                fHeight1 += m_iWaveHeight[3][offset] + kDefaultWaterHeight;

                fHeight = (fHeight + fHeight1 / 2.f) * 0.5f;
            }

            offset = offX + (offY * MAX_WATER_GRID);
            Vector(static_cast<float>(j) * WAVE_SCALE, static_cast<float>(i) * WAVE_SCALE, fHeight, m_Vertices[offset]);
            Vector(0.f, 0.f, 0.f, m_Normals[offset]);
            if (offX >= MAX_WATER_GRID - 1 || offY >= MAX_WATER_GRID - 1)
            {
                VectorCopy(m_Normals[offset - 1], m_Normals[offset]);
                continue;
            }

            if (((offX % 2) == 0 && (offY % 2) == 0) || ((offX % 2) == 1 && (offY % 2) == 1))
            {
                m_iTriangleList[m_iTriangleListNum + 0] = offset;
                m_iTriangleList[m_iTriangleListNum + 1] = offset + 1;
                m_iTriangleList[m_iTriangleListNum + 2] = offset + 1 + MAX_WATER_GRID;

                m_iTriangleList[m_iTriangleListNum + 3] = offset;
                m_iTriangleList[m_iTriangleListNum + 4] = offset + 1 + MAX_WATER_GRID;
                m_iTriangleList[m_iTriangleListNum + 5] = offset + MAX_WATER_GRID;
            }
            else
            {
                m_iTriangleList[m_iTriangleListNum + 0] = offset;
                m_iTriangleList[m_iTriangleListNum + 1] = offset + 1;
                m_iTriangleList[m_iTriangleListNum + 2] = offset + MAX_WATER_GRID;

                m_iTriangleList[m_iTriangleListNum + 3] = offset + 1;
                m_iTriangleList[m_iTriangleListNum + 4] = offset + 1 + MAX_WATER_GRID;
                m_iTriangleList[m_iTriangleListNum + 5] = offset + MAX_WATER_GRID;
            }

            m_iTriangleListNum += 6;
        }
    }

    std::array<int, MAX_WATER_GRID * MAX_WATER_GRID> normalUseCounts{};
    vec3_t  normalV;
    for (int j = 0; j < m_iTriangleListNum; j += 3)
    {
        const int v1 = m_iTriangleList[j + 0];
        const int v2 = m_iTriangleList[j + 1];
        const int v3 = m_iTriangleList[j + 2];

        FaceNormalize(m_Vertices[v1], m_Vertices[v2], m_Vertices[v3], normalV);

        VectorAdd(normalV, m_Normals[v1], m_Normals[v1]);
        VectorAdd(normalV, m_Normals[v2], m_Normals[v2]);
        VectorAdd(normalV, m_Normals[v3], m_Normals[v3]);

        normalUseCounts[v1]++;
        normalUseCounts[v2]++;
        normalUseCounts[v3]++;
    }

    for (int i = 0; i < MAX_WATER_GRID * MAX_WATER_GRID; ++i)
    {
        if (normalUseCounts[i] > 0)
        {
            const float invCount = 1.0f / static_cast<float>(normalUseCounts[i]);
            m_Normals[i][0] *= invCount;
            m_Normals[i][1] *= invCount;
            m_Normals[i][2] *= invCount;
            VectorNormalize(m_Normals[i]);
        }
    }
}

void CSWaterTerrain::addSineWave(int x, int y, int radiusX, int radiusY, int height)
{
    if (radiusX <= 0 || radiusY <= 0 || height == 0)
    {
        return;
    }

    int* waveBuffer = m_iWaveHeight[m_iWaterPage].data();

    const float length = (1024.f / static_cast<float>(radiusX)) * (1024.f / static_cast<float>(radiusX));

    if (x < 0)
    {
        const int minX = 1 + radiusX;
        const int maxX = WATER_TERRAIN_SIZE - radiusX - 1;
        x = Random::RangeInt(minX, std::max<int>(minX, maxX));
    }
    if (y < 0)
    {
        const int minY = 1 + radiusY;
        const int maxY = WATER_TERRAIN_SIZE - radiusY - 1;
        y = Random::RangeInt(minY, std::max<int>(minY, maxY));
    }

    const int radsquare = (radiusX * radiusY);

    int left = -radiusX;
    int right = radiusX;
    int top = -radiusY;
    int bottom = radiusY;

    // Perform edge clipping...
    if ((x - radiusX) < 1) left -= (x - radiusX - 1);
    if ((y - radiusY) < 1) top -= (y - radiusY - 1);
    if ((x + radiusX) > WATER_TERRAIN_SIZE - 1) right -= (x + radiusX - WATER_TERRAIN_SIZE + 1);
    if ((y + radiusY) > WATER_TERRAIN_SIZE - 1) bottom -= (y + radiusY - WATER_TERRAIN_SIZE + 1);

    for (int cy = top; cy < bottom; ++cy)
    {
        for (int cx = left; cx < right; ++cx)
        {
            const int square = cy * cy + cx * cx;
            if (square < radsquare)
            {
                const float dist = std::sqrt(static_cast<float>(square) * length);
                int   sine = static_cast<int>((std::cos(dist) + 0xffff) * height) >> 19;
                sine *= FPS_ANIMATION_FACTOR;
                waveBuffer[WATER_TERRAIN_SIZE * (cy + y) + cx + x] += sine;
            }
        }
    }
}

void    CSWaterTerrain::calcBaseWave(void)
{
    /*
        if ( rand_fps_check(10) )
        {
            m_iSelectWaveX = rand()%WATER_TERRAIN_SIZE;
            m_iSelectWaveY = rand()%WATER_TERRAIN_SIZE;
            m_iAddHeight   = rand()%20+10;
        }
    */
    int MaxHeight;
    int offset;
    int HeroX = static_cast<int>((Hero->PositionX) * 2);
    int HeroY = static_cast<int>((Hero->PositionY) * 2);
    /*
        int HeroX = ( Hero->Object.Position[0]/TERRAIN_SCALE )*2;
        int HeroY = ( Hero->Object.Position[1]/TERRAIN_SCALE )*2;
    */

    int StartX = std::max<int>(0, HeroX - (VIEW_WATER_GRID / 2));
    int StartY = std::max<int>(0, HeroY - (VIEW_WATER_GRID / 2));
    int EndX = std::min<int>(WATER_TERRAIN_SIZE, HeroX + (VIEW_WATER_GRID / 2));
    int EndY = std::min<int>(WATER_TERRAIN_SIZE, HeroY + (VIEW_WATER_GRID / 2));
    for (int i = StartY; i < EndY; i++)        //  y
    {
        for (int j = StartX; j < EndX; j++)    //  x
        {
            offset = j + (i * WATER_TERRAIN_SIZE);

            //  ū ���ΰ
            float alpha = 0.f;//TerrainMappingAlpha[(j/2)+(i/2)*WATER_TERRAIN_SIZE];

            MaxHeight = (int)(sin((WorldTime * 0.005f) + (i * 0.1f) + (j * 0.1f)) * 50 * (1 + alpha));
            m_iWaveHeight[2][offset] = (int)(MaxHeight - sin((WorldTime * 0.003f) + (j * 0.1f) + (i * 0.5f)) * 50 * (1 + alpha));

            //  ���� ���ΰ
            MaxHeight = (int)(sin((WorldTime * 0.001f) + (i * 0.5f) + (j * 0.5f)) * 25 * (1 + alpha));
            m_iWaveHeight[3][offset] = (int)(MaxHeight - sin((WorldTime * 0.002f) + (j * 1.f) + (i * 0.3f)) * 25 * (1 + alpha));
        }
    }
}

void CSWaterTerrain::calcWave(void)
{
    int newh;

    int* newptr = m_iWaveHeight[m_iWaterPage].data();
    int* oldptr = m_iWaveHeight[m_iWaterPage ^ 1].data();

    int x;
    int y = (WATER_TERRAIN_SIZE - 1) * WATER_TERRAIN_SIZE;
    for (int count = WATER_TERRAIN_SIZE + 1; count < y; count += 2)
    {
        for (x = count + WATER_TERRAIN_SIZE - 2; count < x; count++)
        {
            newh = ((oldptr[count + WATER_TERRAIN_SIZE]
                + oldptr[count - WATER_TERRAIN_SIZE]
                + oldptr[count + 1]
                + oldptr[count - 1]
                ) >> 1)
                - newptr[count];
            newh *= FPS_ANIMATION_FACTOR;
            newptr[count] = newh - (newh >> 4);
        }
    }
}

float CSWaterTerrain::GetWaterTerrain(float xf, float yf)
{
    int x = static_cast<int>(xf / TERRAIN_SCALE * 2);
    int y = static_cast<int>(yf / TERRAIN_SCALE * 2);

    x = std::clamp(x, 0, WATER_TERRAIN_SIZE - 1);
    y = std::clamp(y, 0, WATER_TERRAIN_SIZE - 1);

    float fHeight;
    if (m_iWaterPage)
    {
        fHeight = m_iWaveHeight[1][x + (y * WATER_TERRAIN_SIZE)] + kDefaultWaterHeight;
    }
    else
    {
        fHeight = m_iWaveHeight[0][x + (y * WATER_TERRAIN_SIZE)] + kDefaultWaterHeight;
    }

    float fHeight1 = m_iWaveHeight[2][x + (y * WATER_TERRAIN_SIZE)] + kDefaultWaterHeight;
    fHeight1 += m_iWaveHeight[3][x + (y * WATER_TERRAIN_SIZE)] + kDefaultWaterHeight;
    fHeight = (fHeight + fHeight1 / 2.f) * 0.25f;

    return fHeight;
}

void CSWaterTerrain::RenderWaterAlphaBitmap(int Texture, float xf, float yf, float SizeX, float SizeY, vec3_t Light, float Rotation, float Alpha, float Height)
{
    if (Alpha == 1.f)
        glColor3fv(Light);
    else
        glColor4f(Light[0], Light[1], Light[2], Alpha);

    vec3_t Angle;
    Vector(0.f, 0.f, Rotation, Angle);
    float Matrix[3][4];
    AngleMatrix(Angle, Matrix);

    BindTexture(Texture);
    float mxf = (xf / TERRAIN_SCALE * 2);
    float myf = (yf / TERRAIN_SCALE * 2);
    int   mxi = (int)(mxf);
    int   myi = (int)(myf);

    float sizeMax = std::max<float>(SizeX, SizeY);
    float TexU = (((float)mxi - mxf) + 0.5f * sizeMax);
    float TexV = (((float)myi - myf) + 0.5f * sizeMax);
    float TexScaleU = 1.f / sizeMax;
    float TexScaleV = 1.f / sizeMax;
    sizeMax = static_cast<float>(static_cast<int>(sizeMax) + 1);
    float Aspect = SizeX / SizeY;
    for (float y = -sizeMax; y <= sizeMax; y += 1.f)
    {
        for (float x = -sizeMax; x <= sizeMax; x += 1.f)
        {
            vec3_t p1[4], p2[4];
            Vector((TexU + x) * TexScaleU, (TexV + y) * TexScaleV, 0.f, p1[0]);
            Vector((TexU + x + 1.f) * TexScaleU, (TexV + y) * TexScaleV, 0.f, p1[1]);
            Vector((TexU + x + 1.f) * TexScaleU, (TexV + y + 1.f) * TexScaleV, 0.f, p1[2]);
            Vector((TexU + x) * TexScaleU, (TexV + y + 1.f) * TexScaleV, 0.f, p1[3]);
            //bool Clip = false;
            for (int i = 0; i < 4; i++)
            {
                p1[i][0] -= 0.5f;
                p1[i][1] -= 0.5f;
                VectorRotate(p1[i], Matrix, p2[i]);
                p2[i][0] *= Aspect;
                p2[i][0] += 0.5f;
                p2[i][1] += 0.5f;
                //if((p2[i][0]>=0.f && p2[i][0]<=1.f) || (p2[i][1]>=0.f && p2[i][1]<=1.f)) Clip = true;
            }
            //if(Clip==true)
            RenderWaterBitmapTile((float)mxi + x, (float)myi + y, 1.f, 1, p2, false, Alpha, Height);
        }
    }
}

void    CSWaterTerrain::RenderWaterBitmapTile(float xf, float yf, float lodf, int lodi, vec3_t c[4], bool LightEnable, float Alpha, float Height)
{
    vec3_t TerrainVertex[4];
    int xi = (int)xf;
    int yi = (int)yf;
    if (xi < 0 || yi < 0 || xi >= TERRAIN_SIZE_MASK || yi >= TERRAIN_SIZE_MASK) return;
    float TileScale = WAVE_SCALE;
    float sx = xf * WAVE_SCALE;
    float sy = yf * WAVE_SCALE;
    int TerrainIndex1 = xi + (yi * WATER_TERRAIN_SIZE);
    int TerrainIndex2 = xi + lodi + (yi * WATER_TERRAIN_SIZE);
    int TerrainIndex3 = xi + lodi + ((yi + lodi) * WATER_TERRAIN_SIZE);
    int TerrainIndex4 = xi + ((yi + lodi) * WATER_TERRAIN_SIZE);
    Vector(sx, sy, m_iWaveHeight[0][TerrainIndex1] + kTileHeightOffset + Height, TerrainVertex[0]);
    Vector(sx + TileScale, sy, m_iWaveHeight[0][TerrainIndex2] + kTileHeightOffset + Height, TerrainVertex[1]);
    Vector(sx + TileScale, sy + TileScale, m_iWaveHeight[0][TerrainIndex3] + kTileHeightOffset + Height, TerrainVertex[2]);
    Vector(sx, sy + TileScale, m_iWaveHeight[0][TerrainIndex4] + kTileHeightOffset + Height, TerrainVertex[3]);

    vec3_t Light[4];
    if (LightEnable)
    {
        VectorCopy(PrimaryTerrainLight[TerrainIndex1], Light[0]);
        VectorCopy(PrimaryTerrainLight[TerrainIndex2], Light[1]);
        VectorCopy(PrimaryTerrainLight[TerrainIndex3], Light[2]);
        VectorCopy(PrimaryTerrainLight[TerrainIndex4], Light[3]);
    }

    glBegin(GL_TRIANGLE_FAN);
    for (int i = 0; i < 4; i++)
    {
        if (LightEnable)
        {
            if (Alpha == 1.f)
                glColor3fv(Light[i]);
            else
                glColor4f(Light[i][0], Light[i][1], Light[i][2], Alpha);
        }
        glTexCoord2f(c[i][0], c[i][1]);
        glVertex3fv(TerrainVertex[i]);
    }
    glEnd();
}