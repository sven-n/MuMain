#pragma once

extern vec3_t		 PrimaryTerrainLight[];
extern vec3_t		 BackTerrainLight[];
extern vec3_t		 TerrainLight[];
extern float		 PrimaryTerrainHeight[];
extern float		 BackTerrainHeight[];
extern unsigned char TerrainMappingLayer1[];
extern unsigned char TerrainMappingLayer2[];
extern float         TerrainMappingAlpha[];
extern WORD          TerrainWall[];

extern inline int TERRAIN_INDEX(int x, int y);
extern inline int TERRAIN_INDEX_REPEAT(int x, int y);
extern inline BYTE TERRAIN_ATTRIBUTE(float x, float y);

bool OpenTerrainHeight(wchar_t* name);
void SaveTerrainHeight(wchar_t* name);
bool OpenTerrainHeightNew(const wchar_t* strFilename);

void PrefixTerrainHeight();

void CreateTerrainNormal();
void CreateTerrainLight();
void CreateTerrainNormal_Part(int xi, int yi);
void CreateTerrainLight_Part(int xi, int yi);

void CreateTerrain(wchar_t* FileName, bool bNew = false);

bool IsTerrainHeightExtMap(int iWorld);

int OpenTerrainMapping(wchar_t* FileName);
bool SaveTerrainMapping(wchar_t* FileName, int iMapNumber);
int OpenTerrainAttribute(wchar_t* FileName);
bool SaveTerrainAttribute(wchar_t* FileName, int iMapNumber);

//  속성 변경.
void AddTerrainAttribute(int x, int y, BYTE att);
void SubTerrainAttribute(int x, int y, BYTE att);
void AddTerrainAttributeRange(int x, int y, int dx, int dy, BYTE att, BYTE Add = 0);
void SetTerrainWaterState(std::list<int>& terrainIndex, int state = 0);

//Vector RequestTerrainNormal(float xf,float yf);
void AddTerrainHeight(float xf, float yf, float Height, int Range, float* Buffer);

void InitTerrainRay(int HeroX, int HeroY);
void InitTerrainLight();
void InitTerrainShadow();
void SetTerrainLight(float xf, float yf, vec3_t Light, int Range, vec3_t* Buffer);
void AddTerrainLight(float xf, float yf, vec3_t Light, int Range, vec3_t* Buffer);
void AddTerrainLightClip(float xf, float yf, vec3_t Light, int Range, vec3_t* Buffer);
void RequestTerrainLight(float xf, float yf, vec3_t Light);
void OpenTerrainLight(wchar_t* FileName);
void SaveTerrainLight(wchar_t* FileName);

void CreateFrustrum(float xAspect, float yAspect, vec3_t position);
void CreateLodBuffer();

float RequestTerrainHeight(float xf, float yf);
bool TestFrustrum(vec3_t Position, float Range);
bool TestFrustrum2D(float x, float y, float Range);

bool RenderTerrainTile(float xf, float yf, int xi, int yi, float lodf, int lodi, bool Flag);
void RenderTerrain(bool EditFlag);

void RenderTerrainTile_After(float xf, float yf, int xi, int yi, float lodf, int lodi, bool Flag);
void RenderTerrain_After(bool EditFlag);

void RenderTerrainBitmap(int Texture, int mxi, int myi, float Rotation);
void RenderTerrainAlphaBitmap(int Texture, float xf, float yf, float SizeX, float SizeY, vec3_t Light, float Rotation = 0.f, float Alpha = 1.f, float Height = 5.f);

void CreateSun();
void RenderSky();
void RenderSun();

extern float SelectXF;
extern float SelectYF;
extern int   CurrentLayer;

extern const float g_fMinHeight;
extern const float g_fMaxHeight;

#ifdef DYNAMIC_FRUSTRUM

#include <map>
class CFrustrum
{
private:
    float m_FrustrumD[5];
    vec3_t m_FrustrumNorm[5];

    vec3_t	m_vEye;
    float	m_fFov;
    float	m_fAspect;
    float	m_fDist;

    void Make(vec3_t vEye, float fFov, float fAspect, float fDist);

public:
    CFrustrum() {}
    ~CFrustrum() {}

    //3D프러스텀 캡슐화
    void Create(vec3_t vEye, float fFov, float fAspect, float fDist);
    bool Test(vec3_t vPos, float fRange);
    void Reset();

    inline void SetEye(vec3_t vEye) { VectorCopy(vEye, m_vEye); }
    inline void SetFOV(float fFov) { m_fFov = fFov; }
    inline void SetAspect(float fAspect) { m_fAspect = fAspect; }
    inline void SetDist(float fDist) { m_fDist = fDist; }
};

void ResetAllFrustrum();
void DeleteAllFrustrum();
CFrustrum* FindFrustrum(unsigned int iID);
typedef std::map<unsigned int, CFrustrum* > FrustrumMap_t;
extern FrustrumMap_t g_FrustrumMap; //전체 프러스텀과 별개로 아이디로 지정할 수 있는 프러스텀
#endif //DYNAMIC_FRUSTRUM

inline int MapFileEncrypt(BYTE* pbyDst, BYTE* pbySrc, int iSize)
{
    if (!pbyDst)
    {
        return (iSize);
    }
    BYTE byMapXorKey[16] = { 0xD1, 0x73, 0x52, 0xF6, 0xD2, 0x9A, 0xCB, 0x27,
                            0x3E, 0xAF, 0x59, 0x31, 0x37, 0xB3, 0xE7, 0xA2 };

    WORD wMapKey = 0x5E;
    for (int i = 0; i < iSize; ++i)
    {
        pbyDst[i] = (pbySrc[i] + (BYTE)wMapKey) ^ byMapXorKey[i % 16];

        wMapKey = pbyDst[i] + 0x3D;
        wMapKey = wMapKey & 0xFF;
    }

    return (iSize);
}

inline int MapFileDecrypt(BYTE* pbyDst, BYTE* pbySrc, int iSize)
{
    if (!pbyDst)
    {
        return (iSize);
    }
    BYTE byMapXorKey[16] = { 0xD1, 0x73, 0x52, 0xF6, 0xD2, 0x9A, 0xCB, 0x27,
                            0x3E, 0xAF, 0x59, 0x31, 0x37, 0xB3, 0xE7, 0xA2 };

    WORD wMapKey = 0x5E;
    for (int i = 0; i < iSize; ++i)
    {
        pbyDst[i] = (pbySrc[i] ^ byMapXorKey[i % 16]) - (BYTE)wMapKey;
        wMapKey = pbySrc[i] + 0x3D;
        wMapKey = wMapKey & 0xFF;
    }
    return (iSize);
}
