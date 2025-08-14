///////////////////////////////////////////////////////////////////////////////
// Terrain 관련 함수
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <gl\gl.h>
#include <gl\glu.h>
#include <math.h>
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzLodTerrain.h"
#include "zzzpath.h"
#include "ZzzTexture.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzEffect.h"

#include "CSChaosCastle.h"
#include "CMVP1stDirection.h"
#include "CDirection.h"
#include "MapManager.h"

#include "w_MapHeaders.h"
#include "CameraMove.h"

//-------------------------------------------------------------------------------------------------------------

int  TerrainFlag;
bool ActiveTerrain = false;
bool TerrainGrassEnable = true;
bool DetailLowEnable = false;

char            LodBuffer[64 * 64];
vec3_t          TerrainNormal[TERRAIN_SIZE * TERRAIN_SIZE];
vec3_t          PrimaryTerrainLight[TERRAIN_SIZE * TERRAIN_SIZE];
vec3_t          BackTerrainLight[TERRAIN_SIZE * TERRAIN_SIZE];
vec3_t          TerrainLight[TERRAIN_SIZE * TERRAIN_SIZE];
float           PrimaryTerrainHeight[TERRAIN_SIZE * TERRAIN_SIZE];
float           BackTerrainHeight[TERRAIN_SIZE * TERRAIN_SIZE];
unsigned char   TerrainMappingLayer1[TERRAIN_SIZE * TERRAIN_SIZE];
unsigned char   TerrainMappingLayer2[TERRAIN_SIZE * TERRAIN_SIZE];
float           TerrainMappingAlpha[TERRAIN_SIZE * TERRAIN_SIZE];
float           TerrainGrassTexture[TERRAIN_SIZE * TERRAIN_SIZE];
float           TerrainGrassWind[TERRAIN_SIZE * TERRAIN_SIZE];
#ifdef ASG_ADD_MAP_KARUTAN
float			g_fTerrainGrassWind1[TERRAIN_SIZE * TERRAIN_SIZE];
#endif	// ASG_ADD_MAP_KARUTAN

WORD            TerrainWall[TERRAIN_SIZE * TERRAIN_SIZE];

float           SelectXF;
float           SelectYF;
float           WaterMove;
int             CurrentLayer;

float           g_fSpecialHeight = 1200.f;

#ifdef DYNAMIC_FRUSTRUM
FrustrumMap_t	g_FrustrumMap;
#endif //DYNAMIC_FRUSTRUM

const float g_fMinHeight = -500.f;
const float g_fMaxHeight = 1000.f;

extern  short   g_shCameraLevel;
extern  float CameraDistanceTarget;
extern  float CameraDistance;

static  float   g_fFrustumRange = -40.f;

inline int TERRAIN_INDEX(int x, int y)
{
    return (y)*TERRAIN_SIZE + (x);
}

inline int TERRAIN_INDEX_REPEAT(int x, int y)
{
    return ((y & TERRAIN_SIZE_MASK) * TERRAIN_SIZE) + (x & TERRAIN_SIZE_MASK);
}

inline BYTE TERRAIN_ATTRIBUTE(float x, float y)
{
    const int xf = static_cast<int>(x / TERRAIN_SCALE + 0.5f);
    const int yf = static_cast<int>(y / TERRAIN_SCALE + 0.5f);
    return TerrainWall[yf * TERRAIN_SIZE + xf];
}

void InitTerrainMappingLayer()
{
    for (int i = 0; i < TERRAIN_SIZE * TERRAIN_SIZE; ++i)
    {
        TerrainMappingLayer1[i] = 0;
        TerrainMappingLayer2[i] = 255;
        TerrainMappingAlpha[i] = 0.0f;
        TerrainGrassTexture[i] = static_cast<float>(rand() % 4) / 4.0f;
#ifdef ASG_ADD_MAP_KARUTAN
        g_fTerrainGrassWind1[i] = 0.0f;
#endif
    }
}

void ExitProgram()
{
    MessageBoxW(g_hWnd, GlobalText[11], NULL, MB_OK);
    PostQuitMessage(0);
}


