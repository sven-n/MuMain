//////////////////////////////////////////////////////////////////////////
//  CSWaterTerrain.h
//////////////////////////////////////////////////////////////////////////
#ifndef __CSWATER_TERRAIN_H__
#define __CSWATER_TERRAIN_H__

class CSWaterTerrain
{
private:
    //    BYTE    m_bAttribute[WATER_TERRAIN_SIZE*WATER_TERRAIN_SIZE];
    int     m_iMapIndex;
    int     m_iWaveHeight[4][WATER_TERRAIN_SIZE * WATER_TERRAIN_SIZE];
    int     m_iWaterPage;

    vec3_t  m_vLightVector;
    vec3_t  m_Vertices[MAX_WATER_GRID * MAX_WATER_GRID];
    vec3_t  m_Normals[MAX_WATER_GRID * MAX_WATER_GRID];
    int     m_iTriangleList[MAX_WATER_GRID * MAX_WATER_GRID * 6];
    int     m_iTriangleListNum;

    /*
        int     m_iSelectWaveX, m_iSelectWaveY;
        int     m_iAddHeight;
    */

    void    calcBaseWave(void);
    void    calcWave(void);

    void    CreateTerrain(int x, int y);

    void    RenderWaterBitmapTile(float xf, float yf, float lodf, int lodi, vec3_t c[4], bool LightEnable, float Alpha, float Height = 0.f);

public:
    CSWaterTerrain(int map) : m_iMapIndex(map), m_iWaterPage(0), m_iTriangleListNum(0) { Init(); };
    ~CSWaterTerrain(void) {};

    void    Init(void);
    void    Update(void);
    void    Render(void);

    void    addSineWave(int x, int y, int radiusX, int radiusY, int height);
    float   GetWaterTerrain(float xf, float yf);
    void    RenderWaterAlphaBitmap(int Texture, float xf, float yf, float SizeX, float SizeY, vec3_t Light, float Rotation, float Alpha, float Height);
};

#endif// __CSWATER_TERRAIN_H__