int OpenTerrainAttribute(wchar_t* FileName)
{
    FILE* fp = _wfopen(FileName, L"rb");
    if (fp == NULL)
    {
        wchar_t Text[256];
        swprintf(Text, L"%s file not found.", FileName);
        g_ErrorReport.Write(Text);
        g_ErrorReport.Write(L"\r\n");
        MessageBox(g_hWnd, Text, NULL, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return (-1);
    }
    // Read file data
    fseek(fp, 0, SEEK_END);
    long file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    auto* file_data = new unsigned char[file_size];
    fread(file_data, file_size, 1, fp);

    // Decrypt file data
    int iSize = MapFileDecrypt(NULL, file_data, file_size);
    auto* decrypted_data = new unsigned char[iSize];
    MapFileDecrypt(decrypted_data, file_data, file_size);
    delete[] file_data;

    // Check file size
    bool extAtt = false;
    if (iSize != (TERRAIN_SIZE * TERRAIN_SIZE + 4) && iSize != (TERRAIN_SIZE * TERRAIN_SIZE * sizeof(WORD) + 4))
    {
        delete[] decrypted_data;
        return (-1);
    }
    if (iSize == (TERRAIN_SIZE * TERRAIN_SIZE * sizeof(WORD) + 4))
    {
        extAtt = true;
    }

    // Extract file header
    BuxConvert(decrypted_data, iSize);
    BYTE Version = decrypted_data[0];
    int iMap = decrypted_data[1];
    BYTE Width = decrypted_data[2];
    BYTE Height = decrypted_data[3];

    // Extract terrain attribute data
    if (!extAtt)
    {
        unsigned char TWall[TERRAIN_SIZE * TERRAIN_SIZE];
        memcpy(TWall, &decrypted_data[4], TERRAIN_SIZE * TERRAIN_SIZE);

        for (int i = 0; i < TERRAIN_SIZE * TERRAIN_SIZE; ++i)
        {
            TerrainWall[i] = TWall[i];
        }
    }
    else
    {
        memcpy(TerrainWall, &decrypted_data[4], TERRAIN_SIZE * TERRAIN_SIZE * sizeof(WORD));
    }

    delete[] decrypted_data;

    // Check file header
    bool Error = false;
    if (Version != 0 || Width != 255 || Height != 255)
    {
        Error = true;
    }

    // Check active world
    switch (gMapManager.WorldActive)
    {
    case WD_0LORENCIA:
        if (TerrainWall[123 * TERRAIN_SIZE + 135] != 5) Error = true;
        break;
    case WD_1DUNGEON:
        if (TerrainWall[120 * TERRAIN_SIZE + 227] != 4) Error = true;
        break;
    case WD_2DEVIAS:
        if (TerrainWall[55 * TERRAIN_SIZE + 208] != 5) Error = true;
        break;
    case WD_3NORIA:
        if (TerrainWall[119 * TERRAIN_SIZE + 186] != 5) Error = true;
        break;
    case WD_4LOSTTOWER:
        if (TerrainWall[75 * TERRAIN_SIZE + 193] != 5) Error = true;
        break;
    }

    for (int i = 0; i < TERRAIN_SIZE * TERRAIN_SIZE; i++)
    {
        TerrainWall[i] = TerrainWall[i] & 0xFF;
        if (TerrainWall[i] >= 128)
            Error = true;
    }

    if (Error)
    {
        ExitProgram();
        return (-1);
    }

    fclose(fp);
    return iMap;
}

bool SaveTerrainAttribute(wchar_t* FileName, int iMap)
{
    FILE* fp = _wfopen(FileName, L"wb");
    if (fp == NULL) {
        wchar_t Text[256];
        swprintf_s(Text, sizeof(Text), L"%s file not found.", FileName);
        g_ErrorReport.Write(Text);
        g_ErrorReport.Write(L"\r\n");
        MessageBox(g_hWnd, Text, NULL, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return false;
    }
    const BYTE Version = 0;
    const BYTE Width = 255;
    const BYTE Height = 255;

    fwrite(&Version, sizeof(Version), 1, fp);
    fwrite(&iMap, sizeof(iMap), 1, fp);
    fwrite(&Width, sizeof(Width), 1, fp);
    fwrite(&Height, sizeof(Height), 1, fp);

    // Add Frustum Culling here

    fwrite(TerrainWall, TERRAIN_SIZE * TERRAIN_SIZE * sizeof(WORD), 1, fp);

    fclose(fp);
    return true;
}

void AddTerrainAttribute(int x, int y, BYTE att)
{
    int     iIndex = (x + (y * TERRAIN_SIZE));
    TerrainWall[iIndex] |= att;
}

void SubTerrainAttribute(int x, int y, BYTE att)
{
    int     iIndex = (x + (y * TERRAIN_SIZE));

    TerrainWall[iIndex] ^= (TerrainWall[iIndex] & att);
}

void AddTerrainAttributeRange(int x, int y, int dx, int dy, BYTE att, BYTE Add)
{
    for (int j = 0; j < dy; ++j)
    {
        for (int i = 0; i < dx; ++i)
        {
            if (Add)
            {
                AddTerrainAttribute(x + i, y + j, att);
            }
            else
            {
                SubTerrainAttribute(x + i, y + j, att);
            }
        }
    }
}

void SetTerrainWaterState(std::list<int>& terrainIndex, int state)
{
    if (state == 0)
    {
        terrainIndex.clear();
        for (int i = 0; i < TERRAIN_SIZE * TERRAIN_SIZE; ++i)
        {
            if ((TerrainWall[i] & TW_WATER) == TW_WATER)
            {
                TerrainWall[i] = 0;
                terrainIndex.push_back(i);
            }
        }
    }
    else
    {
        for (std::list<int>::iterator iter = terrainIndex.begin(); iter != terrainIndex.end(); ++iter)
        {
            int index = *iter;
            TerrainWall[index] = TW_WATER;
        }
    }
}

int OpenTerrainMapping(wchar_t* FileName) {
    InitTerrainMappingLayer();
    FILE* fp = _wfopen(FileName, L"rb");
    if (fp == NULL) {
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    int EncBytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    auto* EncData = new unsigned char[EncBytes];
    fread(EncData, 1, EncBytes, fp);
    fclose(fp);

    int DataBytes = MapFileDecrypt(NULL, EncData, EncBytes);
    auto* Data = new unsigned char[DataBytes];
    MapFileDecrypt(Data, EncData, EncBytes);
    delete[] EncData;

    int DataPtr = 0;
    DataPtr += 1;

    int iMapNumber = static_cast<int>(*reinterpret_cast<BYTE*>(Data + DataPtr));
    DataPtr += 1;

    memcpy(TerrainMappingLayer1, Data + DataPtr, 256 * 256);
    DataPtr += 256 * 256;

    memcpy(TerrainMappingLayer2, Data + DataPtr, 256 * 256);
    DataPtr += 256 * 256;

    for (int i = 0; i < TERRAIN_SIZE * TERRAIN_SIZE; i++) {
        BYTE Alpha = *(Data + DataPtr);
        DataPtr += 1;
        TerrainMappingAlpha[i] = static_cast<float>(Alpha) / 255.f;
    }

    delete[] Data;

    TerrainGrassEnable = true;

    if (gMapManager.InChaosCastle() || gMapManager.InBattleCastle()) {
        TerrainGrassEnable = false;
    }

    return iMapNumber;
}

bool SaveTerrainMapping(wchar_t* FileName, int iMapNumber)
{
    FILE* fp = _wfopen(FileName, L"wb");
    BYTE Version = 0;
    fwrite(&Version, 1, 1, fp);
    fwrite(&iMapNumber, 1, 1, fp);
    fwrite(TerrainMappingLayer1, TERRAIN_SIZE * TERRAIN_SIZE, 1, fp);
    fwrite(TerrainMappingLayer2, TERRAIN_SIZE * TERRAIN_SIZE, 1, fp);
    for (int i = 0; i < TERRAIN_SIZE * TERRAIN_SIZE; i++)
    {
        auto Alpha = (unsigned char)(TerrainMappingAlpha[i] * 255.f);
        fwrite(&Alpha, 1, 1, fp);
    }
    /*
#ifndef BATTLE_CASTLE
    for(i=0;i<MAX_GROUNDS;i++)
    {
        GROUND *o = &Grounds[i];
        if(o->Live)
        {
            fwrite(&o->Type ,2,1,fp);
            fwrite(&o->x    ,1,1,fp);
            fwrite(&o->y    ,1,1,fp);
            fwrite(&o->Angle,1,1,fp);
        }
    }
#endif// BATTLE_CASTLE
*/
    fclose(fp);

    {
        fp = _wfopen(FileName, L"rb");
        if (fp == NULL)
        {
            return (false);
        }
        fseek(fp, 0, SEEK_END);
        int EncBytes = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        auto* EncData = new unsigned char[EncBytes];
        fread(EncData, 1, EncBytes, fp);
        fclose(fp);

        int DataBytes = MapFileEncrypt(NULL, EncData, EncBytes);
        auto* Data = new unsigned char[DataBytes];
        MapFileEncrypt(Data, EncData, EncBytes);
        delete[] EncData;

        fp = _wfopen(FileName, L"wb");
        fwrite(Data, DataBytes, 1, fp);
        fclose(fp);
        delete[] Data;
    }
    return true;
}

/*
#ifndef BATTLE_CASTLE
    void CreateGround(int Type,int x,int y,float Angle)
    {
        for(int i=0;i<MAX_GROUNDS;i++)
        {
            GROUND *o = &Grounds[i];
            if(!o->Live)
            {
                o->Live  = true;
                o->Type  = Type;
                o->x     = x;
                o->y     = y;
                o->Angle = (unsigned char)(Angle/360.f*255.f);
                return;
            }
        }
    }

    void DeleteGround(int x,int y)
    {
        for(int i=0;i<MAX_GROUNDS;i++)
        {
            GROUND *o = &Grounds[i];
            if(o->Live)
            {
                if(o->x==x && o->y==y) o->Live = false;
            }
        }
    }

    void RenderGrounds()
    {
        for(int i=0;i<MAX_GROUNDS;i++)
        {
            GROUND *o = &Grounds[i];
            if(o->Live)
            {
                float Angle = (float)o->Angle/255.f*360.f;
                RenderTerrainBitmap(BITMAP_CURSOR+6,o->x,o->y,Angle);
            }
        }
    }
#endif// BATTLE_CASTLE
*/
void CreateTerrainNormal()
{
    for (int y = 0; y < TERRAIN_SIZE; y++)
    {
        for (int x = 0; x < TERRAIN_SIZE; x++)
        {
            int Index = TERRAIN_INDEX(x, y);
            vec3_t v1, v2, v3, v4;
            Vector((x * TERRAIN_SCALE), (y * TERRAIN_SCALE), BackTerrainHeight[TERRAIN_INDEX_REPEAT(x, y)], v4);
            Vector(((x + 1) * TERRAIN_SCALE), (y * TERRAIN_SCALE), BackTerrainHeight[TERRAIN_INDEX_REPEAT((x + 1), y)], v1);
            Vector(((x + 1) * TERRAIN_SCALE), ((y + 1) * TERRAIN_SCALE), BackTerrainHeight[TERRAIN_INDEX_REPEAT((x + 1), (y + 1))], v2);
            Vector((x * TERRAIN_SCALE), ((y + 1) * TERRAIN_SCALE), BackTerrainHeight[TERRAIN_INDEX_REPEAT(x, (y + 1))], v3);
            vec3_t face_normal;
            FaceNormalize(v1, v2, v3, face_normal);
            VectorAdd(TerrainNormal[Index], face_normal, TerrainNormal[Index]);
            FaceNormalize(v3, v4, v1, face_normal);
            VectorAdd(TerrainNormal[Index], face_normal, TerrainNormal[Index]);
        }
    }
}

void CreateTerrainNormal_Part(int xi, int yi)
{
    // Clamp xi and yi to ensure they are within valid range
    xi = (xi > TERRAIN_SIZE - 4) ? TERRAIN_SIZE - 4 : ((xi < 4) ? 4 : xi);
    yi = (yi > TERRAIN_SIZE - 4) ? TERRAIN_SIZE - 4 : ((yi < 4) ? 4 : yi);
    for (int y = yi - 4; y < yi + 4; y++)
    {
        for (int x = xi - 4; x < xi + 4; x++)
        {
            int Index = TERRAIN_INDEX(x, y);
            vec3_t v1, v2, v3, v4;
            Vector((x * TERRAIN_SCALE), (y * TERRAIN_SCALE), BackTerrainHeight[TERRAIN_INDEX_REPEAT(x, y)], v4);
            Vector(((x + 1) * TERRAIN_SCALE), (y * TERRAIN_SCALE), BackTerrainHeight[TERRAIN_INDEX_REPEAT((x + 1), y)], v1);
            Vector(((x + 1) * TERRAIN_SCALE), ((y + 1) * TERRAIN_SCALE), BackTerrainHeight[TERRAIN_INDEX_REPEAT((x + 1), (y + 1))], v2);
            Vector((x * TERRAIN_SCALE), ((y + 1) * TERRAIN_SCALE), BackTerrainHeight[TERRAIN_INDEX_REPEAT(x, (y + 1))], v3);
            vec3_t face_normal;
            FaceNormalize(v1, v2, v3, face_normal);
            VectorAdd(TerrainNormal[Index], face_normal, TerrainNormal[Index]);
            FaceNormalize(v3, v4, v1, face_normal);
            VectorAdd(TerrainNormal[Index], face_normal, TerrainNormal[Index]);
        }
    }
}

void CreateTerrainLight()
{
    vec3_t Light;
    if (gMapManager.InBattleCastle())
    {
        Vector(0.5f, -1.f, 1.f, Light);
    }
    else
    {
        Vector(0.5f, -0.5f, 0.5f, Light);
    }
    for (int y = 0; y < TERRAIN_SIZE; y++)
    {
        for (int x = 0; x < TERRAIN_SIZE; x++)
        {
            int Index = TERRAIN_INDEX(x, y);
            float Luminosity = DotProduct(TerrainNormal[Index], Light) + 0.5f;
            if (Luminosity < 0.f)
                Luminosity = 0.f;
            else if (Luminosity > 1.f)
                Luminosity = 1.f;
            for (int i = 0; i < 3; i++)
            {
                BackTerrainLight[Index][i] = TerrainLight[Index][i] * Luminosity;
            }
        }
    }
}

void CreateTerrainLight_Part(int xi, int yi)
{
    if (xi > TERRAIN_SIZE - 4) xi = TERRAIN_SIZE - 4;
    else if (xi < 4)         xi = 4;

    if (yi > TERRAIN_SIZE - 4) yi = TERRAIN_SIZE - 4;
    else if (yi < 4)         yi = 4;

    vec3_t Light;
    Vector(0.5f, -0.5f, 0.5f, Light);
    for (int y = yi - 4; y < yi + 4; y++)
    {
        for (int x = xi - 4; x < xi + 4; x++)
        {
            int Index = TERRAIN_INDEX(x, y);
            float Luminosity = DotProduct(TerrainNormal[Index], Light) + 0.5f;
            if (Luminosity < 0.f) Luminosity = 0.f;
            else if (Luminosity > 1.f) Luminosity = 1.f;
            for (int i = 0; i < 3; i++)
                BackTerrainLight[Index][i] = TerrainLight[Index][i] * Luminosity;
        }
    }
}

void OpenTerrainLight(wchar_t* FileName)
{
    OpenJpegBuffer(FileName, &TerrainLight[0][0]);
    // Apply corrections to the loaded terrain light
    for (int i = 0; i < TERRAIN_SIZE * TERRAIN_SIZE; i++)
    {
        TerrainLight[i][2] -= 0.f; // < - Add/Color
        TerrainLight[i][1] -= 0.f;

        // Clamp channels to [0, 1]
        for (int j = 0; j < 3; j++)
        {
            if (TerrainLight[i][j] < 0.f)
                TerrainLight[i][j] = 0.f;
            else if (TerrainLight[i][j] > 1.f)
                TerrainLight[i][j] = 1.f;
        }
    }

    CreateTerrainNormal();
    CreateTerrainLight();

    /*
    #ifdef BATTLE_CASTLE
            if (battleCastle::InBattleCastle())
            g_fFrustumRange = -80.f;
        else
        #endif
            g_fFrustumRange = -40.f;
    */
}

void SaveTerrainLight(wchar_t* FileName)
{
    auto* Buffer = new unsigned char[TERRAIN_SIZE * TERRAIN_SIZE * 3];
    for (int i = 0; i < TERRAIN_SIZE * TERRAIN_SIZE; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            float Light = TerrainLight[i][j] * 255.f;
            Light = (Light < 0.f) ? 0.f : (Light > 255.f) ? 255.f : Light;
            Buffer[i * 3 + j] = static_cast<unsigned char>(Light);
        }
    }
    WriteJpeg(FileName, TERRAIN_SIZE, TERRAIN_SIZE, Buffer, 100);
    delete[] Buffer;
}

void CreateTerrain(wchar_t* FileName, bool bNew)
{
    ActiveTerrain = true;
    if (bNew)
    {
        OpenTerrainHeightNew(FileName);
    }
    else
    {
        OpenTerrainHeight(FileName);
    }

    CreateSun();
}

unsigned char BMPHeader[1080];

bool IsTerrainHeightExtMap(int iWorld)
{
    return (iWorld == WD_42CHANGEUP3RD_2ND || gMapManager.IsPKField() || iWorld == WD_66DOPPLEGANGER2);
}

bool OpenTerrainHeight(wchar_t* filename)
{
    const int Index = 1080;
    const int Size = 256 * 256 + Index;
    wchar_t FileName[256];

    wchar_t NewFileName[256];

    for (int i = 0; i < (int)wcslen(filename); i++)
    {
        NewFileName[i] = filename[i];
        NewFileName[i + 1] = NULL;
        if (filename[i] == '.')
            break;
    }

    wcscpy_s(FileName, L"Data\\");
    wcscat_s(FileName, NewFileName);
    wcscat_s(FileName, L"OZB");

    FILE* fp;
    errno_t err = _wfopen_s(&fp, FileName, L"rb");
    if (err != 0 || fp == NULL)
    {
        wchar_t Text[256];
        swprintf_s(Text, L"%s file not found.", FileName);
        g_ErrorReport.Write(Text);
        g_ErrorReport.Write(L"\r\n");
        MessageBox(g_hWnd, Text, NULL, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return false;
    }

    auto* Buffer = new unsigned char[Size];

    fseek(fp, 4, SEEK_SET);
    fread_s(Buffer, Size, 1, Size, fp);
    fclose(fp);

    memcpy(BMPHeader, Buffer, Index);

    for (int i = 0; i < 256; i++)
    {
        const unsigned char* src = &Buffer[Index + i * 256];
        float* dst = &BackTerrainHeight[i * 256];
        const float factor = (gMapManager.WorldActive == WD_55LOGINSCENE) ? 3.0f : 1.5f;
        for (int j = 0; j < 256; j++)
        {
            *dst = (float)(*src) * factor;
            src++;
            dst++;
        }
    }

    delete[] Buffer;
    return true;
}

void SaveTerrainHeight(wchar_t* name)
{
    auto* Buffer = new unsigned char[256 * 256];
    for (int i = 0; i < 256; i++)
    {
        float* src = &BackTerrainHeight[i * 256];
        unsigned char* dst = &Buffer[(255 - i) * 256];
        const float factor = (gMapManager.WorldActive == WD_55LOGINSCENE) ? 3.0f : 1.5f;
        for (int j = 0; j < 256; j++)
        {
            float Height = *src / factor;
            if (Height < 0.f) Height = 0.f;
            else if (Height > 255.f) Height = 255.f;
            *dst = (unsigned char)(Height);
            src++;
            dst++;
        }
    }
    FILE* fp = _wfopen(name, L"wb");
    fwrite(BMPHeader, 1080, 1, fp);

    for (int i = 0; i < 256; i++) fwrite(Buffer + (255 - i) * 256, 256, 1, fp);

    SAFE_DELETE_ARRAY(Buffer);
    fclose(fp);
}

bool OpenTerrainHeightNew(const wchar_t* strFilename)
{
    wchar_t FileName[256];
    wchar_t NewFileName[256];

    for (int i = 0; i < (int)wcslen(strFilename); ++i)
    {
        NewFileName[i] = strFilename[i];
        NewFileName[i + 1] = NULL;

        if (strFilename[i] == '.')
            break;
    }

    wcscpy(FileName, L"Data\\");
    wcscat(FileName, NewFileName);
    wcscat(FileName, L"OZB");

    FILE* fp = _wfopen(FileName, L"rb");
    if (!fp)
    {
        wchar_t Text[256];
        swprintf(Text, L"%s file not found.", FileName);
        g_ErrorReport.Write(Text);
        g_ErrorReport.Write(L"\r\n");
        MessageBox(g_hWnd, Text, NULL, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return false;
    }

    fseek(fp, 0, SEEK_END);
    int iBytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    BYTE* pbyData = new BYTE[iBytes];
    fread(pbyData, 1, iBytes, fp);
    fclose(fp);

    DWORD dwCurPos = 0;
    dwCurPos += 4;

    BITMAPINFOHEADER bmiHeader;
    BITMAPFILEHEADER header;

    memcpy(&header, &pbyData[dwCurPos], sizeof(BITMAPFILEHEADER));
    dwCurPos += sizeof(BITMAPFILEHEADER);

    memcpy(&bmiHeader, &pbyData[dwCurPos], sizeof(BITMAPINFOHEADER));
    dwCurPos += sizeof(BITMAPINFOHEADER);

    for (int i = 0; i < TERRAIN_SIZE * TERRAIN_SIZE; ++i)
    {
        BYTE* pbysrc = &pbyData[dwCurPos + i * 3];

        DWORD dwHeight = 0;
        BYTE* pbyHeight = (BYTE*)&dwHeight;

        pbyHeight[0] = pbysrc[2];
        pbyHeight[1] = pbysrc[1];
        pbyHeight[2] = pbysrc[0];

        BackTerrainHeight[i] = (float)dwHeight;
        BackTerrainHeight[i] += g_fMinHeight;
    }

    delete[] pbyData;
    return true;
}

extern EGameScene SceneFlag;

float RequestTerrainHeight(float xf, float yf) {
    if (SceneFlag == SERVER_LIST_SCENE || SceneFlag == WEBZEN_SCENE || SceneFlag == LOADING_SCENE)
        return 0.0f;
    if (xf < 0.0f || yf < 0.0f)
        return 0.0f;

    xf /= TERRAIN_SCALE;
    yf /= TERRAIN_SCALE;

    unsigned int Index = TERRAIN_INDEX(xf, yf);

    if (Index >= TERRAIN_SIZE * TERRAIN_SIZE)
        return g_fSpecialHeight;

    if ((TerrainWall[Index] & TW_HEIGHT) == TW_HEIGHT)
        return g_fSpecialHeight;

    int xi = static_cast<int>(xf);
    int yi = static_cast<int>(yf);
    float xd = xf - static_cast<float>(xi);
    float yd = yf - static_cast<float>(yi);
    unsigned int Index1 = TERRAIN_INDEX_REPEAT(xi, yi);
    unsigned int Index2 = TERRAIN_INDEX_REPEAT(xi, yi + 1);
    unsigned int Index3 = TERRAIN_INDEX_REPEAT(xi + 1, yi);
    unsigned int Index4 = TERRAIN_INDEX_REPEAT(xi + 1, yi + 1);

    if (Index1 >= TERRAIN_SIZE * TERRAIN_SIZE || Index2 >= TERRAIN_SIZE * TERRAIN_SIZE ||
        Index3 >= TERRAIN_SIZE * TERRAIN_SIZE || Index4 >= TERRAIN_SIZE * TERRAIN_SIZE)
        return g_fSpecialHeight;

    float left = BackTerrainHeight[Index1] + (BackTerrainHeight[Index2] - BackTerrainHeight[Index1]) * yd;
    float right = BackTerrainHeight[Index3] + (BackTerrainHeight[Index4] - BackTerrainHeight[Index3]) * yd;
    return left + (right - left) * xd;
}

void RequestTerrainNormal(float xf, float yf, vec3_t Normal)
{
    xf = xf / TERRAIN_SCALE;
    yf = yf / TERRAIN_SCALE;
    int xi = (int)xf;
    int yi = (int)yf;
    VectorCopy(TerrainNormal[TERRAIN_INDEX_REPEAT(xi, yi)], Normal);
}

void AddTerrainHeight(float xf, float yf, float Height, int Range, float* Buffer)
{
    auto rf = (float)Range;

    xf = xf / TERRAIN_SCALE;
    yf = yf / TERRAIN_SCALE;
    int   xi = (int)xf;
    int   yi = (int)yf;
    int   syi = yi - Range;
    int   eyi = yi + Range;
    auto syf = (float)(syi);
    for (; syi <= eyi; syi++, syf += 1.f)
    {
        int   sxi = xi - Range;
        int   exi = xi + Range;
        auto sxf = (float)(sxi);
        for (; sxi <= exi; sxi++, sxf += 1.f)
        {
            float xd = xf - sxf;
            float yd = yf - syf;
            float lf = (rf - sqrtf(xd * xd + yd * yd)) / rf;
            if (lf > 0.f)
            {
                Buffer[TERRAIN_INDEX_REPEAT(sxi, syi)] += Height * lf;
            }
        }
    }
}

void SetTerrainLight(float xf, float yf, vec3_t Light, int Range, vec3_t* Buffer)
{
    auto rf = (float)Range;

    xf = (xf / TERRAIN_SCALE);
    yf = (yf / TERRAIN_SCALE);
    int   xi = (int)xf;
    int   yi = (int)yf;
    int   syi = yi - Range;
    int   eyi = yi + Range;
    auto syf = (float)(syi);
    for (; syi <= eyi; syi++, syf += 1.f)
    {
        int   sxi = xi - Range;
        int   exi = xi + Range;
        auto sxf = (float)(sxi);
        for (; sxi <= exi; sxi++, sxf += 1.f)
        {
            float xd = xf - sxf;
            float yd = yf - syf;
            float lf = (rf - sqrtf(xd * xd + yd * yd)) / rf;
            if (lf > 0.f)
            {
                float* b = &Buffer[TERRAIN_INDEX_REPEAT(sxi, syi)][0];
                for (int i = 0; i < 3; i++)
                {
                    b[i] += Light[i] * lf;
                }
            }
        }
    }
}

void AddTerrainLight(float xf, float yf, vec3_t Light, int Range, vec3_t* Buffer)
{
    auto rf = (float)Range;

    xf = (xf / TERRAIN_SCALE);
    yf = (yf / TERRAIN_SCALE);
    int   xi = (int)xf;
    int   yi = (int)yf;
    int   syi = yi - Range;
    int   eyi = yi + Range;
    auto syf = (float)(syi);
    for (; syi <= eyi; syi++, syf += 1.f)
    {
        int   sxi = xi - Range;
        int   exi = xi + Range;
        auto sxf = (float)(sxi);
        for (; sxi <= exi; sxi++, sxf += 1.f)
        {
            float xd = xf - sxf;
            float yd = yf - syf;
            float lf = (rf - sqrtf(xd * xd + yd * yd)) / rf;
            if (lf > 0.f)
            {
                float* b = &Buffer[TERRAIN_INDEX_REPEAT(sxi, syi)][0];
                for (int i = 0; i < 3; i++)
                {
                    b[i] += Light[i] * lf;
                    if (b[i] < 0.f) b[i] = 0.f;
                }
            }
        }
    }
}

void AddTerrainLightClip(float xf, float yf, vec3_t Light, int Range, vec3_t* Buffer)
{
    auto rf = (float)Range;

    xf = (xf / TERRAIN_SCALE);
    yf = (yf / TERRAIN_SCALE);
    int   xi = (int)xf;
    int   yi = (int)yf;
    int   syi = yi - Range;
    int   eyi = yi + Range;
    auto syf = (float)(syi);
    for (; syi <= eyi; syi++, syf += 1.f)
    {
        int   sxi = xi - Range;
        int   exi = xi + Range;
        auto sxf = (float)(sxi);
        for (; sxi <= exi; sxi++, sxf += 1.f)
        {
            float xd = xf - sxf;
            float yd = yf - syf;
            float lf = (rf - sqrtf(xd * xd + yd * yd)) / rf;
            if (lf > 0.f)
            {
                float* b = &Buffer[TERRAIN_INDEX_REPEAT(sxi, syi)][0];
                for (int i = 0; i < 3; i++)
                {
                    b[i] += Light[i] * lf;
                    if (b[i] < 0.f) b[i] = 0.f;
                    else if (b[i] > 1.f) b[i] = 1.f;
                }
            }
        }
    }
}

void RequestTerrainLight(float xf, float yf, vec3_t Light)
{
    if (SceneFlag == SERVER_LIST_SCENE
        || SceneFlag == WEBZEN_SCENE
        || SceneFlag == LOADING_SCENE
        || ActiveTerrain == false)
    {
        Vector(0.f, 0.f, 0.f, Light);
        return;
    }

    xf = xf / TERRAIN_SCALE;
    yf = yf / TERRAIN_SCALE;
    int xi = (int)xf;
    int yi = (int)yf;
    if (xi<0 || yi<0 || xi>TERRAIN_SIZE_MASK - 1 || yi>TERRAIN_SIZE_MASK - 1)
    {
        Vector(0.f, 0.f, 0.f, Light);
        return;
    }
    int Index1 = ((xi)+(yi)*TERRAIN_SIZE);
    int Index2 = ((xi + 1) + (yi)*TERRAIN_SIZE);
    int Index3 = ((xi + 1) + (yi + 1) * TERRAIN_SIZE);
    int Index4 = ((xi)+(yi + 1) * TERRAIN_SIZE);
    float xd = xf - (float)xi;
    float yd = yf - (float)yi;
    for (int i = 0; i < 3; i++)
    {
        float left = PrimaryTerrainLight[Index1][i] + (PrimaryTerrainLight[Index4][i] - PrimaryTerrainLight[Index1][i]) * yd;
        float right = PrimaryTerrainLight[Index2][i] + (PrimaryTerrainLight[Index3][i] - PrimaryTerrainLight[Index2][i]) * yd;
        Light[i] = (left + (right - left) * xd);
    }
}

void CreateLodBuffer()
{
    for (int y = 0; y < TERRAIN_SIZE; y += 4)
    {
        for (int x = 0; x < TERRAIN_SIZE; x += 4)
        {
            vec3_t NormalMin, NormalMax;
            NormalMin[0] = 1.f;
            NormalMin[1] = 1.f;
            NormalMin[2] = 1.f;
            NormalMax[0] = -1.f;
            NormalMax[1] = -1.f;
            NormalMax[2] = -1.f;
            for (int i = 0; i < 4; i++)
            {
                for (int j = 0; j < 4; j++)
                {
                    vec3_t v1, v2, v3, v4;
                    Vector((x + j) * TERRAIN_SCALE, (y + i) * TERRAIN_SCALE, BackTerrainHeight[TERRAIN_INDEX_REPEAT(x + j, y + i)], v1);
                    Vector((x + j + 1) * TERRAIN_SCALE, (y + i) * TERRAIN_SCALE, BackTerrainHeight[TERRAIN_INDEX_REPEAT(x + j + 1, y + i)], v2);
                    Vector((x + j + 1) * TERRAIN_SCALE, (y + i + 1) * TERRAIN_SCALE, BackTerrainHeight[TERRAIN_INDEX_REPEAT(x + j + 1, y + i + 1)], v3);
                    Vector((x + j) * TERRAIN_SCALE, (y + i + 1) * TERRAIN_SCALE, BackTerrainHeight[TERRAIN_INDEX_REPEAT(x + j, y + i + 1)], v4);

                    int Index = TERRAIN_INDEX(x + j, y + i);
                    FaceNormalize(v1, v2, v3, TerrainNormal[Index]);

                    vec3_t* Normal = &TerrainNormal[Index];
                    if (NormalMin[0] > (*Normal)[0]) NormalMin[0] = (*Normal)[0];
                    if (NormalMax[0] < (*Normal)[0]) NormalMax[0] = (*Normal)[0];
                    if (NormalMin[1] > (*Normal)[1]) NormalMin[1] = (*Normal)[1];
                    if (NormalMax[1] < (*Normal)[1]) NormalMax[1] = (*Normal)[1];
                    if (NormalMin[2] > (*Normal)[2]) NormalMin[2] = (*Normal)[2];
                    if (NormalMax[2] < (*Normal)[2]) NormalMax[2] = (*Normal)[2];
                }
            }
            float Delta = maxf(maxf(absf(NormalMax[0] - NormalMin[0]), absf(NormalMax[1] - NormalMin[1])), absf(NormalMax[2] - NormalMin[2]));
            if (DetailLowEnable == true)
            {
                LodBuffer[y / 4 * 64 + x / 4] = 4;
            }
            else
            {
                if (Delta >= 1.f) LodBuffer[y / 4 * 64 + x / 4] = 1;
                else if (Delta >= 0.5f) LodBuffer[y / 4 * 64 + x / 4] = 2;
                else                   LodBuffer[y / 4 * 64 + x / 4] = 4;
                LodBuffer[y / 4 * 64 + x / 4] = 1;
            }
        }
    }
}

void InterpolationHeight(int lod, int x, int y, int xd, float* TerrainHeight)
{
    if (lod >= 4)
    {
        TerrainHeight[TERRAIN_INDEX_REPEAT(x + xd, y + 2)] = (
            TerrainHeight[TERRAIN_INDEX_REPEAT(x + xd, y)] +
            TerrainHeight[TERRAIN_INDEX_REPEAT(x + xd, y + 4)]) * 0.5f;
    }
    if (lod >= 2)
    {
        TerrainHeight[TERRAIN_INDEX_REPEAT(x + xd, y + 1)] = (
            TerrainHeight[TERRAIN_INDEX_REPEAT(x + xd, y)] +
            TerrainHeight[TERRAIN_INDEX_REPEAT(x + xd, y + 2)]) * 0.5f;
        TerrainHeight[TERRAIN_INDEX_REPEAT(x + xd, y + 3)] = (
            TerrainHeight[TERRAIN_INDEX_REPEAT(x + xd, y + 2)] +
            TerrainHeight[TERRAIN_INDEX_REPEAT(x + xd, y + 4)]) * 0.5f;
    }
}

void InterpolationWidth(int lod, int x, int y, int yd, float* TerrainHeight)
{
    if (lod >= 4)
    {
        TerrainHeight[TERRAIN_INDEX_REPEAT(x + 2, y + yd)] = (
            TerrainHeight[TERRAIN_INDEX_REPEAT(x, y + yd)] +
            TerrainHeight[TERRAIN_INDEX_REPEAT(x + 4, y + yd)]) * 0.5f;
    }
    if (lod >= 2)
    {
        TerrainHeight[TERRAIN_INDEX_REPEAT(x + 1, y + yd)] = (
            TerrainHeight[TERRAIN_INDEX_REPEAT(x, y + yd)] +
            TerrainHeight[TERRAIN_INDEX_REPEAT(x + 2, y + yd)]) * 0.5f;
        TerrainHeight[TERRAIN_INDEX_REPEAT(x + 3, y + yd)] = (
            TerrainHeight[TERRAIN_INDEX_REPEAT(x + 2, y + yd)] +
            TerrainHeight[TERRAIN_INDEX_REPEAT(x + 4, y + yd)]) * 0.5f;
    }
}

void InterpolationCross(int lod, int x, int y)
{
    BackTerrainHeight[TERRAIN_INDEX_REPEAT(x, y)] = (
        BackTerrainHeight[TERRAIN_INDEX_REPEAT(x - lod, y)] +
        BackTerrainHeight[TERRAIN_INDEX_REPEAT(x + lod, y)] +
        BackTerrainHeight[TERRAIN_INDEX_REPEAT(x, y - lod)] +
        BackTerrainHeight[TERRAIN_INDEX_REPEAT(x, y + lod)]) * 0.25f;
}

void PrefixTerrainHeightEdge(int x, int y, int lod, float* TerrainHeight)
{
    if (lod >= LodBuffer[((y) & 63) * 64 + ((x - 1) & 63)]) InterpolationHeight(lod, x * 4, y * 4, 0, TerrainHeight);
    if (lod >= LodBuffer[((y) & 63) * 64 + ((x + 1) & 63)]) InterpolationHeight(lod, x * 4, y * 4, 4, TerrainHeight);
    if (lod >= LodBuffer[((y - 1) & 63) * 64 + ((x) & 63)]) InterpolationWidth(lod, x * 4, y * 4, 0, TerrainHeight);
    if (lod >= LodBuffer[((y + 1) & 63) * 64 + ((x) & 63)]) InterpolationWidth(lod, x * 4, y * 4, 4, TerrainHeight);
}

void PrefixTerrainHeight()
{
    for (int y = 0; y < 64; y++)
    {
        for (int x = 0; x < 64; x++)
        {
            int lod = LodBuffer[((y) & 63) * 64 + ((x) & 63)];
            PrefixTerrainHeightEdge(x, y, lod, BackTerrainHeight);
            if (lod >= 2)
            {
                if (lod >= 4)
                {
                    InterpolationHeight(lod, x * 4, y * 4, 2, BackTerrainHeight);
                    InterpolationWidth(lod, x * 4, y * 4, 2, BackTerrainHeight);
                }
                if (lod >= 2)
                {
                    InterpolationHeight(lod, x * 4, y * 4, 1, BackTerrainHeight);
                    InterpolationWidth(lod, x * 4, y * 4, 1, BackTerrainHeight);
                    InterpolationHeight(lod, x * 4, y * 4, 3, BackTerrainHeight);
                    InterpolationWidth(lod, x * 4, y * 4, 3, BackTerrainHeight);
                    /*InterpolationCross(1,x*4+1,y*4+1);
                    InterpolationCross(1,x*4+1,y*4+3);
                    InterpolationCross(1,x*4+3,y*4+1);
                    InterpolationCross(1,x*4+3,y*4+3);*/
                }
            }
        }
    }
}

bool  SelectFlag;

int    TerrainIndex1;
int    TerrainIndex2;
int    TerrainIndex3;
int    TerrainIndex4;
int    Index0;
int    Index1;
int    Index2;
int    Index3;
int    Index01;
int    Index12;
int    Index23;
int    Index30;
int    Index02;
vec3_t TerrainVertex[4];
vec3_t TerrainVertex01;
vec3_t TerrainVertex12;
vec3_t TerrainVertex23;
vec3_t TerrainVertex30;
vec3_t TerrainVertex02;
float  TerrainTextureCoord[4][2];
float  TerrainTextureCoord01[2];
float  TerrainTextureCoord12[2];
float  TerrainTextureCoord23[2];
float  TerrainTextureCoord30[2];
float  TerrainTextureCoord02[2];
float  TerrainMappingAlpha01;
float  TerrainMappingAlpha12;
float  TerrainMappingAlpha23;
float  TerrainMappingAlpha30;
float  TerrainMappingAlpha02;

inline void Interpolation(int mx, int my)
{
    Index01 = (my * 2) * 512 + (mx * 2 + 1);
    Index12 = (my * 2 + 1) * 512 + (mx * 2 + 2);
    Index23 = (my * 2 + 2) * 512 + (mx * 2 + 1);
    Index30 = (my * 2 + 1) * 512 + (mx * 2);
    Index02 = (my * 2 + 1) * 512 + (mx * 2 + 1);
    for (int i = 0; i < 3; i++)
    {
        TerrainVertex01[i] = (TerrainVertex[0][i] + TerrainVertex[1][i]) * 0.5f;
        TerrainVertex12[i] = (TerrainVertex[1][i] + TerrainVertex[2][i]) * 0.5f;
        TerrainVertex23[i] = (TerrainVertex[2][i] + TerrainVertex[3][i]) * 0.5f;
        TerrainVertex30[i] = (TerrainVertex[3][i] + TerrainVertex[0][i]) * 0.5f;
        TerrainVertex02[i] = (TerrainVertex[0][i] + TerrainVertex[2][i]) * 0.5f;
    }

    for (int i = 0; i < 2; i++)
    {
        TerrainTextureCoord01[i] = (TerrainTextureCoord[0][i] + TerrainTextureCoord[1][i]) * 0.5f;
        TerrainTextureCoord12[i] = (TerrainTextureCoord[1][i] + TerrainTextureCoord[2][i]) * 0.5f;
        TerrainTextureCoord23[i] = (TerrainTextureCoord[2][i] + TerrainTextureCoord[3][i]) * 0.5f;
        TerrainTextureCoord30[i] = (TerrainTextureCoord[3][i] + TerrainTextureCoord[0][i]) * 0.5f;
        TerrainTextureCoord02[i] = (TerrainTextureCoord[0][i] + TerrainTextureCoord[2][i]) * 0.5f;
    }
}

inline void Vertex0()
{
    glTexCoord2f(TerrainTextureCoord[0][0], TerrainTextureCoord[0][1]);
    glColor3fv(PrimaryTerrainLight[TerrainIndex1]);
    glVertex3fv(TerrainVertex[0]);
}

inline void Vertex1()
{
    glTexCoord2f(TerrainTextureCoord[1][0], TerrainTextureCoord[1][1]);
    glColor3fv(PrimaryTerrainLight[TerrainIndex2]);
    glVertex3fv(TerrainVertex[1]);
}

inline void Vertex2()
{
    glTexCoord2f(TerrainTextureCoord[2][0], TerrainTextureCoord[2][1]);
    glColor3fv(PrimaryTerrainLight[TerrainIndex3]);
    glVertex3fv(TerrainVertex[2]);
}

inline void Vertex3()
{
    glTexCoord2f(TerrainTextureCoord[3][0], TerrainTextureCoord[3][1]);
    glColor3fv(PrimaryTerrainLight[TerrainIndex4]);
    glVertex3fv(TerrainVertex[3]);
}

inline void Vertex01()
{
    glTexCoord2f(TerrainTextureCoord01[0], TerrainTextureCoord01[1]);
    glColor3fv(PrimaryTerrainLight[Index01]);
    glVertex3fv(TerrainVertex01);
}

inline void Vertex12()
{
    glTexCoord2f(TerrainTextureCoord12[0], TerrainTextureCoord12[1]);
    glColor3fv(PrimaryTerrainLight[Index12]);
    glVertex3fv(TerrainVertex12);
}

inline void Vertex23()
{
    glTexCoord2f(TerrainTextureCoord23[0], TerrainTextureCoord23[1]);
    glColor3fv(PrimaryTerrainLight[Index23]);
    glVertex3fv(TerrainVertex23);
}

inline void Vertex30()
{
    glTexCoord2f(TerrainTextureCoord30[0], TerrainTextureCoord30[1]);
    glColor3fv(PrimaryTerrainLight[Index30]);
    glVertex3fv(TerrainVertex30);
}

inline void Vertex02()
{
    glTexCoord2f(TerrainTextureCoord02[0], TerrainTextureCoord02[1]);
    glColor3fv(PrimaryTerrainLight[Index02]);
    glVertex3fv(TerrainVertex02);
}

inline void VertexAlpha0()
{
    glTexCoord2f(TerrainTextureCoord[0][0], TerrainTextureCoord[0][1]);
    float* Light = &PrimaryTerrainLight[TerrainIndex1][0];
    glColor4f(Light[0], Light[1], Light[2], TerrainMappingAlpha[TerrainIndex1]);
    glVertex3fv(TerrainVertex[0]);
}

inline void VertexAlpha1()
{
    glTexCoord2f(TerrainTextureCoord[1][0], TerrainTextureCoord[1][1]);
    float* Light = &PrimaryTerrainLight[TerrainIndex2][0];
    glColor4f(Light[0], Light[1], Light[2], TerrainMappingAlpha[TerrainIndex2]);
    glVertex3fv(TerrainVertex[1]);
}

inline void VertexAlpha2()
{
    glTexCoord2f(TerrainTextureCoord[2][0], TerrainTextureCoord[2][1]);
    float* Light = &PrimaryTerrainLight[TerrainIndex3][0];
    glColor4f(Light[0], Light[1], Light[2], TerrainMappingAlpha[TerrainIndex3]);
    glVertex3fv(TerrainVertex[2]);
}

inline void VertexAlpha3()
{
    glTexCoord2f(TerrainTextureCoord[3][0], TerrainTextureCoord[3][1]);
    float* Light = &PrimaryTerrainLight[TerrainIndex4][0];
    glColor4f(Light[0], Light[1], Light[2], TerrainMappingAlpha[TerrainIndex4]);
    glVertex3fv(TerrainVertex[3]);
}

inline void VertexAlpha01()
{
    glTexCoord2f(TerrainTextureCoord01[0], TerrainTextureCoord01[1]);
    float* Light = &PrimaryTerrainLight[Index01][0];
    glColor4f(Light[0], Light[1], Light[2], TerrainMappingAlpha01);
    glVertex3fv(TerrainVertex01);
}

inline void VertexAlpha12()
{
    glTexCoord2f(TerrainTextureCoord12[0], TerrainTextureCoord12[1]);
    float* Light = &PrimaryTerrainLight[Index12][0];
    glColor4f(Light[0], Light[1], Light[2], TerrainMappingAlpha12);
    glVertex3fv(TerrainVertex12);
}

inline void VertexAlpha23()
{
    glTexCoord2f(TerrainTextureCoord23[0], TerrainTextureCoord23[1]);
    float* Light = &PrimaryTerrainLight[Index23][0];
    glColor4f(Light[0], Light[1], Light[2], TerrainMappingAlpha23);
    glVertex3fv(TerrainVertex23);
}

inline void VertexAlpha30()
{
    glTexCoord2f(TerrainTextureCoord30[0], TerrainTextureCoord30[1]);
    float* Light = &PrimaryTerrainLight[Index30][0];
    glColor4f(Light[0], Light[1], Light[2], TerrainMappingAlpha30);
    glVertex3fv(TerrainVertex30);
}

inline void VertexAlpha02()
{
    glTexCoord2f(TerrainTextureCoord02[0], TerrainTextureCoord02[1]);
    float* Light = &PrimaryTerrainLight[Index02][0];
    glColor4f(Light[0], Light[1], Light[2], TerrainMappingAlpha02);
    glVertex3fv(TerrainVertex02);
}

inline void VertexBlend0()
{
    glTexCoord2f(TerrainTextureCoord[0][0], TerrainTextureCoord[0][1]);
    float Light = TerrainMappingAlpha[TerrainIndex1];
    glColor3f(Light, Light, Light);
    glVertex3fv(TerrainVertex[0]);
}

inline void VertexBlend1()
{
    glTexCoord2f(TerrainTextureCoord[1][0], TerrainTextureCoord[1][1]);
    float Light = TerrainMappingAlpha[TerrainIndex2];
    glColor3f(Light, Light, Light);
    glVertex3fv(TerrainVertex[1]);
}

inline void VertexBlend2()
{
    glTexCoord2f(TerrainTextureCoord[2][0], TerrainTextureCoord[2][1]);
    float Light = TerrainMappingAlpha[TerrainIndex3];
    glColor3f(Light, Light, Light);
    glVertex3fv(TerrainVertex[2]);
}

inline void VertexBlend3()
{
    glTexCoord2f(TerrainTextureCoord[3][0], TerrainTextureCoord[3][1]);
    float Light = TerrainMappingAlpha[TerrainIndex4];
    glColor3f(Light, Light, Light);
    glVertex3fv(TerrainVertex[3]);
}

void RenderFace(int Texture, int mx, int my)
{
    if (gMapManager.WorldActive == WD_39KANTURU_3RD)
    {
        if (Texture == 3)
            EnableAlphaTest();
        else if (Texture == 100)
            return;
        else
            DisableAlphaBlend();
    }
    else if (gMapManager.WorldActive >= WD_45CURSEDTEMPLE_LV1 && gMapManager.WorldActive <= WD_45CURSEDTEMPLE_LV6)
    {
        if (Texture == 4)
        {
            EnableAlphaTest();
        }
        else
        {
            DisableAlphaBlend();
        }
    }
    else if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR
        )
    {
        if (Texture == 2)
        {
            EnableAlphaTest();
        }
        else
        {
            DisableAlphaBlend();
        }
    }
    else if (gMapManager.WorldActive == WD_69EMPIREGUARDIAN1 || gMapManager.WorldActive == WD_70EMPIREGUARDIAN2 || gMapManager.WorldActive == WD_71EMPIREGUARDIAN3 || gMapManager.WorldActive == WD_72EMPIREGUARDIAN4
        || gMapManager.WorldActive == WD_73NEW_LOGIN_SCENE || gMapManager.WorldActive == WD_74NEW_CHARACTER_SCENE
        )
    {
        if (Texture == 10)
        {
            EnableAlphaTest();
        }
        else
        {
            DisableAlphaBlend();
        }
    }
#ifdef ASG_ADD_MAP_KARUTAN
    else if (IsKarutanMap())
    {
        if (Texture == 12)
            EnableAlphaTest();
        else
            DisableAlphaBlend();
    }
#endif	// ASG_ADD_MAP_KARUTAN
    else
        DisableAlphaBlend();
    BindTexture(BITMAP_MAPTILE + Texture);

    glBegin(GL_TRIANGLE_FAN);
    Vertex0();
    Vertex1();
    Vertex2();
    Vertex3();
    glEnd();
}

void RenderFace_After(int Texture, int mx, int my)
{
    if (Texture == 100)
        EnableAlphaTest();
    else if (Texture == 101)
        EnableAlphaBlend();
    else
        return;

    BindTexture(BITMAP_MAPTILE + Texture);

    glBegin(GL_TRIANGLE_FAN);
    Vertex0();
    Vertex1();
    Vertex2();
    Vertex3();
    glEnd();
}

void RenderFaceAlpha(int Texture, int mx, int my)
{
    EnableAlphaTest();
    BindTexture(BITMAP_MAPTILE + Texture);
    glBegin(GL_TRIANGLE_FAN);
    VertexAlpha0();
    VertexAlpha1();
    VertexAlpha2();
    VertexAlpha3();
    glEnd();
}

void RenderFaceBlend(int Texture, int mx, int my)
{
    EnableAlphaBlend();
    BindTexture(BITMAP_MAPTILE + Texture);
    glBegin(GL_TRIANGLE_FAN);
    VertexBlend0();
    VertexBlend1();
    VertexBlend2();
    VertexBlend3();
    glEnd();
}

void FaceTexture(int Texture, float xf, float yf, bool Water, bool Scale)
{
    vec3_t Light, Pos;
    Vector(0.30f, 0.40f, 0.20f, Light);
    BITMAP_t* b = &Bitmaps[BITMAP_MAPTILE + Texture];
    float Width, Height;
    if (Scale)
    {
        Width = 16.f / b->Width;
        Height = 16.f / b->Height;
    }
    else
    {
        Width = 64.f / b->Width;
        Height = 64.f / b->Height;
    }
    float suf = xf * Width;
    float svf = yf * Height;
    if (!Water)
    {
        TEXCOORD(TerrainTextureCoord[0], suf, svf);
        TEXCOORD(TerrainTextureCoord[1], suf + Width, svf);
        TEXCOORD(TerrainTextureCoord[2], suf + Width, svf + Height);
        TEXCOORD(TerrainTextureCoord[3], suf, svf + Height);
    }
    else
    {
        float Water1 = 0.f;
        float Water2 = 0.f;
        float Water3 = 0.f;
        float Water4 = 0.f;
        if (gMapManager.WorldActive == WD_34CRYWOLF_1ST && Texture == 5)
        {
            if (rand_fps_check(50))
            {
                float sx = xf * TERRAIN_SCALE + (float)((rand() % 100 + 1) * 1.0f);
                float sy = yf * TERRAIN_SCALE + (float)((rand() % 100 + 1) * 1.0f);
                Vector(sx, sy, Hero->Object.Position[2] + 10.f, Pos);
                CreateParticle(BITMAP_SPOT_WATER, Pos, Hero->Object.Angle, Light, 0);
            }
        }

        if (gMapManager.WorldActive == WD_30BATTLECASTLE && Texture == 5)
            suf -= WaterMove;
        else
            suf += WaterMove;

        if (Scale)
        {
            Water3 = TerrainGrassWind[TerrainIndex1] * 0.008f;
            Water4 = TerrainGrassWind[TerrainIndex2] * 0.008f;
        }
        else
        {
            Water3 = TerrainGrassWind[TerrainIndex1] * 0.002f;
            Water4 = TerrainGrassWind[TerrainIndex2] * 0.002f;
        }

        TEXCOORD(TerrainTextureCoord[0], suf + Water1, svf + Water3);
        TEXCOORD(TerrainTextureCoord[1], suf + Width + Water2, svf + Water4);
        TEXCOORD(TerrainTextureCoord[2], suf + Width + Water2, svf + Height + Water4);
        TEXCOORD(TerrainTextureCoord[3], suf + Water1, svf + Height + Water3);
    }
}

int WaterTextureNumber = 0;

void RenderTerrainFace(float xf, float yf, int xi, int yi, float lodf)
{
    RenderTerrainVisual(xi, yi);

    if (TerrainFlag != TERRAIN_MAP_GRASS)
    {
        int Texture;
        bool Alpha;
        bool Water = false;
        if (TerrainMappingAlpha[TerrainIndex1] >= 1.f && TerrainMappingAlpha[TerrainIndex2] >= 1.f && TerrainMappingAlpha[TerrainIndex3] >= 1.f && TerrainMappingAlpha[TerrainIndex4] >= 1.f)
        {
            Texture = TerrainMappingLayer2[TerrainIndex1];
            Alpha = false;
        }
        else
        {
            Texture = TerrainMappingLayer1[TerrainIndex1];
            Alpha = true;
            if (Texture == 5)
            {
                Water = true;
            }
            if (Texture == 11 && (gMapManager.IsPKField() || IsDoppelGanger2()))
                Water = true;
        }
        FaceTexture(Texture, xf, yf, Water, false);
        RenderFace(Texture, xi, yi);

        if (TerrainMappingAlpha[TerrainIndex1] > 0.f
            || TerrainMappingAlpha[TerrainIndex2] > 0.f
            || TerrainMappingAlpha[TerrainIndex3] > 0.f
            || TerrainMappingAlpha[TerrainIndex4] > 0.f)
        {
            if ((gMapManager.WorldActive == WD_7ATLANSE || IsDoppelGanger3()) && TerrainMappingLayer2[TerrainIndex1] == 5)
            {
                Texture = BITMAP_WATER - BITMAP_MAPTILE + WaterTextureNumber;
                FaceTexture(Texture, xf, yf, false, true);
                RenderFaceBlend(Texture, xi, yi);
            }
            else if (Alpha)
            {
                Texture = TerrainMappingLayer2[TerrainIndex1];
                if (Texture != 5)
                    Water = false;
                if (Texture != 255)
                {
                    FaceTexture(Texture, xf, yf, Water, false);
                    RenderFaceAlpha(Texture, xi, yi);
                }
            }
        }
    }
    else
    {
        if (TerrainMappingAlpha[TerrainIndex1] > 0.f || TerrainMappingAlpha[TerrainIndex2] > 0.f || TerrainMappingAlpha[TerrainIndex3] > 0.f || TerrainMappingAlpha[TerrainIndex4] > 0.f)
        {
            return;
        }
        if (
            CurrentLayer == 0 && (gMapManager.InBloodCastle() == false)
            )
        {
            int Texture = BITMAP_MAPGRASS + TerrainMappingLayer1[TerrainIndex1];

            BITMAP_t* pBitmap = Bitmaps.FindTexture(Texture);
            if (pBitmap)
            {
                float Height = pBitmap->Height * 2.f;
                BindTexture(Texture);

                if (gMapManager.IsPKField() || IsDoppelGanger2())
                    EnableAlphaBlend();

                float Width = 64.f / 256.f;
                float su = xf * Width;
                su += TerrainGrassTexture[yi & TERRAIN_SIZE_MASK];
                TEXCOORD(TerrainTextureCoord[0], su, 0.f);
                TEXCOORD(TerrainTextureCoord[1], su + Width, 0.f);
                TEXCOORD(TerrainTextureCoord[2], su + Width, 1.f);
                TEXCOORD(TerrainTextureCoord[3], su, 1.f);
                VectorCopy(TerrainVertex[0], TerrainVertex[3]);
                VectorCopy(TerrainVertex[2], TerrainVertex[1]);
                TerrainVertex[0][2] += Height;
                TerrainVertex[1][2] += Height;
                TerrainVertex[0][0] += -50.f;
                TerrainVertex[1][0] += -50.f;
#ifdef ASG_ADD_MAP_KARUTAN
                if (IsKarutanMap())
                {
                    TerrainVertex[0][1] += g_fTerrainGrassWind1[TerrainIndex1];
                    TerrainVertex[1][1] += g_fTerrainGrassWind1[TerrainIndex2];
                }
                else
                {
#endif	// ASG_ADD_MAP_KARUTAN
                    TerrainVertex[0][1] += TerrainGrassWind[TerrainIndex1];
                    TerrainVertex[1][1] += TerrainGrassWind[TerrainIndex2];
#ifdef ASG_ADD_MAP_KARUTAN
                }
#endif	// ASG_ADD_MAP_KARUTAN
                glBegin(GL_QUADS);
                glTexCoord2f(TerrainTextureCoord[0][0], TerrainTextureCoord[0][1]);
                glColor3fv(PrimaryTerrainLight[TerrainIndex1]);
                glVertex3fv(TerrainVertex[0]);
                glTexCoord2f(TerrainTextureCoord[1][0], TerrainTextureCoord[1][1]);
                glColor3fv(PrimaryTerrainLight[TerrainIndex2]);
                glVertex3fv(TerrainVertex[1]);
                glTexCoord2f(TerrainTextureCoord[2][0], TerrainTextureCoord[2][1]);
                glColor3fv(PrimaryTerrainLight[TerrainIndex3]);
                glVertex3fv(TerrainVertex[2]);
                glTexCoord2f(TerrainTextureCoord[3][0], TerrainTextureCoord[3][1]);
                glColor3fv(PrimaryTerrainLight[TerrainIndex4]);
                glVertex3fv(TerrainVertex[3]);
                glEnd();

                if (gMapManager.IsPKField() || IsDoppelGanger2())
                    DisableAlphaBlend();
            }
        }
    }
}

void RenderTerrainFace_After(float xf, float yf, int xi, int yi, float lodf)
{
    if (TerrainFlag != TERRAIN_MAP_GRASS)
    {
        int Texture;
        int Water = 0;
        if (TerrainMappingAlpha[TerrainIndex1] >= 1.f && TerrainMappingAlpha[TerrainIndex2] >= 1.f && TerrainMappingAlpha[TerrainIndex3] >= 1.f && TerrainMappingAlpha[TerrainIndex4] >= 1.f)
        {
            Texture = TerrainMappingLayer2[TerrainIndex1];
        }
        else
        {
            Texture = TerrainMappingLayer1[TerrainIndex1];
            if (TerrainMappingLayer1[TerrainIndex1] == 5)
                Water = 1;
        }

        FaceTexture(Texture, xf, yf, Water, false);
        RenderFace_After(Texture, xi, yi);
    }
}

extern PATH* path;

extern int SelectWall;

bool RenderTerrainTile(float xf, float yf, int xi, int yi, float lodf, int lodi, bool Flag)
{
    TerrainIndex1 = TERRAIN_INDEX(xi, yi);
    if ((TerrainWall[TerrainIndex1] & TW_NOGROUND) == TW_NOGROUND && !Flag) return false;

    TerrainIndex2 = TERRAIN_INDEX(xi + lodi, yi);
    TerrainIndex3 = TERRAIN_INDEX(xi + lodi, yi + lodi);
    TerrainIndex4 = TERRAIN_INDEX(xi, yi + lodi);

    float sx = xf * TERRAIN_SCALE;
    float sy = yf * TERRAIN_SCALE;

    Vector(sx, sy, BackTerrainHeight[TerrainIndex1], TerrainVertex[0]);
    Vector(sx + TERRAIN_SCALE, sy, BackTerrainHeight[TerrainIndex2], TerrainVertex[1]);
    Vector(sx + TERRAIN_SCALE, sy + TERRAIN_SCALE, BackTerrainHeight[TerrainIndex3], TerrainVertex[2]);
    Vector(sx, sy + TERRAIN_SCALE, BackTerrainHeight[TerrainIndex4], TerrainVertex[3]);

    if ((TerrainWall[TerrainIndex1] & TW_HEIGHT) == TW_HEIGHT) TerrainVertex[0][2] = g_fSpecialHeight;
    if ((TerrainWall[TerrainIndex2] & TW_HEIGHT) == TW_HEIGHT) TerrainVertex[1][2] = g_fSpecialHeight;
    if ((TerrainWall[TerrainIndex3] & TW_HEIGHT) == TW_HEIGHT) TerrainVertex[2][2] = g_fSpecialHeight;
    if ((TerrainWall[TerrainIndex4] & TW_HEIGHT) == TW_HEIGHT) TerrainVertex[3][2] = g_fSpecialHeight;

    if (!Flag)
    {
        RenderTerrainFace(xf, yf, xi, yi, lodf);
#ifdef SHOW_PATH_INFO
#ifdef CSK_DEBUG_MAP_PATHFINDING
        if (g_bShowPath == true)
#endif // CSK_DEBUG_MAP_PATHFINDING
        {
            if (2 <= path->GetClosedStatus(TerrainIndex1))
            {
                EnableAlphaTest();
                DisableTexture();
                glBegin(GL_TRIANGLE_FAN);
                if (4 <= path->GetClosedStatus(TerrainIndex1))
                {
                    glColor4f(0.3f, 0.3f, 1.0f, 0.5f);
                }
                else
                {
                    glColor4f(1.0f, 1.0f, 1.0f, 0.3f);
                }
                for (int i = 0; i < 4; i++)
                {
                    glVertex3fv(TerrainVertex[i]);
                }
                glEnd();
                DisableAlphaBlend();
            }
        }
#endif // SHOW_PATH_INFO
    }
    else
    {
#ifdef _DEBUG
        if (EditFlag != EDIT_LIGHT)
        {
            DisableTexture();
            glColor3f(0.5f, 0.5f, 0.5f);
            glBegin(GL_LINE_STRIP);
            for (int i = 0; i < 4; i++)
            {
                glVertex3fv(TerrainVertex[i]);
            }
            glEnd();
            DisableAlphaBlend();
        }
#endif// _DEBUG

        vec3_t Normal;
        FaceNormalize(TerrainVertex[0], TerrainVertex[1], TerrainVertex[2], Normal);
        bool Success = CollisionDetectLineToFace(MousePosition, MouseTarget, 3, TerrainVertex[0], TerrainVertex[1], TerrainVertex[2], TerrainVertex[3], Normal);
        if (Success == false)
        {
            FaceNormalize(TerrainVertex[0], TerrainVertex[2], TerrainVertex[3], Normal);
            Success = CollisionDetectLineToFace(MousePosition, MouseTarget, 3, TerrainVertex[0], TerrainVertex[2], TerrainVertex[3], TerrainVertex[1], Normal);
        }
        if (Success == true)
        {
            SelectFlag = true;
            SelectXF = xf;
            SelectYF = yf;
        }
#ifdef CSK_DEBUG_MAP_ATTRIBUTE
        if (EditFlag == EDIT_WALL &&
            ((SelectWall == 0 && (TerrainWall[TerrainIndex1] & TW_NOMOVE) == TW_NOMOVE)
                || (SelectWall == 2 && (TerrainWall[TerrainIndex1] & TW_SAFEZONE) == TW_SAFEZONE)
                || (SelectWall == 6 && (TerrainWall[TerrainIndex1] & TW_CAMERA_UP) == TW_CAMERA_UP)
                || (SelectWall == 7 && (TerrainWall[TerrainIndex1] & TW_NOATTACKZONE) == TW_NOATTACKZONE)
                || (SelectWall == 8 && (TerrainWall[TerrainIndex1] & TW_ATT1) == TW_ATT1)
                || (SelectWall == 9 && (TerrainWall[TerrainIndex1] & TW_ATT2) == TW_ATT2)
                || (SelectWall == 10 && (TerrainWall[TerrainIndex1] & TW_ATT3) == TW_ATT3)
                || (SelectWall == 11 && (TerrainWall[TerrainIndex1] & TW_ATT4) == TW_ATT4)
                || (SelectWall == 12 && (TerrainWall[TerrainIndex1] & TW_ATT5) == TW_ATT5)
                || (SelectWall == 13 && (TerrainWall[TerrainIndex1] & TW_ATT6) == TW_ATT6)
                || (SelectWall == 14 && (TerrainWall[TerrainIndex1] & TW_ATT7) == TW_ATT7)
                ))
        {
            DisableDepthTest();
            EnableAlphaTest();
            DisableTexture();

            glBegin(GL_TRIANGLE_FAN);
            glColor4f(1.f, 0.5f, 0.5f, 0.3f);
            for (int i = 0; i < 4; i++)
            {
                glVertex3fv(TerrainVertex[i]);
            }
            glEnd();

            DisableAlphaBlend();
        }
#endif // CSK_DEBUG_MAP_ATTRIBUTE

        return Success;
    }
    return false;
}

void RenderTerrainTile_After(float xf, float yf, int xi, int yi, float lodf, int lodi, bool Flag)
{
    TerrainIndex1 = TERRAIN_INDEX(xi, yi);
    TerrainIndex2 = TERRAIN_INDEX(xi + lodi, yi);
    TerrainIndex3 = TERRAIN_INDEX(xi + lodi, yi + lodi);
    TerrainIndex4 = TERRAIN_INDEX(xi, yi + lodi);

    float sx = xf * TERRAIN_SCALE;
    float sy = yf * TERRAIN_SCALE;

    Vector(sx, sy, BackTerrainHeight[TerrainIndex1], TerrainVertex[0]);
    Vector(sx + TERRAIN_SCALE, sy, BackTerrainHeight[TerrainIndex2], TerrainVertex[1]);
    Vector(sx + TERRAIN_SCALE, sy + TERRAIN_SCALE, BackTerrainHeight[TerrainIndex3], TerrainVertex[2]);
    Vector(sx, sy + TERRAIN_SCALE, BackTerrainHeight[TerrainIndex4], TerrainVertex[3]);

    if ((TerrainWall[TerrainIndex1] & TW_HEIGHT) == TW_HEIGHT) TerrainVertex[0][2] = g_fSpecialHeight;
    if ((TerrainWall[TerrainIndex2] & TW_HEIGHT) == TW_HEIGHT) TerrainVertex[1][2] = g_fSpecialHeight;
    if ((TerrainWall[TerrainIndex3] & TW_HEIGHT) == TW_HEIGHT) TerrainVertex[2][2] = g_fSpecialHeight;
    if ((TerrainWall[TerrainIndex4] & TW_HEIGHT) == TW_HEIGHT) TerrainVertex[3][2] = g_fSpecialHeight;

    if (!Flag)
    {
        if ((TerrainWall[TerrainIndex1] & TW_NOGROUND) != TW_NOGROUND)
            RenderTerrainFace_After(xf, yf, xi, yi, lodf);
    }
}

void RenderTerrainBitmapTile(float xf, float yf, float lodf, int lodi, vec3_t c[4], bool LightEnable, float Alpha, float Height = 0.f)
{
    int xi = (int)xf;
    int yi = (int)yf;
    if (xi < 0 || yi < 0 || xi >= TERRAIN_SIZE_MASK || yi >= TERRAIN_SIZE_MASK) return;
    float TileScale = TERRAIN_SCALE * lodf;
    float sx = xf * TERRAIN_SCALE;
    float sy = yf * TERRAIN_SCALE;
    TerrainIndex1 = TERRAIN_INDEX(xi, yi);
    TerrainIndex2 = TERRAIN_INDEX(xi + lodi, yi);
    TerrainIndex3 = TERRAIN_INDEX(xi + lodi, yi + lodi);
    TerrainIndex4 = TERRAIN_INDEX(xi, yi + lodi);
    Vector(sx, sy, BackTerrainHeight[TerrainIndex1] + Height, TerrainVertex[0]);
    Vector(sx + TileScale, sy, BackTerrainHeight[TerrainIndex2] + Height, TerrainVertex[1]);
    Vector(sx + TileScale, sy + TileScale, BackTerrainHeight[TerrainIndex3] + Height, TerrainVertex[2]);
    Vector(sx, sy + TileScale, BackTerrainHeight[TerrainIndex4] + Height, TerrainVertex[3]);

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

void RenderTerrainBitmap(int Texture, int mxi, int myi, float Rotation)
{
    glColor3f(1.f, 1.f, 1.f);

    vec3_t Angle;
    Vector(0.f, 0.f, Rotation, Angle);
    float Matrix[3][4];
    AngleMatrix(Angle, Matrix);

    BindTexture(Texture);
    BITMAP_t* b = &Bitmaps[Texture];
    float TexScaleU = 64.f / b->Width;
    float TexScaleV = 64.f / b->Height;
    for (float y = 0.f; y < b->Height / 64.f; y += 1.f)
    {
        for (float x = 0.f; x < b->Width / 64.f; x += 1.f)
        {
            vec3_t p1[4], p2[4];
            Vector((x)*TexScaleU, (y)*TexScaleV, 0.f, p1[0]);
            Vector((x + 1.f) * TexScaleU, (y)*TexScaleV, 0.f, p1[1]);
            Vector((x + 1.f) * TexScaleU, (y + 1.f) * TexScaleV, 0.f, p1[2]);
            Vector((x)*TexScaleU, (y + 1.f) * TexScaleV, 0.f, p1[3]);
            for (int i = 0; i < 4; i++)
            {
                p1[i][0] -= 0.5f;
                p1[i][1] -= 0.5f;
                VectorRotate(p1[i], Matrix, p2[i]);
                p2[i][0] += 0.5f;
                p2[i][1] += 0.5f;
            }
            RenderTerrainBitmapTile((float)mxi + x, (float)myi + y, 1.f, 1, p2, true, 1.f);
        }
    }
}

void RenderTerrainAlphaBitmap(int Texture, float xf, float yf, float SizeX, float SizeY, vec3_t Light, float Rotation, float Alpha, float Height)
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
    float mxf = (xf / TERRAIN_SCALE);
    float myf = (yf / TERRAIN_SCALE);
    int   mxi = (int)(mxf);
    int   myi = (int)(myf);

    float Size;
    if (SizeX >= SizeY)
        Size = SizeX;
    else
        Size = SizeY;
    float TexU = (((float)mxi - mxf) + 0.5f * Size);
    float TexV = (((float)myi - myf) + 0.5f * Size);
    float TexScaleU = 1.f / Size;
    float TexScaleV = 1.f / Size;
    Size = (float)((int)Size + 1);
    float Aspect = SizeX / SizeY;
    for (float y = -Size; y <= Size; y += 1.f)
    {
        for (float x = -Size; x <= Size; x += 1.f)
        {
            vec3_t p1[4], p2[4];
            Vector((TexU + x) * TexScaleU, (TexV + y) * TexScaleV, 0.f, p1[0]);
            Vector((TexU + x + 1.f) * TexScaleU, (TexV + y) * TexScaleV, 0.f, p1[1]);
            Vector((TexU + x + 1.f) * TexScaleU, (TexV + y + 1.f) * TexScaleV, 0.f, p1[2]);
            Vector((TexU + x) * TexScaleU, (TexV + y + 1.f) * TexScaleV, 0.f, p1[3]);
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
            RenderTerrainBitmapTile((float)mxi + x, (float)myi + y, 1.f, 1, p2, false, Alpha, Height);
        }
    }
}

vec3_t  FrustrumVertex[5];
vec3_t  FrustrumFaceNormal[5];
float   FrustrumFaceD[5];
int     FrustrumBoundMinX = 0;
int     FrustrumBoundMinY = 0;
int     FrustrumBoundMaxX = TERRAIN_SIZE_MASK;
int     FrustrumBoundMaxY = TERRAIN_SIZE_MASK;

float FrustrumX[4];
float FrustrumY[4];

extern int GetScreenWidth();

void CreateFrustrum2D(vec3_t Position)
{
    float Width = 0.0f, CameraViewFar = 0.0f, CameraViewNear = 0.0f, CameraViewTarget = 0.0f;
    float WidthFar = 0.0f, WidthNear = 0.0f;

    if (gMapManager.InBattleCastle() && SceneFlag == MAIN_SCENE)
    {
        Width = (float)GetScreenWidth() / 480.f;// * 0.1f;
        if (battleCastle::InBattleCastle2(Hero->Object.Position) && (Hero->Object.Position[0] < 17100.f || Hero->Object.Position[0]>18300.f))
        {
            CameraViewFar = 5100.f;// * 0.1f;
            CameraViewNear = CameraViewFar * 0.19f;//0.22
            CameraViewTarget = CameraViewFar * 0.47f;//0.47
            WidthFar = 2250.f * Width; // 1140.f
            WidthNear = 540.f * Width; // 540.f
        }
        else
        {
            CameraViewFar = 3300.f;// * 0.1f;
            CameraViewNear = CameraViewFar * 0.19f;//0.22
            CameraViewTarget = CameraViewFar * 0.47f;//0.47
            WidthFar = 1300.f * Width; // 1140.f
            WidthNear = 580.f * Width; // 540.f
        }
    }
    else if (gMapManager.WorldActive == WD_62SANTA_TOWN)
    {
        Width = (float)GetScreenWidth() / 450.f * 1.0f;
        CameraViewFar = 2400.f;
        CameraViewNear = CameraViewFar * 0.19f;
        CameraViewTarget = CameraViewFar * 0.47f;
        CameraViewFar = 2650.f;
        WidthFar = 1250.f * Width;
        WidthNear = 540.f * Width;
    }
    else if (gMapManager.IsPKField() || IsDoppelGanger2())
    {
        Width = (float)GetScreenWidth() / 500.f;
        CameraViewFar = 1700.0f;
        CameraViewNear = 55.0f;
        CameraViewTarget = 830.0f;
        CameraViewFar = 3300.f;
        WidthFar = 1900.f * Width;
        WidthNear = 600.f * Width;
    }
    else
    {
        static  int CameraLevel;

        if ((int)CameraDistanceTarget >= (int)CameraDistance)
            CameraLevel = g_shCameraLevel;

        switch (CameraLevel)
        {
        case 0:
            if (SceneFlag == LOG_IN_SCENE)
            {
            }
            else if (SceneFlag == CHARACTER_SCENE)
            {
                Width = (float)GetScreenWidth() / 640.f * 9.1f * 0.404998f;
            }
            else if (g_Direction.m_CKanturu.IsMayaScene())
            {
                Width = (float)GetScreenWidth() / 640.f * 10.0f * 0.115f;
            }
            else
            {
                Width = (float)GetScreenWidth()/640.f * 1.1f;
            }

            if (SceneFlag == LOG_IN_SCENE)
            {
            }
            else if (SceneFlag == CHARACTER_SCENE)
            {
                CameraViewFar = 2000.f * 9.1f * 0.404998f;
            }
            else if (gMapManager.WorldActive == WD_39KANTURU_3RD)
            {
                CameraViewFar = 2000.f * 10.0f * 0.115f;
            }
            else
            {
                CameraViewFar = 2400.f;
            }

            if (SceneFlag == LOG_IN_SCENE)
            {
                Width = (float)GetScreenWidth() / 640.f;
                CameraViewFar = 2400.f * 17.0f * 13.0f;
                CameraViewNear = 2400.f * 17.0f * 0.5f;
                CameraViewTarget = 2400.f * 17.0f * 0.5f;
                WidthFar = 5000.f * Width;
                WidthNear = 300.f * Width;
            }
            else
            {
                CameraViewNear = CameraViewFar * 0.19f;//0.22
                CameraViewTarget = CameraViewFar * 0.47f;//0.47
                WidthFar = 1190.f * Width * sqrtf(CameraFOV / 33.f); // 1140.f
                WidthNear = 540.f * Width * sqrtf(CameraFOV / 33.f); // 540.f
            }
            break;
        case 1:
            Width = (float)GetScreenWidth() / 500.f + 0.1f;// * 0.1f;
            CameraViewFar = 2700.f;// * 0.1f;
            CameraViewNear = CameraViewFar * 0.19f;//0.22
            CameraViewTarget = CameraViewFar * 0.47f;//0.47
            WidthFar = 1200.f * Width; // 1140.f
            WidthNear = 540.f * Width; // 540.f
            break;
        case 2:
            Width = (float)GetScreenWidth() / 500.f + 0.1f;// * 0.1f;
            CameraViewFar = 3000.f;// * 0.1f;
            CameraViewNear = CameraViewFar * 0.19f;//0.22
            CameraViewTarget = CameraViewFar * 0.47f;//0.47
            WidthFar = 1300.f * Width; // 1140.f
            WidthNear = 540.f * Width; // 540.f
            break;
        case 3:
            Width = (float)GetScreenWidth() / 500.f + 0.1f;// * 0.1f;
            CameraViewFar = 3300.f;// * 0.1f;
            CameraViewNear = CameraViewFar * 0.19f;//0.22
            CameraViewTarget = CameraViewFar * 0.47f;//0.47
            WidthFar = 1500.f * Width; // 1140.f
            WidthNear = 580.f * Width; // 540.f
            break;
        case 4:
            Width = (float)GetScreenWidth() / 500.f + 0.1f;// * 0.1f;
            CameraViewFar = 5100.f;// * 0.1f;
            CameraViewNear = CameraViewFar * 0.19f;//0.22
            CameraViewTarget = CameraViewFar * 0.47f;//0.47
            WidthFar = 2250.f * Width; // 1140.f
            WidthNear = 540.f * Width; // 540.f
            break;
        case 5:
            Width = (float)GetScreenWidth() / 500.f + 0.1f;// * 0.1f;
            CameraViewFar = 3400.f;// * 0.1f;
            CameraViewNear = CameraViewFar * 0.19f;//0.22
            CameraViewTarget = CameraViewFar * 0.47f;//0.47
            WidthFar = 1600.f * Width; // 1140.f
            WidthNear = 660.f * Width; // 540.f
            break;
        }
    }

    vec3_t p[4];
    Vector(-WidthFar, CameraViewFar - CameraViewTarget, 0.f, p[0]);
    Vector(WidthFar, CameraViewFar - CameraViewTarget, 0.f, p[1]);
    Vector(WidthNear, CameraViewNear - CameraViewTarget, 0.f, p[2]);
    Vector(-WidthNear, CameraViewNear - CameraViewTarget, 0.f, p[3]);
    vec3_t Angle;
    float Matrix[3][4];

    if (gMapManager.WorldActive == WD_73NEW_LOGIN_SCENE)
    {
        VectorScale(CameraAngle, -1.0f, Angle);
        CCameraMove::GetInstancePtr()->SetFrustumAngle(89.5f);
        vec3_t _Temp = { CCameraMove::GetInstancePtr()->GetFrustumAngle(), 0.0f, 0.0f };
        VectorAdd(Angle, _Temp, Angle);
    }
    else
    {
        Vector(0.f, 0.f, -CameraAngle[2], Angle);
    }

    AngleMatrix(Angle, Matrix);
    vec3_t Frustrum[4];
    for (int i = 0; i < 4; i++)
    {
        VectorRotate(p[i], Matrix, Frustrum[i]);
        VectorAdd(Frustrum[i], Position, Frustrum[i]);
        FrustrumX[i] = Frustrum[i][0] * 0.01f;
        FrustrumY[i] = Frustrum[i][1] * 0.01f;
    }
}

bool TestFrustrum2D(float x, float y, float Range)
{
    if (SceneFlag == SERVER_LIST_SCENE || SceneFlag == WEBZEN_SCENE || SceneFlag == LOADING_SCENE)
        return true;

    int j = 3;
    for (int i = 0; i < 4; j = i, i++)
    {
        float d = (FrustrumX[i] - x) * (FrustrumY[j] - y) -
            (FrustrumX[j] - x) * (FrustrumY[i] - y);
        if (d <= Range)
        {
            return false;
        }
    }
    return true;
}

void CreateFrustrum(float xAspect, float yAspect, vec3_t position)
{
    const auto fovv = tanf(CameraFOV * Q_PI / 360.f);
    float Distance = CameraViewFar;
    float Width = fovv * Distance * xAspect + 100.f;
    float Height = fovv * Distance * yAspect + 100.f;

    vec3_t Temp[5];
    Vector(0.f, 0.f, 0.f, Temp[0]);
    Vector(-Width, Height, -Distance, Temp[1]);
    Vector(Width, Height, -Distance, Temp[2]);
    Vector(Width, -Height, -Distance, Temp[3]);
    Vector(-Width, -Height, -Distance, Temp[4]);

    float FrustrumMinX = (float)TERRAIN_SIZE * TERRAIN_SCALE;
    float FrustrumMinY = (float)TERRAIN_SIZE * TERRAIN_SCALE;
    float FrustrumMaxX = 0.f;
    float FrustrumMaxY = 0.f;
    float Matrix[3][4];
    GetOpenGLMatrix(Matrix);
    for (int i = 0; i < 5; i++)
    {
        vec3_t t;
        VectorIRotate(Temp[i], Matrix, t);
        VectorAdd(t, CameraPosition, FrustrumVertex[i]);
        if (FrustrumMinX > FrustrumVertex[i][0]) FrustrumMinX = FrustrumVertex[i][0];
        if (FrustrumMinY > FrustrumVertex[i][1]) FrustrumMinY = FrustrumVertex[i][1];
        if (FrustrumMaxX < FrustrumVertex[i][0]) FrustrumMaxX = FrustrumVertex[i][0];
        if (FrustrumMaxY < FrustrumVertex[i][1]) FrustrumMaxY = FrustrumVertex[i][1];
    }

    int tileWidth = 4;

    FrustrumBoundMinX = (int)(FrustrumMinX / TERRAIN_SCALE) / tileWidth * tileWidth - tileWidth;
    FrustrumBoundMinY = (int)(FrustrumMinY / TERRAIN_SCALE) / tileWidth * tileWidth - tileWidth;
    FrustrumBoundMaxX = (int)(FrustrumMaxX / TERRAIN_SCALE) / tileWidth * tileWidth + tileWidth;
    FrustrumBoundMaxY = (int)(FrustrumMaxY / TERRAIN_SCALE) / tileWidth * tileWidth + tileWidth;
    FrustrumBoundMinX = FrustrumBoundMinX < 0 ? 0 : FrustrumBoundMinX;
    FrustrumBoundMinY = FrustrumBoundMinY < 0 ? 0 : FrustrumBoundMinY;
    FrustrumBoundMaxX = FrustrumBoundMaxX > TERRAIN_SIZE_MASK - tileWidth ? TERRAIN_SIZE_MASK - tileWidth : FrustrumBoundMaxX;
    FrustrumBoundMaxY = FrustrumBoundMaxY > TERRAIN_SIZE_MASK - tileWidth ? TERRAIN_SIZE_MASK - tileWidth : FrustrumBoundMaxY;

    FaceNormalize(FrustrumVertex[0], FrustrumVertex[1], FrustrumVertex[2], FrustrumFaceNormal[0]);
    FaceNormalize(FrustrumVertex[0], FrustrumVertex[2], FrustrumVertex[3], FrustrumFaceNormal[1]);
    FaceNormalize(FrustrumVertex[0], FrustrumVertex[3], FrustrumVertex[4], FrustrumFaceNormal[2]);
    FaceNormalize(FrustrumVertex[0], FrustrumVertex[4], FrustrumVertex[1], FrustrumFaceNormal[3]);
    FaceNormalize(FrustrumVertex[3], FrustrumVertex[2], FrustrumVertex[1], FrustrumFaceNormal[4]);
    FrustrumFaceD[0] = -DotProduct(FrustrumVertex[0], FrustrumFaceNormal[0]);
    FrustrumFaceD[1] = -DotProduct(FrustrumVertex[0], FrustrumFaceNormal[1]);
    FrustrumFaceD[2] = -DotProduct(FrustrumVertex[0], FrustrumFaceNormal[2]);
    FrustrumFaceD[3] = -DotProduct(FrustrumVertex[0], FrustrumFaceNormal[3]);
    FrustrumFaceD[4] = -DotProduct(FrustrumVertex[1], FrustrumFaceNormal[4]);
    
    CreateFrustrum2D(position);
}

bool TestFrustrum(vec3_t Position, float Range)
{
    for (int i = 0; i < 5; i++)
    {
        float Value;
        Value = FrustrumFaceD[i] + DotProduct(Position, FrustrumFaceNormal[i]);
        if (Value < -Range) return false;
    }
    return true;
}

#ifdef DYNAMIC_FRUSTRUM

void CFrustrum::Make(vec3_t vEye, float fFov, float fAspect, float fDist)
{
    float Width = tanf(fFov * 0.5f * Q_PI / 180.f) * fDist * fAspect + 100.f;
    float Height = Width * 3.f / 4.f;
    vec3_t Temp[5];
    vec3_t FrustrumVertex[5];
    Vector(0.f, 0.f, 0.f, Temp[0]);
    Vector(-Width, Height, -fDist, Temp[1]);
    Vector(Width, Height, -fDist, Temp[2]);
    Vector(Width, -Height, -fDist, Temp[3]);
    Vector(-Width, -Height, -fDist, Temp[4]);

    float Matrix[3][4];
    GetOpenGLMatrix(Matrix);
    for (int i = 0; i < 5; i++)
    {
        vec3_t t;
        VectorIRotate(Temp[i], Matrix, t);
        VectorAdd(t, vEye, FrustrumVertex[i]);
    }

    FaceNormalize(FrustrumVertex[0], FrustrumVertex[1], FrustrumVertex[2], m_FrustrumNorm[0]);
    FaceNormalize(FrustrumVertex[0], FrustrumVertex[2], FrustrumVertex[3], m_FrustrumNorm[1]);
    FaceNormalize(FrustrumVertex[0], FrustrumVertex[3], FrustrumVertex[4], m_FrustrumNorm[2]);
    FaceNormalize(FrustrumVertex[0], FrustrumVertex[4], FrustrumVertex[1], m_FrustrumNorm[3]);
    FaceNormalize(FrustrumVertex[3], FrustrumVertex[2], FrustrumVertex[1], m_FrustrumNorm[4]);
    m_FrustrumD[0] = -DotProduct(FrustrumVertex[0], m_FrustrumNorm[0]);
    m_FrustrumD[1] = -DotProduct(FrustrumVertex[0], m_FrustrumNorm[1]);
    m_FrustrumD[2] = -DotProduct(FrustrumVertex[0], m_FrustrumNorm[2]);
    m_FrustrumD[3] = -DotProduct(FrustrumVertex[0], m_FrustrumNorm[3]);
    m_FrustrumD[4] = -DotProduct(FrustrumVertex[1], m_FrustrumNorm[4]);
}

void CFrustrum::Create(vec3_t vEye, float fFov, float fAspect, float fDist)
{
    Make(vEye, fFov, fAspect, fDist);

    SetEye(vEye);
    SetFOV(fFov);
    SetAspect(fAspect);
    SetDist(fDist);
}

bool CFrustrum::Test(vec3_t vPos, float fRange)
{
    for (int i = 0; i < 5; ++i)
    {
        float fValue;
        fValue = m_FrustrumD[i] + DotProduct(vPos, m_FrustrumNorm[i]);
        if (fValue < -fRange) return false;
    }
    return true;
}

void CFrustrum::Reset()
{
    Make(m_vEye, m_fFov, m_fAspect, m_fDist);
}

void ResetAllFrustrum()
{
    FrustrumMap_t::iterator iter = g_FrustrumMap.begin();
    for (; iter != g_FrustrumMap.end(); ++iter)
    {
        CFrustrum* pData = iter->second;
        if (!pData) continue;
        pData->SetEye(CameraPosition);
        pData->Reset();
    }
}

CFrustrum* FindFrustrum(unsigned int iID)
{
    FrustrumMap_t::iterator iter = g_FrustrumMap.find(iID);
    if (iter != g_FrustrumMap.end()) return (CFrustrum*)iter->second;
    return NULL;
}

void DeleteAllFrustrum()
{
    FrustrumMap_t::iterator iter = g_FrustrumMap.begin();
    for (; iter != g_FrustrumMap.end(); ++iter)
    {
        CFrustrum* pData = iter->second;
        SAFE_DELETE(pData);
    }
    g_FrustrumMap.clear();
}

#endif// DYNAMIC_FRUSTRUM

/*bool TestFrustrum(vec3_t Position,float Range)
{
    int j = 3;
    for(int i=0;i<4;j=i,i++)
    {
        float d = (Frustrum[i][0]-Position[0]) * (Frustrum[j][1]-Position[1]) -
                  (Frustrum[j][0]-Position[0]) * (Frustrum[i][1]-Position[1]);
        if(d < 0.f) return false;
    }
    return true;
}*/

extern float RainCurrent;
extern int EnableEvent;

void InitTerrainLight()
{
    int xi, yi;
    yi = FrustrumBoundMinY;
    for (; yi <= FrustrumBoundMaxY + 3; yi += 1)
    {
        xi = FrustrumBoundMinX;
        for (; xi <= FrustrumBoundMaxX + 3; xi += 1)
        {
            int Index = TERRAIN_INDEX_REPEAT(xi, yi);
            VectorCopy(BackTerrainLight[Index], PrimaryTerrainLight[Index]);
        }
    }
    float WindScale;
    float WindSpeed;

    if (EnableEvent == 0)
    {
        WindScale = 10.f;
        WindSpeed = (int)WorldTime % (360000 * 2) * (0.002f);
    }
    else
    {
        WindScale = 10.f;
        WindSpeed = (int)WorldTime % 36000 * (0.01f);
    }
#ifdef ASG_ADD_MAP_KARUTAN
    float WindScale1 = 0.f;
    float WindSpeed1 = 0.f;
    if (IsKarutanMap())
    {
        WindScale1 = 15.f;
        WindSpeed1 = (int)WorldTime % 36000 * (0.008f);
    }
#endif	// ASG_ADD_MAP_KARUTAN
    yi = FrustrumBoundMinY;

    for (; yi <= min(FrustrumBoundMaxY + 3, TERRAIN_SIZE_MASK); yi += 1)
    {
        xi = FrustrumBoundMinX;
        auto xf = (float)xi;
        for (; xi <= min(FrustrumBoundMaxX + 3, TERRAIN_SIZE_MASK); xi += 1, xf += 1.f)
        {
            int Index = TERRAIN_INDEX(xi, yi);
            if (gMapManager.WorldActive == WD_8TARKAN)
            {
                TerrainGrassWind[Index] = sinf(WindSpeed + xf * 50.f) * WindScale;
            }
#ifdef ASG_ADD_MAP_KARUTAN
            else if (IsKarutanMap())
            {
                TerrainGrassWind[Index] = sinf(WindSpeed + xf * 50.f) * WindScale;
                g_fTerrainGrassWind1[Index] = sinf(WindSpeed1 + xf * 50.f) * WindScale1;
            }
#endif	// ASG_ADD_MAP_KARUTAN

            else if (gMapManager.WorldActive == WD_57ICECITY || gMapManager.WorldActive == WD_58ICECITY_BOSS)
            {
                WindScale = 60.f;
                TerrainGrassWind[Index] = sinf(WindSpeed + xf * 50.f) * WindScale;
            }
            else
            {
                TerrainGrassWind[Index] = sinf(WindSpeed + xf * 5.f) * WindScale;
            }
        }
    }
}

void InitTerrainShadow()
{
    /*int xi,yi;
    yi = FrustrumBoundMinY*2;
    for(;yi<=FrustrumBoundMaxY*2;yi+=2)
    {
        xi = FrustrumBoundMinX*2;
        for(;xi<=FrustrumBoundMaxX*2;xi+=2)
        {
            int Index1 = (yi  )*512+(xi  );
            int Index2 = (yi  )*512+(xi+1);
            int Index3 = (yi+1)*512+(xi+1);
            int Index4 = (yi+1)*512+(xi  );
            if(TerrainShadow[Index1] >= 1.f)
            {
                Vector(0.f,0.f,0.f,PrimaryTerrainLight[Index1]);
                Vector(0.f,0.f,0.f,PrimaryTerrainLight[Index2]);
                Vector(0.f,0.f,0.f,PrimaryTerrainLight[Index3]);
                Vector(0.f,0.f,0.f,PrimaryTerrainLight[Index4]);
            }
        }
    }*/
}

void Ray(int sx1, int sy1, int sx2, int sy2)
{
    /*int ShadowIndex = (sy1*TERRAIN_SIZE*2+sx1);
    if(TerrainShadow[ShadowIndex]==1.f) return;

    int nx1,ny1,d1,d2,len1,len2;
    int px1 = sx2-sx1;
    int py1 = sy2-sy1;
    if(px1 < 0  ) {px1 = -px1;nx1 =-1             ;} else nx1 = 1;
    if(py1 < 0  ) {py1 = -py1;ny1 =-TERRAIN_SIZE*2;} else ny1 = TERRAIN_SIZE*2;
    if(px1 > py1) {len1 = px1;len2 = py1;d1 = ny1;d2 = nx1;}
    else          {len1 = py1;len2 = px1;d1 = nx1;d2 = ny1;}

    int error = 0,count = 0;
    float Shadow = 0.f;
    do{
        TerrainShadow[ShadowIndex] = Shadow;
        int x = ShadowIndex%(TERRAIN_SIZE*2)/2;
        int y = ShadowIndex/(TERRAIN_SIZE*2)/2;
        if(TerrainWall[TERRAIN_INDEX(x,y)] >= 5) Shadow = 1.f;
        error += len2;
        if(error > len1/2)
        {
            ShadowIndex += d1;
            error -= len1;
        }
        ShadowIndex += d2;
    } while(++count <= len1);*/
}

void InitTerrainRay(int HeroX, int HeroY)
{
    /*TerrainShadow[HeroY*512+HeroX] = 0.f;

    int xi,yi;
    yi = FrustrumBoundMinY*2;
    for(;yi<=FrustrumBoundMaxY*2;yi++)
    {
        xi = FrustrumBoundMinX*2;
        for(;xi<=FrustrumBoundMaxX*2;xi++)
        {
            TerrainShadow[(yi*TERRAIN_SIZE*2+xi)] = -1.f;
        }
    }
    yi = FrustrumBoundMinY*2;
    for(;yi<=FrustrumBoundMaxY*2;yi++)
    {
        xi = FrustrumBoundMinX*2;
        for(;xi<=FrustrumBoundMaxX*2;xi++)
        {
            Ray(HeroX,HeroY,xi,yi);
        }
    }*/
}

void RenderTerrainBlock(float xf, float yf, int xi, int yi, bool EditFlag)
{
    //int x = ((xi/4)&63);
    //int y = ((yi/4)&63);
    //int lodi = LodBuffer[y*64+x];
    int lodi = 1;
    auto lodf = (float)lodi;
    for (int i = 0; i < 4; i += lodi)
    {
        float temp = xf;
        for (int j = 0; j < 4; j += lodi)
        {
            if (TestFrustrum2D(xf + 0.5f, yf + 0.5f, 0.f) || CameraTopViewEnable)
            {
                RenderTerrainTile(xf, yf, xi + j, yi + i, lodf, lodi, EditFlag);
            }
            xf += lodf;
        }
        xf = temp;
        yf += lodf;
    }
}

void RenderTerrainFrustrum(bool EditFlag)
{
    int     xi;
    int     yi = FrustrumBoundMinY;
    float   xf;
    auto yf = (float)yi;

    for (; yi <= FrustrumBoundMaxY; yi += 4, yf += 4.f)
    {
        xi = FrustrumBoundMinX;
        xf = (float)xi;
        for (; xi <= FrustrumBoundMaxX; xi += 4, xf += 4.f)
        {
            if (TestFrustrum2D(xf + 2.f, yf + 2.f, g_fFrustumRange) || CameraTopViewEnable)
            {
                if (gMapManager.WorldActive == WD_73NEW_LOGIN_SCENE)
                {
                    float fDistance_x = CameraPosition[0] - xf / 0.01f;
                    float fDistance_y = CameraPosition[1] - yf / 0.01f;
                    float fDistance = sqrtf(fDistance_x * fDistance_x + fDistance_y * fDistance_y);

                    if (fDistance > 5200.f)
                        continue;
                }
                RenderTerrainBlock(xf, yf, xi, yi, EditFlag);
            }
        }
    }
}

void RenderTerrainBlock_After(float xf, float yf, int xi, int yi, bool EditFlag)
{
    int lodi = 1;
    auto lodf = (float)lodi;
    for (int i = 0; i < 4; i += lodi)
    {
        float temp = xf;
        for (int j = 0; j < 4; j += lodi)
        {
            if (TestFrustrum2D(xf + 0.5f, yf + 0.5f, 0.f) || CameraTopViewEnable)
            {
                RenderTerrainTile_After(xf, yf, xi + j, yi + i, lodf, lodi, EditFlag);
            }
            xf += lodf;
        }
        xf = temp;
        yf += lodf;
    }
}

void RenderTerrainFrustrum_After(bool EditFlag)
{
    int   xi, yi;
    float xf, yf;
    yi = FrustrumBoundMinY;
    yf = (float)yi;
    for (; yi <= FrustrumBoundMaxY; yi += 4, yf += 4.f)
    {
        xi = FrustrumBoundMinX;
        xf = (float)xi;
        for (; xi <= FrustrumBoundMaxX; xi += 4, xf += 4.f)
        {
            if (TestFrustrum2D(xf + 2.f, yf + 2.f, -80.f) || CameraTopViewEnable)
            {
                RenderTerrainBlock_After(xf, yf, xi, yi, EditFlag);
            }
        }
    }
}

extern int SelectMapping;
extern void RenderCharactersClient();

void RenderTerrain(bool EditFlag)
{
    if (!EditFlag)
    {
        if (gMapManager.WorldActive == WD_8TARKAN)
        {
            WaterMove = (float)((int)(WorldTime) % 40000) * 0.000025f;
        }
        else if (gMapManager.WorldActive == WD_42CHANGEUP3RD_2ND)
        {
            int iWorldTime = (int)WorldTime;
            int iRemainder = iWorldTime % 50000;
            WaterMove = (float)iRemainder * 0.00002f;
        }
        else
        {
            WaterMove = (float)((int)(WorldTime) % 20000) * 0.00005f;
        }
    }

    if (!EditFlag)
    {
        DisableAlphaBlend();
    }
    else
    {
        SelectFlag = false;
        InitCollisionDetectLineToFace();
    }

    TerrainFlag = TERRAIN_MAP_NORMAL;
    RenderTerrainFrustrum(EditFlag);
    //
    if (EditFlag && SelectFlag)
    {
        RenderTerrainTile(SelectXF, SelectYF, (int)SelectXF, (int)SelectYF, 1.f, 1, EditFlag);
    }
    if (!EditFlag)
    {
        EnableAlphaTest();
        if (TerrainGrassEnable && gMapManager.WorldActive != WD_7ATLANSE && !IsDoppelGanger3())
        {
            TerrainFlag = TERRAIN_MAP_GRASS;
            RenderTerrainFrustrum(EditFlag);
        }
        DisableDepthTest();
        EnableCullFace();
        RenderPointers();
        EnableDepthTest();
    }
}

void RenderTerrain_After(bool EditFlag)
{
    if (gMapManager.WorldActive != WD_39KANTURU_3RD)
        return;

    TerrainFlag = TERRAIN_MAP_NORMAL;
    RenderTerrainFrustrum_After(EditFlag);
}

OBJECT Sun;

void CreateSun()
{
    //Sun.Type = BITMAP_LIGHT;
    Sun.Scale = 8.f;
    Sun.AnimationFrame = 1.f;
}

void RenderSun()
{
    EnableAlphaBlend();
    vec3_t Angle;
    float Matrix[3][4];
    Angle[0] = 0.f;
    Angle[1] = 0.f;
    Angle[2] = CameraAngle[2];
    AngleIMatrix(Angle, Matrix);
    vec3_t p, Position;
    Vector(-900.f, CameraViewFar * 0.9f, 0.f, p);
    VectorRotate(p, Matrix, Position);
    VectorAdd(CameraPosition, Position, Sun.Position);
    Sun.Position[2] = 550.f;
    Sun.Visible = TestDepthBuffer(Sun.Position);
    BeginSprite();
    //RenderSprite(&Sun);
    EndSprite();
    DisableAlphaBlend();
}

void RenderSky()
{
    vec3_t Angle;
    float Matrix[3][4];
    Angle[0] = 0.f;
    Angle[1] = 0.f;
    Angle[2] = CameraAngle[2];
    AngleIMatrix(Angle, Matrix);
    float Aspect = (float)(WindowWidth) / (float)(WindowWidth);
    float Width = 1780.f * Aspect;

    BeginSprite();
    vec3_t p, Position;
    float Num = 20.f;
    vec3_t LightTable[21];

    for (int i = 0; i <= Num; i++)
    {
        Vector(((float)i - Num * 0.5f) * (Width / Num), CameraViewFar * 0.99f, 0.f, p);
        VectorRotate(p, Matrix, Position);
        VectorAdd(CameraPosition, Position, Position);
        RequestTerrainLight(Position[0], Position[1], LightTable[i]);
    }

    for (int i = 1; i <= (int)Num; i++)
    {
        if (LightTable[i][0] <= 0.f)
        {
            Vector(0.f, 0.f, 0.f, LightTable[i - 1]);
        }
    }

    for (int i = (int)Num - 1; i >= 0; i--)
    {
        if (LightTable[i][0] <= 0.f)
        {
            Vector(0.f, 0.f, 0.f, LightTable[i + 1]);
        }
    }
    for (float x = 0.f; x < Num; x += 1.f)
    {
        float UV[4][2];
        TEXCOORD(UV[0], (x) * (1.f / Num), 1.f);
        TEXCOORD(UV[1], (x + 1.f) * (1.f / Num), 1.f);
        TEXCOORD(UV[2], (x + 1.f) * (1.f / Num), 0.f);
        TEXCOORD(UV[3], (x) * (1.f / Num), 0.f);

        vec3_t Light[4];
        VectorCopy(LightTable[(int)x], Light[0]);
        VectorCopy(LightTable[(int)x + 1], Light[1]);
        //VectorCopy(LightTable[(int)x+1],Light[2]);
        //VectorCopy(LightTable[(int)x  ],Light[3]);
        Vector(1.f, 1.f, 1.f, Light[2]);
        Vector(1.f, 1.f, 1.f, Light[3]);

        Vector((x - Num * 0.5f + 0.5f) * (Width / Num), CameraViewFar * 0.9f, 0.f, p);
        VectorRotate(p, Matrix, Position);
        VectorAdd(CameraPosition, Position, Position);
        Position[2] = 400.f;
        //RenderSpriteUV(BITMAP_SKY,Position,Width/Num,Height,UV,Light);
    }
    EndSprite();
}