///////////////////////////////////////////////////////////////////////////////
// Terrain 관련 함수
// 뮤의 배경중에 Terrain 바닥을 처리하는 함수
// 배경 바닥 빛처리, 랜더링 등등
//
// *** 함수 레벨: 2
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
#include "./Utilities/Log/ErrorReport.h"
#include "CSChaosCastle.h"
#include "GMBattleCastle.h"
#include "CMVP1stDirection.h"
#include "CDirection.h"
#ifdef CSK_LH_DEBUG_CONSOLE
#include "./Utilities/Log/muConsoleDebug.h"
#endif // CSK_LH_DEBUG_CONSOLE
// 맵 관련 include
#ifdef PSW_ADD_MAPSYSTEM
#include "w_MapHeaders.h"
#endif // PSW_ADD_MAPSYSTEM
#ifdef PJH_NEW_SERVER_SELECT_MAP
#include "CameraMove.h"
#endif //PJH_NEW_SERVER_SELECT_MAP


//-------------------------------------------------------------------------------------------------------------

int  TerrainFlag;
bool ActiveTerrain      = false;
bool TerrainGrassEnable = true;
bool DetailLowEnable    = false;

char            LodBuffer[64*64];
vec3_t          TerrainNormal[TERRAIN_SIZE*TERRAIN_SIZE];
vec3_t          PrimaryTerrainLight[TERRAIN_SIZE*TERRAIN_SIZE];
vec3_t          BackTerrainLight[TERRAIN_SIZE*TERRAIN_SIZE];
vec3_t          TerrainLight[TERRAIN_SIZE*TERRAIN_SIZE];
float           PrimaryTerrainHeight[TERRAIN_SIZE*TERRAIN_SIZE];
float           BackTerrainHeight[TERRAIN_SIZE*TERRAIN_SIZE];
unsigned char   TerrainMappingLayer1[TERRAIN_SIZE*TERRAIN_SIZE];
unsigned char   TerrainMappingLayer2[TERRAIN_SIZE*TERRAIN_SIZE];
float           TerrainMappingAlpha[TERRAIN_SIZE*TERRAIN_SIZE];
float           TerrainGrassTexture[TERRAIN_SIZE*TERRAIN_SIZE];
float           TerrainGrassWind[TERRAIN_SIZE*TERRAIN_SIZE];
#ifdef ASG_ADD_MAP_KARUTAN
float			g_fTerrainGrassWind1[TERRAIN_SIZE*TERRAIN_SIZE];	// 특정맵에서 물 흐름 속도와 풀 흔들리는 속도를 다르게 쓰고 싶을 때 사용.
#endif	// ASG_ADD_MAP_KARUTAN

WORD            TerrainWall[TERRAIN_SIZE*TERRAIN_SIZE];

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

//////////////////////////////////////////////////////////////////////////
//  Extern.
//////////////////////////////////////////////////////////////////////////
extern  short   g_shCameraLevel;
extern  float CameraDistanceTarget;
extern  float CameraDistance;

//////////////////////////////////////////////////////////////////////////
//  Static Global Variable.
//////////////////////////////////////////////////////////////////////////
static  float   g_fFrustumRange = -40.f;


///////////////////////////////////////////////////////////////////////////////
// Util
///////////////////////////////////////////////////////////////////////////////

inline int TERRAIN_INDEX(int x,int y)
{
	return (y)*TERRAIN_SIZE+(x);
}

inline int TERRAIN_INDEX_REPEAT(int x,int y)
{
	return (y&TERRAIN_SIZE_MASK)*TERRAIN_SIZE+(x&TERRAIN_SIZE_MASK);
}

inline WORD TERRAIN_ATTRIBUTE(float x,float y)
{
    int xf = (int)(x/TERRAIN_SCALE);
    int yf = (int)(y/TERRAIN_SCALE);
    return TerrainWall[(yf)*TERRAIN_SIZE+(xf)];
}

void InitTerrainMappingLayer()
{
	for(int i=0;i<TERRAIN_SIZE*TERRAIN_SIZE;i++)
	{
		TerrainMappingLayer1[i] = 0;
		TerrainMappingLayer2[i] = 255;
		TerrainMappingAlpha[i] = 0.f;
        TerrainGrassTexture[i] = (float)((rand()%4)/4.f);
	}
}

void ExitProgram()
{
	MessageBox(g_hWnd,GlobalText[11],NULL,MB_OK);
	SendMessage(g_hWnd,WM_DESTROY,0,0);
}


static BYTE bBuxCode[3] = {0xfc,0xcf,0xab};

static void BuxConvert(BYTE *Buffer,int Size)
{
	for(int i=0;i<Size;i++)
		Buffer[i] ^= bBuxCode[i%3];
}

int OpenTerrainAttribute(char *FileName)
{
	// 암호화 이후

	FILE *fp = fopen(FileName,"rb");
	if(fp == NULL)
	{
		char Text[256];
   		sprintf(Text,"%s file not found.",FileName);
		g_ErrorReport.Write( Text);
		g_ErrorReport.Write( "\r\n");
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		return ( -1);
	}

	BYTE Version;
	BYTE Width;
	BYTE Height;

	// 읽고
	fseek(fp,0,SEEK_END);
	int EncBytes = ftell(fp);	//
	fseek(fp,0,SEEK_SET);
	unsigned char *EncData = new unsigned char [EncBytes];
	fread( EncData, EncBytes, 1, fp);

	// 암호화 풀기
	int iSize = MapFileDecrypt( NULL, EncData, EncBytes);	//
	unsigned char *byBuffer = new unsigned char[iSize];		//
	MapFileDecrypt( byBuffer, EncData, EncBytes);	//
	delete [] EncData;		//

    bool extAtt = false;

    if ( iSize!=131076 && iSize!=65540 )
	{
		delete [] byBuffer;
		return ( -1);
	}

    if ( iSize==131076 )
    {
        extAtt = true;
    }

	BuxConvert( byBuffer, iSize);
	Version = byBuffer[0];
	int iMap = byBuffer[1];
	Width = byBuffer[2];
	Height = byBuffer[3];

    if ( extAtt==false )
    {
        unsigned char TWall[TERRAIN_SIZE*TERRAIN_SIZE];
    	memcpy ( TWall, &byBuffer[4], TERRAIN_SIZE*TERRAIN_SIZE );

        for ( int i=0; i<TERRAIN_SIZE*TERRAIN_SIZE; ++i )
        {
            TerrainWall[i] = TWall[i];
        }
    }
    else
    {
	    memcpy ( TerrainWall, &byBuffer[4], TERRAIN_SIZE*TERRAIN_SIZE*sizeof(WORD) );
    }

	delete [] byBuffer;	//

	bool Error = false;
	if(Version!=0 || Width!=255 || Height!=255)
	{
		Error = true;
	}

	switch(World)
	{
	case WD_0LORENCIA:if(TerrainWall[123*256+135]!=5) Error=true;break;
	case WD_1DUNGEON:if(TerrainWall[120*256+227]!=4) Error=true;break;
	case WD_2DEVIAS:if(TerrainWall[ 55*256+208]!=5) Error=true;break;
	case WD_3NORIA:if(TerrainWall[119*256+186]!=5) Error=true;break;
	case WD_4LOSTTOWER:if(TerrainWall[ 75*256+193]!=5) Error=true;break;
	}
	
	for(int i=0;i<256*256;i++)
	{
#ifdef _VS2008PORTING
		WORD wWall = TerrainWall[i];
		//Wall = ( Wall ^ ( Wall & 8)) | ( (TerrainWall[i]&4) << 1);
		//Wall = ( Wall ^ ( Wall & 4)) | ( (TerrainWall[i]&8) >> 1);
		TerrainWall[i] = wWall;
		TerrainWall[i] = TerrainWall[i] & 0xFF;

		if((BYTE)TerrainWall[i] >= 128)
			Error = true;
#else // _VS2008PORTING
		BYTE Wall = TerrainWall[i];
		//Wall = ( Wall ^ ( Wall & 8)) | ( (TerrainWall[i]&4) << 1);
		//Wall = ( Wall ^ ( Wall & 4)) | ( (TerrainWall[i]&8) >> 1);
		TerrainWall[i] = Wall;

		if(TerrainWall[i] >= 128)
			Error = true;
#endif // _VS2008PORTING
	}
	if(Error)
	{
		ExitProgram();
		return ( -1);
	}

	fclose(fp);
    return iMap;
}

bool SaveTerrainAttribute(char *FileName, int iMap)
{
	FILE *fp = fopen(FileName,"wb");

	BYTE Version = 0;
	BYTE Width   = 255;
	BYTE Height  = 255;
 	fwrite(&Version,1,1,fp);
	fwrite(&iMap,1,1,fp);
 	fwrite(&Width,1,1,fp);
 	fwrite(&Height,1,1,fp);
 	fwrite(TerrainWall,TERRAIN_SIZE*TERRAIN_SIZE,1,fp);

	fclose(fp);
    return true;
}

//  속성 변경.
void AddTerrainAttribute ( int x, int y, BYTE att )
{
    int     iIndex = (x+(y*TERRAIN_SIZE));

    TerrainWall[iIndex] |= att;
}


void SubTerrainAttribute ( int x, int y, BYTE att )
{
    int     iIndex = (x+(y*TERRAIN_SIZE));

    TerrainWall[iIndex] ^= (TerrainWall[iIndex]&att);
}

void AddTerrainAttributeRange ( int x, int y, int dx, int dy, BYTE att, BYTE Add )
{
    for ( int j=0; j<dy; ++j )
    {
        for ( int i=0; i<dx; ++i )
        {
            if ( Add )             //  추가.
            {
                AddTerrainAttribute ( x+i, y+j, att );
            }
            else                    //  제거.
            {
                SubTerrainAttribute ( x+i, y+j, att );
            }
        }
    }
}

void SetTerrainWaterState( list<int>& terrainIndex, int state )
{
	if( state == 0 )
	{
		for ( int i = 0; i < TERRAIN_SIZE*TERRAIN_SIZE; ++i )
		{
			if( (TerrainWall[i]&TW_WATER)==TW_WATER )
			{
				TerrainWall[i] = 0;
				terrainIndex.push_back( i );
			}
		}
	}
	else
	{
		for ( list<int>::iterator iter = terrainIndex.begin(); iter != terrainIndex.end(); )
		{
			list<int>::iterator curiter = iter;
			++iter;
			int index = *curiter;
			TerrainWall[index] = TW_WATER;
		}
	}
}

int OpenTerrainMapping(char *FileName)	//
{
    InitTerrainMappingLayer();

    FILE *fp = fopen(FileName,"rb");
	if(fp == NULL)
	{
		return ( -1);
	}
	fseek(fp,0,SEEK_END);
	int EncBytes = ftell(fp);	//
	fseek(fp,0,SEEK_SET);
    unsigned char *EncData = new unsigned char[EncBytes];	//
	fread(EncData,1,EncBytes,fp);	//
	fclose(fp);

	// 암호화 된 것 풀기
	int DataBytes = MapFileDecrypt( NULL, EncData, EncBytes);	//
	unsigned char *Data = new unsigned char[DataBytes];		//
	MapFileDecrypt( Data, EncData, EncBytes);	//
	delete [] EncData;		//

	int DataPtr = 0;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	DataPtr+=1;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	BYTE Version = *((BYTE *)(Data+DataPtr));DataPtr+=1;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int iMapNumber = ( int)*((BYTE *)(Data+DataPtr));DataPtr+=1;		//
	memcpy(TerrainMappingLayer1,Data+DataPtr,256*256);DataPtr+=256*256;
	memcpy(TerrainMappingLayer2,Data+DataPtr,256*256);DataPtr+=256*256;
	for(int i=0;i<TERRAIN_SIZE*TERRAIN_SIZE;i++)
	{
		BYTE Alpha;
      	Alpha = *((BYTE *)(Data+DataPtr));DataPtr+=1;
		TerrainMappingAlpha[i] = (float)Alpha/255.f;
	}
	delete [] Data;
	
	fclose(fp);

    TerrainGrassEnable = true;

    if ( InChaosCastle()==true )
    {
        TerrainGrassEnable = false;
    }
    if ( battleCastle::InBattleCastle() )
    {
        TerrainGrassEnable = false;
    }

    return ( iMapNumber);
}

bool SaveTerrainMapping(char *FileName, int iMapNumber)	//
{
	FILE *fp = fopen(FileName,"wb");

	BYTE Version = 0;
 	fwrite(&Version,1,1,fp);
	fwrite(&iMapNumber,1,1,fp);	//
 	fwrite(TerrainMappingLayer1,TERRAIN_SIZE*TERRAIN_SIZE,1,fp);
 	fwrite(TerrainMappingLayer2,TERRAIN_SIZE*TERRAIN_SIZE,1,fp);
	for(int i=0;i<TERRAIN_SIZE*TERRAIN_SIZE;i++)
	{
		unsigned char Alpha = (unsigned char)(TerrainMappingAlpha[i]*255.f);
		fwrite(&Alpha,1,1,fp);
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

	// 암호화
	{
		fp = fopen(FileName,"rb");
		if(fp == NULL)
		{
			return ( false);
		}
		fseek(fp,0,SEEK_END);
		int EncBytes = ftell(fp);	//
		fseek(fp,0,SEEK_SET);
		unsigned char *EncData = new unsigned char[EncBytes];	//
		fread(EncData,1,EncBytes,fp);	//
		fclose(fp);

		// 암호화 시키기
		int DataBytes = MapFileEncrypt( NULL, EncData, EncBytes);	//
		unsigned char *Data = new unsigned char[DataBytes];		//
		MapFileEncrypt( Data, EncData, EncBytes);	//
		delete [] EncData;		//

		// 암호화 저장
		fp = fopen(FileName,"wb");
		fwrite( Data, DataBytes, 1, fp);
		fclose( fp);
		delete [] Data;
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
	for(int y=0;y<TERRAIN_SIZE;y++)
	{
		for(int x=0;x<TERRAIN_SIZE;x++)
		{
			int Index = TERRAIN_INDEX(x,y);
			vec3_t v1,v2,v3,v4;
			Vector((x  )*TERRAIN_SCALE,(y  )*TERRAIN_SCALE,BackTerrainHeight[TERRAIN_INDEX_REPEAT(x  ,y  )],v4);
			Vector((x+1)*TERRAIN_SCALE,(y  )*TERRAIN_SCALE,BackTerrainHeight[TERRAIN_INDEX_REPEAT(x+1,y  )],v1);
			Vector((x+1)*TERRAIN_SCALE,(y+1)*TERRAIN_SCALE,BackTerrainHeight[TERRAIN_INDEX_REPEAT(x+1,y+1)],v2);
			Vector((x  )*TERRAIN_SCALE,(y+1)*TERRAIN_SCALE,BackTerrainHeight[TERRAIN_INDEX_REPEAT(x  ,y+1)],v3);
			FaceNormalize(v1,v2,v3,TerrainNormal[Index]);
		}
	}
}

void CreateTerrainNormal_Part ( int xi, int yi )
{
    if ( xi>TERRAIN_SIZE-4 ) xi = TERRAIN_SIZE-4;
    else if ( xi<4 )         xi = 4;
    
    if ( yi>TERRAIN_SIZE-4 ) yi = TERRAIN_SIZE-4;
    else if ( yi<4 )         yi = 4;

	for(int y=yi-4;y<yi+4;y++)
	{
		for(int x=xi-4;x<xi+4;x++)
		{
			int Index = TERRAIN_INDEX(x,y);
			vec3_t v1,v2,v3,v4;
			Vector((x  )*TERRAIN_SCALE,(y  )*TERRAIN_SCALE,BackTerrainHeight[TERRAIN_INDEX_REPEAT(x  ,y  )],v4);
			Vector((x+1)*TERRAIN_SCALE,(y  )*TERRAIN_SCALE,BackTerrainHeight[TERRAIN_INDEX_REPEAT(x+1,y  )],v1);
			Vector((x+1)*TERRAIN_SCALE,(y+1)*TERRAIN_SCALE,BackTerrainHeight[TERRAIN_INDEX_REPEAT(x+1,y+1)],v2);
			Vector((x  )*TERRAIN_SCALE,(y+1)*TERRAIN_SCALE,BackTerrainHeight[TERRAIN_INDEX_REPEAT(x  ,y+1)],v3);
			FaceNormalize(v1,v2,v3,TerrainNormal[Index]);
		}
	}
}

void CreateTerrainLight()
{
	vec3_t Light;
    if ( battleCastle::InBattleCastle() )
    {
        Vector ( 0.5f, -1.f, 1.f, Light );
    }
    else
    {
	    Vector(0.5f,-0.5f,0.5f,Light);
    }
	for(int y=0;y<TERRAIN_SIZE;y++)
	{
		for(int x=0;x<TERRAIN_SIZE;x++)
		{
			int Index = TERRAIN_INDEX(x,y);
#ifdef ON_AIR_BLOODCASTLE
			float Luminosity = DotProduct(TerrainNormal[Index],Light) + 0.8f;
#else
			float Luminosity = DotProduct(TerrainNormal[Index],Light) + 0.5f;
#endif// ON_AIR_BLOODCASTLE
			if(Luminosity < 0.f) Luminosity = 0.f;
			else if(Luminosity > 1.f) Luminosity = 1.f;
			for(int i=0;i<3;i++)
    			BackTerrainLight[Index][i] = TerrainLight[Index][i] * Luminosity;
		}
	}
}


void CreateTerrainLight_Part ( int xi, int yi )
{
    if ( xi>TERRAIN_SIZE-4 ) xi = TERRAIN_SIZE-4;
    else if ( xi<4 )         xi = 4;
    
    if ( yi>TERRAIN_SIZE-4 ) yi = TERRAIN_SIZE-4;
    else if ( yi<4 )         yi = 4;

	vec3_t Light;
	Vector(0.5f,-0.5f,0.5f,Light);
	for(int y=yi-4;y<yi+4;y++)
	{
		for(int x=xi-4;x<xi+4;x++)
		{
			int Index = TERRAIN_INDEX(x,y);
			float Luminosity = DotProduct(TerrainNormal[Index],Light) + 0.5f;
			if(Luminosity < 0.f) Luminosity = 0.f;
			else if(Luminosity > 1.f) Luminosity = 1.f;
			for(int i=0;i<3;i++)
    			BackTerrainLight[Index][i] = TerrainLight[Index][i] * Luminosity;
		}
	}
}

void OpenTerrainLight(char *FileName)
{
    OpenJpegBuffer(FileName,&TerrainLight[0][0]);

	/*float Light;
	for(int i=0;i<256*256;i++)
	{
		Light = TerrainLight[i][2];
		Light -= 0.3f;
		if(Light < 0.f) Light = 0.f;
		TerrainLight[i][2] = Light;

		Light = TerrainLight[i][1];
		Light -= 0.2f;
		if(Light < 0.f) Light = 0.f;
		TerrainLight[i][1] = Light;
	}*/

	CreateTerrainNormal();
    CreateTerrainLight();

/*
#ifdef BATTLE_CASTLE
    if ( 1 )//battleCastle::InBattleCastle() )
    {
        g_fFrustumRange = -80.f;
    }
    else
#endif// BATTLE_CASTLE
    {
        g_fFrustumRange = -40.f;
    }
*/
}

void SaveTerrainLight(char *FileName)
{
	unsigned char *Buffer = new unsigned char [TERRAIN_SIZE*TERRAIN_SIZE*3];
	for(int i=0;i<TERRAIN_SIZE*TERRAIN_SIZE;i++)
	{
		for(int j=0;j<3;j++)
		{
			float Light = TerrainLight[i][j]*255.f;
			if(Light < 0.f) Light = 0.f;
			else if(Light > 255.f) Light = 255.f;
			Buffer[i*3+j] = (unsigned char)(Light);
		}
	}
	WriteJpeg(FileName,TERRAIN_SIZE,TERRAIN_SIZE,Buffer,100);
#ifdef KJH_FIX_ARRAY_DELETE
	SAFE_DELETE_ARRAY(Buffer);
#else // KJH_FIX_ARRAY_DELETE
	delete Buffer;
#endif // KJH_FIX_ARRAY_DELETE
}

void CreateTerrain(char *FileName, bool bNew)
{
	ActiveTerrain = true;

	if(bNew)
	{
		OpenTerrainHeightNew(FileName);
	}
	else
	{
		OpenTerrainHeight(FileName);
	}

	CreateSun();
}


///////////////////////////////////////////////////////////////////////////////
// Height 관련 함수
///////////////////////////////////////////////////////////////////////////////

unsigned char BMPHeader[1080];

bool IsTerrainHeightExtMap(int iWorld)
{
#ifdef PBG_ADD_PKFIELD
	if(iWorld == WD_42CHANGEUP3RD_2ND || IsPKField()
#ifdef YDG_ADD_MAP_DOPPELGANGER2
		|| iWorld == WD_66DOPPLEGANGER2
#endif	// YDG_ADD_MAP_DOPPELGANGER2
		)// 63 PK필드
#else //PBG_ADD_PKFIELD
	if(iWorld == WD_42CHANGEUP3RD_2ND)
#endif //PBG_ADD_PKFIELD
	{
		return true;
	}

	return false;
}

// 바닥 높이 데이타 로딩
bool OpenTerrainHeight(char *filename)
{
	char FileName[256];

	char NewFileName[256];
#ifdef _VS2008PORTING
	for(int i=0;i<(int)strlen(filename);i++)
	{
		NewFileName[i] = filename[i];
		NewFileName[i+1] = NULL;
		if(filename[i]=='.') 
			break;
	}
#else // _VS2008PORTING
	for(int i=0;i<(int)strlen(filename);i++)
	{
		NewFileName[i] = filename[i];
		if(filename[i]=='.') break;
	}
	NewFileName[i+1] = NULL;
#endif // _VS2008PORTING
	strcpy(FileName,"Data\\");
    strcat(FileName,NewFileName);
	strcat(FileName,"OZB");
	
    FILE *fp = fopen(FileName,"rb");
    if(fp == NULL)
	{
		char Text[256];
   		sprintf(Text,"%s file not found.",FileName);
		g_ErrorReport.Write( Text);
		g_ErrorReport.Write( "\r\n");
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		return false;
	}
	fseek(fp,4,SEEK_SET);
	int Index = 1080;
	int Size = 256*256+Index;
	unsigned char *Buffer = new unsigned char [Size];
   	fread(Buffer,1,Size,fp);
	fclose(fp);
	memcpy(BMPHeader,Buffer,Index);

#ifdef _VS2008PORTING
    for(int i=0;i<256;i++)
#else // _VS2008PORTING
    for(i=0;i<256;i++)
#endif // _VS2008PORTING
	{
		unsigned char *src = &Buffer[Index+i*256];
		float *dst = &BackTerrainHeight[i*256];
		for(int j=0;j<256;j++)
		{
			if(World == WD_55LOGINSCENE)
				*dst = (float)(*src)*3.0f;
			else
     			*dst = (float)(*src)*1.5f;
			src++;dst++;
		}
	}
#ifdef KJH_FIX_ARRAY_DELETE
	SAFE_DELETE_ARRAY(Buffer);
#else // KJH_FIX_ARRAY_DELETE
	delete Buffer;
#endif // KJH_FIX_ARRAY_DELETE
	return true;
}

// 바닥 높이 데이타 저장
void SaveTerrainHeight(char *name)
{
	unsigned char *Buffer = new unsigned char [256*256];
    for(int i=0;i<256;i++)
	{
		float *src = &BackTerrainHeight[i*256];
		unsigned char *dst = &Buffer[(255-i)*256];
		for(int j=0;j<256;j++)
		{
			float Height;
			if(World == WD_55LOGINSCENE)
				Height = *src/3.0f;
			else
				Height = *src/1.5f;
			if(Height < 0.f) Height = 0.f;
			else if(Height > 255.f) Height = 255.f;
			*dst = (unsigned char)(Height);
			src++;dst++;
		}
	}

    FILE *fp = fopen(name,"wb");
	fwrite(BMPHeader,1080,1,fp);
#ifdef _VS2008PORTING
	for(int i=0;i<256;i++) fwrite(Buffer+(255-i)*256,256,1,fp);
#else // _VS2008PORTING
	for(i=0;i<256;i++) fwrite(Buffer+(255-i)*256,256,1,fp);
#endif // _VS2008PORTING
#ifdef KJH_FIX_ARRAY_DELETE
	SAFE_DELETE_ARRAY(Buffer);
#else // KJH_FIX_ARRAY_DELETE
	delete Buffer;
#endif // KJH_FIX_ARRAY_DELETE
	fclose(fp);
}

bool OpenTerrainHeightNew(const char* strFilename)
{
	char FileName[256];
	char NewFileName[256];
#ifdef _VS2008PORTING
	for(int i=0;i<(int)strlen(strFilename);i++)
	{
		NewFileName[i] = strFilename[i];
		NewFileName[i+1] = NULL;
		if(strFilename[i]=='.') break;
	}
#else // _VS2008PORTING
	for(int i=0;i<(int)strlen(strFilename);i++)
	{
		NewFileName[i] = strFilename[i];
		if(strFilename[i]=='.') break;
	}

	NewFileName[i+1] = NULL;
#endif // _VS2008PORTING
	strcpy(FileName,"Data\\");
    strcat(FileName,NewFileName);
	strcat(FileName,"OZB");

	FILE* fp = fopen(FileName, "rb");
	fseek(fp, 0, SEEK_END);
	int iBytes = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	BYTE *pbyData = new BYTE[iBytes];
	fread(pbyData, 1, iBytes, fp);
	fclose(fp);

	DWORD dwCurPos = 0;
	dwCurPos += 4;

	BITMAPINFOHEADER bmiHeader;
    BITMAPFILEHEADER header;
	memcpy(&header, &pbyData[dwCurPos], sizeof(BITMAPFILEHEADER)); dwCurPos += sizeof(BITMAPFILEHEADER);
	memcpy(&bmiHeader, &pbyData[dwCurPos], sizeof(BITMAPINFOHEADER)); dwCurPos += sizeof(BITMAPINFOHEADER);

#ifdef _VS2008PORTING
    for(int i=0; i<TERRAIN_SIZE*TERRAIN_SIZE; ++i)
#else // _VS2008PORTING
    for(i=0; i<TERRAIN_SIZE*TERRAIN_SIZE; ++i)
#endif // _VS2008PORTING
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

extern int SceneFlag;

float RequestTerrainHeight(float xf,float yf)
{
	if ( SceneFlag == SERVER_LIST_SCENE || SceneFlag == WEBZEN_SCENE || SceneFlag == LOADING_SCENE)
		return 0.f;
    if ( xf<0.f || yf<0.f )
		return 0.f;

    xf = xf/TERRAIN_SCALE;
    yf = yf/TERRAIN_SCALE;

	unsigned int Index = TERRAIN_INDEX(xf,yf);

	//. 예외처리
	if(Index >= TERRAIN_SIZE*TERRAIN_SIZE)
		return g_fSpecialHeight;

    if ( (TerrainWall[Index]&TW_HEIGHT)==TW_HEIGHT )
    {
        return g_fSpecialHeight;
    }

    int xi = (int)xf;
    int yi = (int)yf;
    float xd = xf-(float)xi;
    float yd = yf-(float)yi;
    unsigned int Index1 = TERRAIN_INDEX_REPEAT(xi  ,yi  );
    unsigned int Index2 = TERRAIN_INDEX_REPEAT(xi  ,yi+1);
    unsigned int Index3 = TERRAIN_INDEX_REPEAT(xi+1,yi  );
    unsigned int Index4 = TERRAIN_INDEX_REPEAT(xi+1,yi+1);
	
	//. 예외처리
	if(Index1 >= TERRAIN_SIZE*TERRAIN_SIZE || Index2 >= TERRAIN_SIZE*TERRAIN_SIZE || 
		Index3 >= TERRAIN_SIZE*TERRAIN_SIZE || Index4 >= TERRAIN_SIZE*TERRAIN_SIZE)
		return g_fSpecialHeight;

    float left  = BackTerrainHeight[Index1]+(BackTerrainHeight[Index2]-BackTerrainHeight[Index1])*yd;
    float right = BackTerrainHeight[Index3]+(BackTerrainHeight[Index4]-BackTerrainHeight[Index3])*yd;
    return left+(right-left)*xd;
}

void RequestTerrainNormal(float xf,float yf,vec3_t Normal)
{
    xf = xf/TERRAIN_SCALE;
    yf = yf/TERRAIN_SCALE;
    int xi = (int)xf;
    int yi = (int)yf;
    VectorCopy(TerrainNormal[TERRAIN_INDEX_REPEAT(xi,yi)],Normal);
}

void AddTerrainHeight(float xf,float yf,float Height,int Range,float *Buffer)
{
	float rf = (float)Range;

    xf = xf/TERRAIN_SCALE;
    yf = yf/TERRAIN_SCALE;
    int   xi = (int)xf;
    int   yi = (int)yf;
	int   syi = yi-Range;
	int   eyi = yi+Range;
	float syf = (float)(syi);
    for(;syi<=eyi;syi++,syf+=1.f)
	{
		int   sxi = xi-Range;
		int   exi = xi+Range;
		float sxf = (float)(sxi);
        for(;sxi<=exi;sxi++,sxf+=1.f)
        {
			float xd = xf-sxf;
			float yd = yf-syf;
			float lf = (rf-sqrtf(xd*xd+yd*yd)) / rf;
			if(lf > 0.f)
			{
				Buffer[TERRAIN_INDEX_REPEAT(sxi,syi)] += Height * lf;
			}
        }
	}
}

///////////////////////////////////////////////////////////////////////////////
// Light 관련 함수
///////////////////////////////////////////////////////////////////////////////

void SetTerrainLight(float xf,float yf,vec3_t Light,int Range,vec3_t *Buffer)
{
	float rf = (float)Range;

    xf = (xf/TERRAIN_SCALE);
    yf = (yf/TERRAIN_SCALE);
    int   xi = (int)xf;
    int   yi = (int)yf;
	int   syi = yi-Range;
	int   eyi = yi+Range;
	float syf = (float)(syi);
    for(;syi<=eyi;syi++,syf+=1.f)
	{
		int   sxi = xi-Range;
		int   exi = xi+Range;
		float sxf = (float)(sxi);
        for(;sxi<=exi;sxi++,sxf+=1.f)
        {
			float xd = xf-sxf;
			float yd = yf-syf;
			float lf = (rf-sqrtf(xd*xd+yd*yd)) / rf;
			if(lf > 0.f)
			{
				float *b = &Buffer[TERRAIN_INDEX_REPEAT(sxi,syi)][0];
				for(int i=0;i<3;i++)
				{
     				b[i] += Light[i] * lf;
				}
			}
        }
	}
}

void AddTerrainLight(float xf,float yf,vec3_t Light,int Range,vec3_t *Buffer)
{
	float rf = (float)Range;

    xf = (xf/TERRAIN_SCALE);
    yf = (yf/TERRAIN_SCALE);
    int   xi = (int)xf;
    int   yi = (int)yf;
	int   syi = yi-Range;   //  y축 시작 좌표.
	int   eyi = yi+Range;   //  y축 끝 좌표.
	float syf = (float)(syi);
    for(;syi<=eyi;syi++,syf+=1.f)
	{
		int   sxi = xi-Range;   //  x축 시작 좌표.
		int   exi = xi+Range;   //  x축 끝 좌표.
		float sxf = (float)(sxi);
        for(;sxi<=exi;sxi++,sxf+=1.f)
        {
			float xd = xf-sxf;
			float yd = yf-syf;
			float lf = (rf-sqrtf(xd*xd+yd*yd)) / rf;
			if(lf > 0.f)
			{
				float *b = &Buffer[TERRAIN_INDEX_REPEAT(sxi,syi)][0];
				for(int i=0;i<3;i++)
				{
     				b[i] += Light[i] * lf;
#ifdef ON_AIR_BLOODCASTLE
                    b[i] *= 1.5f;
#endif// ON_AIR_BLOODCASTLE
					if ( b[i]<0.f ) b[i] = 0.f;
				}
			}
        }
	}
}

void AddTerrainLightClip(float xf,float yf,vec3_t Light,int Range,vec3_t *Buffer)
{
	float rf = (float)Range;

    xf = (xf/TERRAIN_SCALE);
    yf = (yf/TERRAIN_SCALE);
    int   xi = (int)xf;
    int   yi = (int)yf;
	int   syi = yi-Range;
	int   eyi = yi+Range;
	float syf = (float)(syi);
    for(;syi<=eyi;syi++,syf+=1.f)
	{
		int   sxi = xi-Range;
		int   exi = xi+Range;
		float sxf = (float)(sxi);
        for(;sxi<=exi;sxi++,sxf+=1.f)
        {
			float xd = xf-sxf;
			float yd = yf-syf;
			float lf = (rf-sqrtf(xd*xd+yd*yd)) / rf;
			if(lf > 0.f)
			{
				float *b = &Buffer[TERRAIN_INDEX_REPEAT(sxi,syi)][0];
				for(int i=0;i<3;i++)
				{
     				b[i] += Light[i] * lf;
					if(b[i] < 0.f) b[i] = 0.f;
					else if(b[i] > 1.f) b[i] = 1.f;
				}
			}
        }
	}
}

void RequestTerrainLight(float xf,float yf,vec3_t Light)
{
	if(SceneFlag == SERVER_LIST_SCENE 
		|| SceneFlag == WEBZEN_SCENE 
		|| SceneFlag == LOADING_SCENE 
		|| ActiveTerrain == false)
	{
		Vector(0.f,0.f,0.f,Light);
		return;
	}
	
    xf = xf/TERRAIN_SCALE;
    yf = yf/TERRAIN_SCALE;
    int xi = (int)xf;
    int yi = (int)yf;
	if(xi<0 || yi<0 || xi>TERRAIN_SIZE_MASK-1 || yi>TERRAIN_SIZE_MASK-1)
	{
		Vector(0.f,0.f,0.f,Light);
		return;
	}
    int Index1 = ((xi  )+(yi  )*TERRAIN_SIZE);
    int Index2 = ((xi+1)+(yi  )*TERRAIN_SIZE);
    int Index3 = ((xi+1)+(yi+1)*TERRAIN_SIZE);
    int Index4 = ((xi  )+(yi+1)*TERRAIN_SIZE);
    float xd = xf-(float)xi;
    float yd = yf-(float)yi;
	for(int i=0;i<3;i++)
	{
		float left  = PrimaryTerrainLight[Index1][i]+(PrimaryTerrainLight[Index4][i]-PrimaryTerrainLight[Index1][i])*yd;
		float right = PrimaryTerrainLight[Index2][i]+(PrimaryTerrainLight[Index3][i]-PrimaryTerrainLight[Index2][i])*yd;
		Light[i] = (left+(right-left)*xd);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Lod 관련 함수
///////////////////////////////////////////////////////////////////////////////

void CreateLodBuffer()
{
	for(int y=0;y<TERRAIN_SIZE;y+=4)
	{
		for(int x=0;x<TERRAIN_SIZE;x+=4)
		{
			vec3_t NormalMin,NormalMax;
			NormalMin[0] = 1.f;
			NormalMin[1] = 1.f;
			NormalMin[2] = 1.f;
			NormalMax[0] = -1.f;
			NormalMax[1] = -1.f;
			NormalMax[2] = -1.f;
			for(int i=0;i<4;i++)
			{
				for(int j=0;j<4;j++)
				{
					vec3_t v1,v2,v3,v4;
					Vector((x+j  )*TERRAIN_SCALE,(y+i  )*TERRAIN_SCALE, BackTerrainHeight[TERRAIN_INDEX_REPEAT(x+j  ,y+i  )], v1);
					Vector((x+j+1)*TERRAIN_SCALE,(y+i  )*TERRAIN_SCALE, BackTerrainHeight[TERRAIN_INDEX_REPEAT(x+j+1,y+i  )], v2);
					Vector((x+j+1)*TERRAIN_SCALE,(y+i+1)*TERRAIN_SCALE, BackTerrainHeight[TERRAIN_INDEX_REPEAT(x+j+1,y+i+1)], v3);
					Vector((x+j  )*TERRAIN_SCALE,(y+i+1)*TERRAIN_SCALE, BackTerrainHeight[TERRAIN_INDEX_REPEAT(x+j  ,y+i+1)], v4);

					int Index = TERRAIN_INDEX(x+j,y+i);
					FaceNormalize(v1,v2,v3,TerrainNormal[Index]);
                    
					vec3_t *Normal = &TerrainNormal[Index];
					if(NormalMin[0] > (*Normal)[0]) NormalMin[0] = (*Normal)[0];
					if(NormalMax[0] < (*Normal)[0]) NormalMax[0] = (*Normal)[0];
					if(NormalMin[1] > (*Normal)[1]) NormalMin[1] = (*Normal)[1];
					if(NormalMax[1] < (*Normal)[1]) NormalMax[1] = (*Normal)[1];
					if(NormalMin[2] > (*Normal)[2]) NormalMin[2] = (*Normal)[2];
					if(NormalMax[2] < (*Normal)[2]) NormalMax[2] = (*Normal)[2];
				}
			}
			float Delta = maxf(maxf(absf(NormalMax[0]-NormalMin[0]),absf(NormalMax[1]-NormalMin[1])),absf(NormalMax[2]-NormalMin[2]));
			if(DetailLowEnable == true)
			{
				LodBuffer[y/4*64+x/4] = 4;
			}
			else
			{
				if     (Delta >= 1.f ) LodBuffer[y/4*64+x/4] = 1;
				else if(Delta >= 0.5f) LodBuffer[y/4*64+x/4] = 2;
				else                   LodBuffer[y/4*64+x/4] = 4;
        		LodBuffer[y/4*64+x/4] = 1;
			}
		}
	}
}

void InterpolationHeight(int lod,int x,int y,int xd,float *TerrainHeight)
{
	if(lod >= 4)
	{
		TerrainHeight[TERRAIN_INDEX_REPEAT(x+xd,y+2)] = (
		TerrainHeight[TERRAIN_INDEX_REPEAT(x+xd,y  )]+
		TerrainHeight[TERRAIN_INDEX_REPEAT(x+xd,y+4)])*0.5f;
	}
	if(lod >= 2)
	{
		TerrainHeight[TERRAIN_INDEX_REPEAT(x+xd,y+1)] = (
		TerrainHeight[TERRAIN_INDEX_REPEAT(x+xd,y  )]+
		TerrainHeight[TERRAIN_INDEX_REPEAT(x+xd,y+2)])*0.5f;
		TerrainHeight[TERRAIN_INDEX_REPEAT(x+xd,y+3)] = (
		TerrainHeight[TERRAIN_INDEX_REPEAT(x+xd,y+2)]+
		TerrainHeight[TERRAIN_INDEX_REPEAT(x+xd,y+4)])*0.5f;
	}
}

void InterpolationWidth(int lod,int x,int y,int yd,float *TerrainHeight)
{
	if(lod >= 4)
	{
		TerrainHeight[TERRAIN_INDEX_REPEAT(x+2,y+yd)] = (
		TerrainHeight[TERRAIN_INDEX_REPEAT(x  ,y+yd)]+
		TerrainHeight[TERRAIN_INDEX_REPEAT(x+4,y+yd)])*0.5f;
	}
	if(lod >= 2)
	{
		TerrainHeight[TERRAIN_INDEX_REPEAT(x+1,y+yd)] = (
		TerrainHeight[TERRAIN_INDEX_REPEAT(x  ,y+yd)]+
		TerrainHeight[TERRAIN_INDEX_REPEAT(x+2,y+yd)])*0.5f;
		TerrainHeight[TERRAIN_INDEX_REPEAT(x+3,y+yd)] = (
		TerrainHeight[TERRAIN_INDEX_REPEAT(x+2,y+yd)]+
		TerrainHeight[TERRAIN_INDEX_REPEAT(x+4,y+yd)])*0.5f;
	}
}

void InterpolationCross(int lod,int x,int y)
{
	BackTerrainHeight[TERRAIN_INDEX_REPEAT(x    ,y    )] = (
	BackTerrainHeight[TERRAIN_INDEX_REPEAT(x-lod,y    )]+
	BackTerrainHeight[TERRAIN_INDEX_REPEAT(x+lod,y    )]+
	BackTerrainHeight[TERRAIN_INDEX_REPEAT(x    ,y-lod)]+
	BackTerrainHeight[TERRAIN_INDEX_REPEAT(x    ,y+lod)])*0.25f;
}

void PrefixTerrainHeightEdge(int x,int y,int lod,float *TerrainHeight)
{
	if(lod >= LodBuffer[((y  )&63)*64+((x-1)&63)]) InterpolationHeight(lod,x*4,y*4,0,TerrainHeight);
	if(lod >= LodBuffer[((y  )&63)*64+((x+1)&63)]) InterpolationHeight(lod,x*4,y*4,4,TerrainHeight);
	if(lod >= LodBuffer[((y-1)&63)*64+((x  )&63)]) InterpolationWidth(lod,x*4,y*4,0,TerrainHeight);
	if(lod >= LodBuffer[((y+1)&63)*64+((x  )&63)]) InterpolationWidth(lod,x*4,y*4,4,TerrainHeight);
}

void PrefixTerrainHeight()
{
	for(int y=0;y<64;y++)
	{
		for(int x=0;x<64;x++)
		{
			int lod = LodBuffer[((y)&63)*64+((x)&63)];
			PrefixTerrainHeightEdge(x,y,lod,BackTerrainHeight);
			if(lod >= 2)
			{
				if(lod >= 4)
				{
					InterpolationHeight(lod,x*4,y*4,2,BackTerrainHeight);
					InterpolationWidth(lod,x*4,y*4,2,BackTerrainHeight);
				}
				if(lod >= 2)
				{
					InterpolationHeight(lod,x*4,y*4,1,BackTerrainHeight);
					InterpolationWidth(lod,x*4,y*4,1,BackTerrainHeight);
					InterpolationHeight(lod,x*4,y*4,3,BackTerrainHeight);
					InterpolationWidth(lod,x*4,y*4,3,BackTerrainHeight);
					/*InterpolationCross(1,x*4+1,y*4+1);
					InterpolationCross(1,x*4+1,y*4+3);
					InterpolationCross(1,x*4+3,y*4+1);
					InterpolationCross(1,x*4+3,y*4+3);*/
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// Rendering 관련 함수
///////////////////////////////////////////////////////////////////////////////

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

inline void Interpolation(int mx,int my)
{
	Index01 = (my*2  )*512+(mx*2+1);
	Index12 = (my*2+1)*512+(mx*2+2);
	Index23 = (my*2+2)*512+(mx*2+1);
	Index30 = (my*2+1)*512+(mx*2  );
	Index02 = (my*2+1)*512+(mx*2+1);
	for(int i=0;i<3;i++)
	{
		TerrainVertex01[i] = (TerrainVertex[0][i]+TerrainVertex[1][i])*0.5f;
		TerrainVertex12[i] = (TerrainVertex[1][i]+TerrainVertex[2][i])*0.5f;
		TerrainVertex23[i] = (TerrainVertex[2][i]+TerrainVertex[3][i])*0.5f;
		TerrainVertex30[i] = (TerrainVertex[3][i]+TerrainVertex[0][i])*0.5f;
		TerrainVertex02[i] = (TerrainVertex[0][i]+TerrainVertex[2][i])*0.5f;
	}
#ifdef _VS2008PORTING
	for(int i=0;i<2;i++)
#else // _VS2008PORTING
	for(i=0;i<2;i++)
#endif // _VS2008PORTING
	{
		TerrainTextureCoord01[i] = (TerrainTextureCoord[0][i]+TerrainTextureCoord[1][i])*0.5f;
		TerrainTextureCoord12[i] = (TerrainTextureCoord[1][i]+TerrainTextureCoord[2][i])*0.5f;
		TerrainTextureCoord23[i] = (TerrainTextureCoord[2][i]+TerrainTextureCoord[3][i])*0.5f;
		TerrainTextureCoord30[i] = (TerrainTextureCoord[3][i]+TerrainTextureCoord[0][i])*0.5f;
		TerrainTextureCoord02[i] = (TerrainTextureCoord[0][i]+TerrainTextureCoord[2][i])*0.5f;
	}
}

inline void Vertex0()
{
	glTexCoord2f(TerrainTextureCoord[0][0],TerrainTextureCoord[0][1]);
	glColor3fv(PrimaryTerrainLight[TerrainIndex1]);
	glVertex3fv(TerrainVertex[0]);
}

inline void Vertex1()
{
	glTexCoord2f(TerrainTextureCoord[1][0],TerrainTextureCoord[1][1]);
	glColor3fv(PrimaryTerrainLight[TerrainIndex2]);
	glVertex3fv(TerrainVertex[1]);
}

inline void Vertex2()
{
	glTexCoord2f(TerrainTextureCoord[2][0],TerrainTextureCoord[2][1]);
	glColor3fv(PrimaryTerrainLight[TerrainIndex3]);
	glVertex3fv(TerrainVertex[2]);
}

inline void Vertex3()
{
	glTexCoord2f(TerrainTextureCoord[3][0],TerrainTextureCoord[3][1]);
	glColor3fv(PrimaryTerrainLight[TerrainIndex4]);
	glVertex3fv(TerrainVertex[3]);
}

inline void Vertex01()
{
	glTexCoord2f(TerrainTextureCoord01[0],TerrainTextureCoord01[1]);
	glColor3fv(PrimaryTerrainLight[Index01]);
	glVertex3fv(TerrainVertex01);
}

inline void Vertex12()
{
	glTexCoord2f(TerrainTextureCoord12[0],TerrainTextureCoord12[1]);
	glColor3fv(PrimaryTerrainLight[Index12]);
	glVertex3fv(TerrainVertex12);
}

inline void Vertex23()
{
	glTexCoord2f(TerrainTextureCoord23[0],TerrainTextureCoord23[1]);
	glColor3fv(PrimaryTerrainLight[Index23]);
	glVertex3fv(TerrainVertex23);
}

inline void Vertex30()
{
	glTexCoord2f(TerrainTextureCoord30[0],TerrainTextureCoord30[1]);
	glColor3fv(PrimaryTerrainLight[Index30]);
	glVertex3fv(TerrainVertex30);
}

inline void Vertex02()
{
	glTexCoord2f(TerrainTextureCoord02[0],TerrainTextureCoord02[1]);
	glColor3fv(PrimaryTerrainLight[Index02]);
	glVertex3fv(TerrainVertex02);
}
				
inline void VertexAlpha0()
{
	glTexCoord2f(TerrainTextureCoord[0][0],TerrainTextureCoord[0][1]);
    float *Light = &PrimaryTerrainLight[TerrainIndex1][0];
	glColor4f(Light[0],Light[1],Light[2],TerrainMappingAlpha[TerrainIndex1]);
	glVertex3fv(TerrainVertex[0]);
}

inline void VertexAlpha1()
{
	glTexCoord2f(TerrainTextureCoord[1][0],TerrainTextureCoord[1][1]);
    float *Light = &PrimaryTerrainLight[TerrainIndex2][0];
	glColor4f(Light[0],Light[1],Light[2],TerrainMappingAlpha[TerrainIndex2]);
	glVertex3fv(TerrainVertex[1]);
}

inline void VertexAlpha2()
{
	glTexCoord2f(TerrainTextureCoord[2][0],TerrainTextureCoord[2][1]);
    float *Light = &PrimaryTerrainLight[TerrainIndex3][0];
	glColor4f(Light[0],Light[1],Light[2],TerrainMappingAlpha[TerrainIndex3]);
	glVertex3fv(TerrainVertex[2]);
}

inline void VertexAlpha3()
{
	glTexCoord2f(TerrainTextureCoord[3][0],TerrainTextureCoord[3][1]);
    float *Light = &PrimaryTerrainLight[TerrainIndex4][0];
	glColor4f(Light[0],Light[1],Light[2],TerrainMappingAlpha[TerrainIndex4]);
	glVertex3fv(TerrainVertex[3]);
}

inline void VertexAlpha01()
{
	glTexCoord2f(TerrainTextureCoord01[0],TerrainTextureCoord01[1]);
    float *Light = &PrimaryTerrainLight[Index01][0];
	glColor4f(Light[0],Light[1],Light[2],TerrainMappingAlpha01);
	glVertex3fv(TerrainVertex01);
}

inline void VertexAlpha12()
{
	glTexCoord2f(TerrainTextureCoord12[0],TerrainTextureCoord12[1]);
    float *Light = &PrimaryTerrainLight[Index12][0];
	glColor4f(Light[0],Light[1],Light[2],TerrainMappingAlpha12);
	glVertex3fv(TerrainVertex12);
}

inline void VertexAlpha23()
{
	glTexCoord2f(TerrainTextureCoord23[0],TerrainTextureCoord23[1]);
    float *Light = &PrimaryTerrainLight[Index23][0];
	glColor4f(Light[0],Light[1],Light[2],TerrainMappingAlpha23);
	glVertex3fv(TerrainVertex23);
}

inline void VertexAlpha30()
{
	glTexCoord2f(TerrainTextureCoord30[0],TerrainTextureCoord30[1]);
    float *Light = &PrimaryTerrainLight[Index30][0];
	glColor4f(Light[0],Light[1],Light[2],TerrainMappingAlpha30);
	glVertex3fv(TerrainVertex30);
}

inline void VertexAlpha02()
{
	glTexCoord2f(TerrainTextureCoord02[0],TerrainTextureCoord02[1]);
    float *Light = &PrimaryTerrainLight[Index02][0];
	glColor4f(Light[0],Light[1],Light[2],TerrainMappingAlpha02);
	glVertex3fv(TerrainVertex02);
}
				
inline void VertexBlend0()
{
	glTexCoord2f(TerrainTextureCoord[0][0],TerrainTextureCoord[0][1]);
    float Light = TerrainMappingAlpha[TerrainIndex1];
	glColor3f(Light,Light,Light);
	glVertex3fv(TerrainVertex[0]);
}

inline void VertexBlend1()
{
	glTexCoord2f(TerrainTextureCoord[1][0],TerrainTextureCoord[1][1]);
    float Light = TerrainMappingAlpha[TerrainIndex2];
	glColor3f(Light,Light,Light);
	glVertex3fv(TerrainVertex[1]);
}

inline void VertexBlend2()
{
	glTexCoord2f(TerrainTextureCoord[2][0],TerrainTextureCoord[2][1]);
    float Light = TerrainMappingAlpha[TerrainIndex3];
	glColor3f(Light,Light,Light);
	glVertex3fv(TerrainVertex[2]);
}

inline void VertexBlend3()
{
	glTexCoord2f(TerrainTextureCoord[3][0],TerrainTextureCoord[3][1]);
    float Light = TerrainMappingAlpha[TerrainIndex4];
	glColor3f(Light,Light,Light);
	glVertex3fv(TerrainVertex[3]);
}

void RenderFace(int Texture,int mx,int my)
{
	if(World == WD_39KANTURU_3RD)
	{
		if(Texture == 3)
			EnableAlphaTest();
		else if(Texture == 100)	// 나중에 그리고 싶은 텍스쳐 번호를 지정해 준다
			return;
		else
		  	DisableAlphaBlend();
	}
	else if( World >= WD_45CURSEDTEMPLE_LV1 && World <= WD_45CURSEDTEMPLE_LV6 )
	{
		if(Texture == 4)
		{
			EnableAlphaTest();
		}
		else
		{
			DisableAlphaBlend();
		}
	}
	else if(World == WD_51HOME_6TH_CHAR 
#ifndef PJH_NEW_SERVER_SELECT_MAP
		|| World == WD_77NEW_LOGIN_SCENE 
		|| World == WD_78NEW_CHARACTER_SCENE
#endif //PJH_NEW_SERVER_SELECT_MAP
		)
	{
		if(Texture == 2)
		{
			EnableAlphaTest();
		}
		else
		{
		  	DisableAlphaBlend();
		}
	}
#ifdef LDS_ADD_EMPIRE_GUARDIAN
	else if( World == WD_69EMPIREGUARDIAN1 || World == WD_70EMPIREGUARDIAN2 || World == WD_71EMPIREGUARDIAN3 || World == WD_72EMPIREGUARDIAN4
#ifdef PJH_NEW_SERVER_SELECT_MAP
			|| World == WD_73NEW_LOGIN_SCENE || World == WD_74NEW_CHARACTER_SCENE 
#endif //PJH_NEW_SERVER_SELECT_MAP
		)
	{
		if(Texture == 10)	// 에디트일때는 투명타일 표시
		{
			EnableAlphaTest();
		}
		else
		{
			DisableAlphaBlend();
		}
	}
#endif // LDS_ADD_EMPIRE_GUARDIAN
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
	BindTexture(BITMAP_MAPTILE+Texture);

	glBegin(GL_TRIANGLE_FAN);
	Vertex0();
	Vertex1();
	Vertex2();
	Vertex3();
	glEnd();
}

void RenderFace_After(int Texture, int mx, int my)
{
	if(Texture == 100)			// Tga 파일 투명 일 때
		EnableAlphaTest();
	else if( Texture == 101)	// Jpg 파일 투명 일 때
		EnableAlphaBlend();
	else
		return;
	
	BindTexture(BITMAP_MAPTILE+Texture);

	glBegin(GL_TRIANGLE_FAN);
		Vertex0();
		Vertex1();
		Vertex2();
		Vertex3();
	glEnd();
}

void RenderFaceAlpha(int Texture,int mx,int my)
{
    EnableAlphaTest();
	BindTexture(BITMAP_MAPTILE+Texture);
	glBegin(GL_TRIANGLE_FAN);
	VertexAlpha0();
	VertexAlpha1();
	VertexAlpha2();
	VertexAlpha3();
	glEnd();
}

void RenderFaceBlend(int Texture,int mx,int my)
{
	EnableAlphaBlend();
	BindTexture(BITMAP_MAPTILE+Texture);
	glBegin(GL_TRIANGLE_FAN);
	VertexBlend0();
	VertexBlend1();
	VertexBlend2();
	VertexBlend3();
	glEnd();
}

void FaceTexture(int Texture,float xf,float yf,bool Water,bool Scale)
{
	vec3_t Light,Pos;
	Vector(0.30f,0.40f,0.20f,Light);
	BITMAP_t *b = &Bitmaps[BITMAP_MAPTILE+Texture];
	float Width,Height;
	if(Scale)
	{
		Width  = 16.f/b->Width;
		Height = 16.f/b->Height;
	}
	else
	{
		Width  = 64.f/b->Width;
		Height = 64.f/b->Height;
	}
	float suf = xf*Width;
	float svf = yf*Height;
	if(!Water)
	{
		TEXCOORD(TerrainTextureCoord[0],suf      ,svf       );
		TEXCOORD(TerrainTextureCoord[1],suf+Width,svf       );
		TEXCOORD(TerrainTextureCoord[2],suf+Width,svf+Height);
		TEXCOORD(TerrainTextureCoord[3],suf      ,svf+Height);
	}
	else
	{
		float Water1 = 0.f;
		float Water2 = 0.f;
		float Water3 = 0.f;
		float Water4 = 0.f;
		if(World == WD_34CRYWOLF_1ST && Texture == 5)
		{
			if(rand()%50 == 0)
			{
				float sx = xf*TERRAIN_SCALE + (float)((rand()%100+1)*1.0f);
				float sy = yf*TERRAIN_SCALE + (float)((rand()%100+1)*1.0f);
				Vector(sx,sy,Hero->Object.Position[2]+10.f,Pos);
				CreateParticle(BITMAP_SPOT_WATER,Pos,Hero->Object.Angle, Light,0);
			}
		}

		if(World == WD_30BATTLECASTLE && Texture == 5)
			suf -= WaterMove;
		else
    		suf += WaterMove;
		
		if(Scale)
		{
			Water3 = TerrainGrassWind[TerrainIndex1]*0.008f;
			Water4 = TerrainGrassWind[TerrainIndex2]*0.008f;
		}
		else
		{
			Water3 = TerrainGrassWind[TerrainIndex1]*0.002f;
			Water4 = TerrainGrassWind[TerrainIndex2]*0.002f;
		}
		
		TEXCOORD(TerrainTextureCoord[0],suf      +Water1,svf       +Water3);
		TEXCOORD(TerrainTextureCoord[1],suf+Width+Water2,svf       +Water4);
		TEXCOORD(TerrainTextureCoord[2],suf+Width+Water2,svf+Height+Water4);
		TEXCOORD(TerrainTextureCoord[3],suf      +Water1,svf+Height+Water3);
	}
}

int WaterTextureNumber = 0;

void RenderTerrainFace(float xf,float yf,int xi,int yi,float lodf)
{
    RenderTerrainVisual ( xi, yi );

	if ( TerrainFlag != TERRAIN_MAP_GRASS )
	{
		int Texture;
		bool Alpha;
		bool Water = false;
		if(TerrainMappingAlpha[TerrainIndex1]>=1.f && TerrainMappingAlpha[TerrainIndex2]>=1.f && TerrainMappingAlpha[TerrainIndex3]>=1.f && TerrainMappingAlpha[TerrainIndex4]>=1.f)
		{
      		Texture = TerrainMappingLayer2[TerrainIndex1];
			Alpha = false;
		}
		else
		{
      		Texture = TerrainMappingLayer1[TerrainIndex1];
			Alpha = true;
			// BITMAP_MAPTILE+5 면 물지면 텍스쳐이다.
			if ( Texture == 5)
			{
				Water = true;
			}
#ifdef PBG_ADD_PKFIELD
			if(Texture == 11
				&& (IsPKField()
#ifdef YDG_ADD_MAP_DOPPELGANGER2
				|| IsDoppelGanger2()
#endif	// YDG_ADD_MAP_DOPPELGANGER2
				))
				Water = true;
#endif //PBG_ADD_PKFIELD
		}
		FaceTexture(Texture,xf,yf,Water,false);
    	RenderFace(Texture,xi,yi);

        if ( TerrainMappingAlpha[TerrainIndex1] > 0.f 
			|| TerrainMappingAlpha[TerrainIndex2] > 0.f 
			|| TerrainMappingAlpha[TerrainIndex3] > 0.f 
			|| TerrainMappingAlpha[TerrainIndex4] > 0.f )
        {
    		if ( (World == WD_7ATLANSE
#ifdef YDG_ADD_MAP_DOPPELGANGER3
				|| IsDoppelGanger3()
#endif	// YDG_ADD_MAP_DOPPELGANGER3
				) && TerrainMappingLayer2[TerrainIndex1] == 5 )
            {
			    Texture = BITMAP_WATER-BITMAP_MAPTILE+WaterTextureNumber;
			    FaceTexture(Texture,xf,yf,false,true);
			    RenderFaceBlend(Texture,xi,yi);
            }
            else if ( Alpha )
            {
			    Texture = TerrainMappingLayer2[TerrainIndex1];
			    if(Texture != 5)
				    Water = false;
				if (Texture != 255)
				{
    				FaceTexture(Texture,xf,yf,Water,false);
					RenderFaceAlpha(Texture,xi,yi);
				}
            }
        }
	}
	else
	{
  	    if ( TerrainMappingAlpha[TerrainIndex1]>0.f || TerrainMappingAlpha[TerrainIndex2]>0.f || TerrainMappingAlpha[TerrainIndex3]>0.f || TerrainMappingAlpha[TerrainIndex4]>0.f ) 
		{
			return;
		}
 		if ( 
              CurrentLayer == 0 && ( InBloodCastle() == false )
           )
		{
			int Texture = BITMAP_MAPGRASS+TerrainMappingLayer1[TerrainIndex1];
			
			BITMAP_t* pBitmap = Bitmaps.FindTexture(Texture);
			if(pBitmap)
			{
				float Height = pBitmap->Height * 2.f;
				BindTexture(Texture);
#ifdef PBG_ADD_PKFIELD
				if(IsPKField()
#ifdef YDG_ADD_MAP_DOPPELGANGER2
					|| IsDoppelGanger2()
#endif	// YDG_ADD_MAP_DOPPELGANGER2
					)
					EnableAlphaBlend();
#endif //PBG_ADD_PKFIELD
           		float Width  = 64.f/256.f;
         		float su = xf*Width;
				su += TerrainGrassTexture[yi&TERRAIN_SIZE_MASK];
				TEXCOORD(TerrainTextureCoord[0],su      , 0.f);
				TEXCOORD(TerrainTextureCoord[1],su+Width, 0.f);
				TEXCOORD(TerrainTextureCoord[2],su+Width, 1.f);
				TEXCOORD(TerrainTextureCoord[3],su      , 1.f);
				VectorCopy(TerrainVertex[0],TerrainVertex[3]);
				VectorCopy(TerrainVertex[2],TerrainVertex[1]);
				TerrainVertex[0][2] += Height;
				TerrainVertex[1][2] += Height;
				TerrainVertex[0][0] += -50.f;
				TerrainVertex[1][0] += -50.f;
#ifdef ASG_ADD_MAP_KARUTAN
				// 칼루탄맵에서는 풀이 흔들리는 정도가 다름. g_fTerrainGrassWind1 값을 따로 사용.
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
				glTexCoord2f(TerrainTextureCoord[0][0],TerrainTextureCoord[0][1]);
				glColor3fv(PrimaryTerrainLight[TerrainIndex1]);
				glVertex3fv(TerrainVertex[0]);
				glTexCoord2f(TerrainTextureCoord[1][0],TerrainTextureCoord[1][1]);
				glColor3fv(PrimaryTerrainLight[TerrainIndex2]);
				glVertex3fv(TerrainVertex[1]);
				glTexCoord2f(TerrainTextureCoord[2][0],TerrainTextureCoord[2][1]);
				glColor3fv(PrimaryTerrainLight[TerrainIndex3]);
				glVertex3fv(TerrainVertex[2]);
				glTexCoord2f(TerrainTextureCoord[3][0],TerrainTextureCoord[3][1]);
				glColor3fv(PrimaryTerrainLight[TerrainIndex4]);
				glVertex3fv(TerrainVertex[3]);
				glEnd();
#ifdef PBG_ADD_PKFIELD
				if(IsPKField()
#ifdef YDG_ADD_MAP_DOPPELGANGER2
					|| IsDoppelGanger2()
#endif	// YDG_ADD_MAP_DOPPELGANGER2
					)
					DisableAlphaBlend();
#endif //PBG_ADD_PKFIELD

			}
		}
	}
}

void RenderTerrainFace_After(float xf,float yf,int xi,int yi,float lodf)
{
	if(TerrainFlag != TERRAIN_MAP_GRASS)
	{
		int Texture;
		int Water = 0;
		if(TerrainMappingAlpha[TerrainIndex1]>=1.f && TerrainMappingAlpha[TerrainIndex2]>=1.f && TerrainMappingAlpha[TerrainIndex3]>=1.f && TerrainMappingAlpha[TerrainIndex4]>=1.f)
		{
      		Texture = TerrainMappingLayer2[TerrainIndex1];
		}
		else
		{
      		Texture = TerrainMappingLayer1[TerrainIndex1];
			if(TerrainMappingLayer1[TerrainIndex1]==5)
				Water = 1;   //  물 흐르기.  왼쪽으로.
		}
		
		FaceTexture(Texture,xf,yf,Water,false);
    	RenderFace_After(Texture,xi,yi);
	}
}

#ifdef SHOW_PATH_INFO
extern PATH* path;
#endif // SHOW_PATH_INFO

extern int SelectWall;

bool RenderTerrainTile(float xf,float yf,int xi,int yi,float lodf,int lodi,bool Flag)
{
	TerrainIndex1 = TERRAIN_INDEX(xi     ,yi     );
    if ( (TerrainWall[TerrainIndex1]&TW_NOGROUND)==TW_NOGROUND && !Flag ) return false;
	
    TerrainIndex2 = TERRAIN_INDEX(xi+lodi,yi     );
	TerrainIndex3 = TERRAIN_INDEX(xi+lodi,yi+lodi);
	TerrainIndex4 = TERRAIN_INDEX(xi     ,yi+lodi);

    float sx = xf*TERRAIN_SCALE;
    float sy = yf*TERRAIN_SCALE;

    Vector(sx              ,sy              ,BackTerrainHeight[TerrainIndex1],TerrainVertex[0]);
	Vector(sx+TERRAIN_SCALE,sy              ,BackTerrainHeight[TerrainIndex2],TerrainVertex[1]);
	Vector(sx+TERRAIN_SCALE,sy+TERRAIN_SCALE,BackTerrainHeight[TerrainIndex3],TerrainVertex[2]);
	Vector(sx              ,sy+TERRAIN_SCALE,BackTerrainHeight[TerrainIndex4],TerrainVertex[3]);

    if ( (TerrainWall[TerrainIndex1]&TW_HEIGHT)==TW_HEIGHT ) TerrainVertex[0][2] = g_fSpecialHeight;
    if ( (TerrainWall[TerrainIndex2]&TW_HEIGHT)==TW_HEIGHT ) TerrainVertex[1][2] = g_fSpecialHeight;
    if ( (TerrainWall[TerrainIndex3]&TW_HEIGHT)==TW_HEIGHT ) TerrainVertex[2][2] = g_fSpecialHeight;
    if ( (TerrainWall[TerrainIndex4]&TW_HEIGHT)==TW_HEIGHT ) TerrainVertex[3][2] = g_fSpecialHeight;

	if(!Flag)
	{
		RenderTerrainFace(xf,yf,xi,yi,lodf);
		// 길찾기 정보 표시
#ifdef SHOW_PATH_INFO
#ifdef CSK_DEBUG_MAP_PATHFINDING
		if(g_bShowPath == true)
#endif // CSK_DEBUG_MAP_PATHFINDING
		{
			if ( 2 <= path->GetClosedStatus( TerrainIndex1))
			{
				EnableAlphaTest();
				DisableTexture();
				glBegin(GL_TRIANGLE_FAN);
				if ( 4 <= path->GetClosedStatus( TerrainIndex1))
				{
					glColor4f(0.3f,0.3f,1.0f,0.5f);
				}
				else
				{
					glColor4f(1.0f,1.0f,1.0f,0.3f);
				}
				for(int i=0;i<4;i++)
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
        if(EditFlag!=EDIT_LIGHT)
		{
			DisableTexture();
			glColor3f(0.5f,0.5f,0.5f);
			glBegin(GL_LINE_STRIP);
			for(int i=0;i<4;i++)
			{
				glVertex3fv(TerrainVertex[i]);
			}
			glEnd();
			DisableAlphaBlend();
		}
#endif// _DEBUG
		
		vec3_t Normal;
        //  현재 타일의 노멜벡터값을 구한다.
		FaceNormalize ( TerrainVertex[0], TerrainVertex[1], TerrainVertex[2], Normal );
		bool Success = CollisionDetectLineToFace ( MousePosition, MouseTarget, 3, TerrainVertex[0], TerrainVertex[1], TerrainVertex[2], TerrainVertex[3], Normal );
		if ( Success==false )
		{
			FaceNormalize ( TerrainVertex[0], TerrainVertex[2], TerrainVertex[3], Normal );
			Success = CollisionDetectLineToFace ( MousePosition, MouseTarget, 3, TerrainVertex[0], TerrainVertex[2], TerrainVertex[3], TerrainVertex[1], Normal );
		}
		if ( Success==true )
		{
			SelectFlag = true;
			SelectXF = xf;
			SelectYF = yf;
		}
#ifdef CSK_DEBUG_MAP_ATTRIBUTE
		if ( EditFlag == EDIT_WALL && 
            ( ( SelectWall==0 && (TerrainWall[TerrainIndex1]&TW_NOMOVE)==TW_NOMOVE) 
           || ( SelectWall==2 && (TerrainWall[TerrainIndex1]&TW_SAFEZONE)==TW_SAFEZONE)
           || ( SelectWall==6 && (TerrainWall[TerrainIndex1]&TW_CAMERA_UP)==TW_CAMERA_UP)
           || ( SelectWall==7 && (TerrainWall[TerrainIndex1]&TW_NOATTACKZONE)==TW_NOATTACKZONE)
           || ( SelectWall==8 && (TerrainWall[TerrainIndex1]&TW_ATT1)==TW_ATT1)
           || ( SelectWall==9 && (TerrainWall[TerrainIndex1]&TW_ATT2)==TW_ATT2)
           || ( SelectWall==10&& (TerrainWall[TerrainIndex1]&TW_ATT3)==TW_ATT3)
           || ( SelectWall==11&& (TerrainWall[TerrainIndex1]&TW_ATT4)==TW_ATT4)
           || ( SelectWall==12&& (TerrainWall[TerrainIndex1]&TW_ATT5)==TW_ATT5)
           || ( SelectWall==13&& (TerrainWall[TerrainIndex1]&TW_ATT6)==TW_ATT6)
           || ( SelectWall==14&& (TerrainWall[TerrainIndex1]&TW_ATT7)==TW_ATT7)
           ) )
		{
			DisableDepthTest();
			EnableAlphaTest();
			DisableTexture();
     		
			glBegin(GL_TRIANGLE_FAN);
			glColor4f(1.f,0.5f,0.5f,0.3f);
			for(int i=0;i<4;i++)
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

void RenderTerrainTile_After(float xf,float yf,int xi,int yi,float lodf,int lodi,bool Flag)
{
	TerrainIndex1 = TERRAIN_INDEX(xi     ,yi     );
	TerrainIndex2 = TERRAIN_INDEX(xi+lodi,yi     );
	TerrainIndex3 = TERRAIN_INDEX(xi+lodi,yi+lodi);
	TerrainIndex4 = TERRAIN_INDEX(xi     ,yi+lodi);

    float sx = xf*TERRAIN_SCALE;
    float sy = yf*TERRAIN_SCALE;

	Vector(sx              ,sy              ,BackTerrainHeight[TerrainIndex1],TerrainVertex[0]);
	Vector(sx+TERRAIN_SCALE,sy              ,BackTerrainHeight[TerrainIndex2],TerrainVertex[1]);
	Vector(sx+TERRAIN_SCALE,sy+TERRAIN_SCALE,BackTerrainHeight[TerrainIndex3],TerrainVertex[2]);
	Vector(sx              ,sy+TERRAIN_SCALE,BackTerrainHeight[TerrainIndex4],TerrainVertex[3]);

    if ( (TerrainWall[TerrainIndex1]&TW_HEIGHT)==TW_HEIGHT ) TerrainVertex[0][2] = g_fSpecialHeight;
    if ( (TerrainWall[TerrainIndex2]&TW_HEIGHT)==TW_HEIGHT ) TerrainVertex[1][2] = g_fSpecialHeight;
    if ( (TerrainWall[TerrainIndex3]&TW_HEIGHT)==TW_HEIGHT ) TerrainVertex[2][2] = g_fSpecialHeight;
    if ( (TerrainWall[TerrainIndex4]&TW_HEIGHT)==TW_HEIGHT ) TerrainVertex[3][2] = g_fSpecialHeight;

	if(!Flag)
	{
        if ( (TerrainWall[TerrainIndex1]&TW_NOGROUND)!=TW_NOGROUND )
			RenderTerrainFace_After(xf,yf,xi,yi,lodf);
	}
}

void RenderTerrainBitmapTile(float xf,float yf,float lodf,int lodi,vec3_t c[4],bool LightEnable,float Alpha,float Height=0.f)
{
	int xi = (int)xf;
    int yi = (int)yf;
	if(xi<0 || yi<0 || xi>=TERRAIN_SIZE_MASK || yi>=TERRAIN_SIZE_MASK) return;
	float TileScale = TERRAIN_SCALE*lodf;
    float sx = xf*TERRAIN_SCALE;
    float sy = yf*TERRAIN_SCALE;
	TerrainIndex1 = TERRAIN_INDEX(xi     ,yi     );
	TerrainIndex2 = TERRAIN_INDEX(xi+lodi,yi     );
	TerrainIndex3 = TERRAIN_INDEX(xi+lodi,yi+lodi);
	TerrainIndex4 = TERRAIN_INDEX(xi     ,yi+lodi);
	Vector(sx          ,sy          ,BackTerrainHeight[TerrainIndex1]+Height,TerrainVertex[0]);
	Vector(sx+TileScale,sy          ,BackTerrainHeight[TerrainIndex2]+Height,TerrainVertex[1]);
	Vector(sx+TileScale,sy+TileScale,BackTerrainHeight[TerrainIndex3]+Height,TerrainVertex[2]);
	Vector(sx          ,sy+TileScale,BackTerrainHeight[TerrainIndex4]+Height,TerrainVertex[3]);

	vec3_t Light[4];
	if(LightEnable)
	{
		VectorCopy(PrimaryTerrainLight[TerrainIndex1],Light[0]);
		VectorCopy(PrimaryTerrainLight[TerrainIndex2],Light[1]);
		VectorCopy(PrimaryTerrainLight[TerrainIndex3],Light[2]);
		VectorCopy(PrimaryTerrainLight[TerrainIndex4],Light[3]);
	}

	glBegin(GL_TRIANGLE_FAN);
	for(int i=0;i<4;i++)
	{
		if(LightEnable)
		{
			if(Alpha==1.f)
				glColor3fv(Light[i]);
			else
				glColor4f(Light[i][0],Light[i][1],Light[i][2],Alpha);
		}
		glTexCoord2f ( c[i][0], c[i][1] );
		glVertex3fv ( TerrainVertex[i] );
	}
	glEnd();
}

void RenderTerrainBitmap(int Texture,int mxi,int myi,float Rotation)
{
	glColor3f(1.f,1.f,1.f);

	vec3_t Angle;
	Vector(0.f,0.f,Rotation,Angle);
	float Matrix[3][4];
	AngleMatrix(Angle,Matrix);

	BindTexture(Texture);
	BITMAP_t *b = &Bitmaps[Texture];
	float TexScaleU = 64.f/b->Width;
	float TexScaleV = 64.f/b->Height;
    for(float y=0.f;y<b->Height/64.f;y+=1.f)
	{
		for(float x=0.f;x<b->Width/64.f;x+=1.f)
		{
			vec3_t p1[4],p2[4];
			Vector((x    )*TexScaleU,(y    )*TexScaleV,0.f,p1[0]);
			Vector((x+1.f)*TexScaleU,(y    )*TexScaleV,0.f,p1[1]);
			Vector((x+1.f)*TexScaleU,(y+1.f)*TexScaleV,0.f,p1[2]);
			Vector((x    )*TexScaleU,(y+1.f)*TexScaleV,0.f,p1[3]);
			for(int i=0;i<4;i++)
			{
				p1[i][0] -= 0.5f;
				p1[i][1] -= 0.5f;
				VectorRotate(p1[i],Matrix,p2[i]);
				p2[i][0] += 0.5f;
				p2[i][1] += 0.5f;
			}
   			RenderTerrainBitmapTile((float)mxi+x,(float)myi+y,1.f,1,p2,true,1.f);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//  지형에 맞춰서 이미지를 표시한다.
//////////////////////////////////////////////////////////////////////////
void RenderTerrainAlphaBitmap(int Texture,float xf,float yf,float SizeX,float SizeY,vec3_t Light,float Rotation,float Alpha,float Height)
{
	if(Alpha==1.f)
     	glColor3fv(Light);
	else
     	glColor4f(Light[0],Light[1],Light[2],Alpha);

	vec3_t Angle;
	Vector(0.f,0.f,Rotation,Angle);
	float Matrix[3][4];
	AngleMatrix(Angle,Matrix);
	
	BindTexture(Texture);
	float mxf = (xf/TERRAIN_SCALE);
	float myf = (yf/TERRAIN_SCALE);
	int   mxi = (int)(mxf);
	int   myi = (int)(myf);

	float Size;
	if(SizeX >= SizeY)
		Size = SizeX;
	else
		Size = SizeY;
	float TexU = (((float)mxi-mxf)+0.5f*Size);
	float TexV = (((float)myi-myf)+0.5f*Size);
	float TexScaleU = 1.f/Size;
	float TexScaleV = 1.f/Size;
	Size = (float)((int)Size+1);
	float Aspect = SizeX/SizeY;
    for(float y=-Size;y<=Size;y+=1.f)
	{
		for(float x=-Size;x<=Size;x+=1.f)
		{
			vec3_t p1[4],p2[4];
			Vector((TexU+x    )*TexScaleU,(TexV+y    )*TexScaleV,0.f,p1[0]);
			Vector((TexU+x+1.f)*TexScaleU,(TexV+y    )*TexScaleV,0.f,p1[1]);
			Vector((TexU+x+1.f)*TexScaleU,(TexV+y+1.f)*TexScaleV,0.f,p1[2]);
			Vector((TexU+x    )*TexScaleU,(TexV+y+1.f)*TexScaleV,0.f,p1[3]);
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			bool Clip = false;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			for(int i=0;i<4;i++) 
			{
				p1[i][0] -= 0.5f;
				p1[i][1] -= 0.5f;
				VectorRotate(p1[i],Matrix,p2[i]);
				p2[i][0] *= Aspect;
				p2[i][0] += 0.5f;
				p2[i][1] += 0.5f;
				//if((p2[i][0]>=0.f && p2[i][0]<=1.f) || (p2[i][1]>=0.f && p2[i][1]<=1.f)) Clip = true;
			}
  			RenderTerrainBitmapTile((float)mxi+x,(float)myi+y,1.f,1,p2,false,Alpha,Height);
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// Frustrum 관련 함수
///////////////////////////////////////////////////////////////////////////////

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
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	float Width = 0.0f, CameraViewFar = 0.0f, CameraViewNear = 0.0f, CameraViewTarget = 0.0f;
	float WidthFar = 0.0f, WidthNear = 0.0f;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
    float Width, CameraViewFar, CameraViewNear, CameraViewTarget;
	float WidthFar, WidthNear;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

#ifdef BATTLE_SOCCER_EVENT
    if ( World==WD_6STADIUM && ( FindText( Hero->ID, "webzen" ) || FindText( Hero->ID, "webzen2" ) )  )
    {
        Width = (float)GetScreenWidth()/640.f;
	    CameraViewFar    = 8500.f;
	    CameraViewNear   = CameraViewFar*0.05f;
	    CameraViewTarget = CameraViewFar*0.47f;
        WidthFar         = 3000.f*Width;
        WidthNear        = 540.f*Width;
    }
    else
#endif// BATTLE_SOCCER_EVENT
    {
        if ( battleCastle::InBattleCastle() && SceneFlag == MAIN_SCENE)
        {
            Width = (float)GetScreenWidth()/640.f;// * 0.1f;
            if ( battleCastle::InBattleCastle2( Hero->Object.Position ) && ( Hero->Object.Position[0]<17100.f || Hero->Object.Position[0]>18300.f ) )
            {
                CameraViewFar    = 5100.f;// * 0.1f;
                CameraViewNear   = CameraViewFar*0.19f;//0.22
                CameraViewTarget = CameraViewFar*0.47f;//0.47
                WidthFar  = 2250.f*Width; // 1140.f
                WidthNear = 540.f*Width; // 540.f
            }
            else
            {
                CameraViewFar    = 3300.f;// * 0.1f;
                CameraViewNear   = CameraViewFar*0.19f;//0.22
                CameraViewTarget = CameraViewFar*0.47f;//0.47
                WidthFar  = 1300.f*Width; // 1140.f
                WidthNear = 580.f*Width; // 540.f
            }
        }
#ifdef YDG_ADD_MAP_SANTA_TOWN
		else if(World == WD_62SANTA_TOWN)	// 산타마을
		{
			Width = (float)GetScreenWidth()/640.f * 1.0f;
	        CameraViewFar    = 2400.f;
			CameraViewNear   = CameraViewFar*0.19f;
			CameraViewTarget = CameraViewFar*0.47f;
	        CameraViewFar    = 2650.f;
			WidthFar  = 1250.f*Width;
			WidthNear = 540.f*Width;
		}
#endif	// YDG_ADD_MAP_SANTA_TOWN
#ifdef PBG_ADD_PKFIELD
		else if(IsPKField()
#ifdef YDG_ADD_MAP_DOPPELGANGER2
			|| IsDoppelGanger2()
#endif	// YDG_ADD_MAP_DOPPELGANGER2
			)
		{
			Width = (float)GetScreenWidth()/640.f;
			CameraViewFar    = 1700.0f;
			CameraViewNear   = 55.0f;
			CameraViewTarget = 830.0f;
			CameraViewFar    = 3300.f;
			WidthFar         = 1900.f*Width;
			WidthNear        = 600.f*Width;
		}
#endif //PBG_ADD_PKFIELD
        else
        {
            static  int CameraLevel;

            if((int)CameraDistanceTarget >= (int)CameraDistance )
                CameraLevel = g_shCameraLevel;

            switch ( CameraLevel )
            {
            case 0:
				if(SceneFlag == LOG_IN_SCENE)				// 이혁재 - 로그인 씬 일때 거리 세팅
				{
				}
				else if(SceneFlag == CHARACTER_SCENE)			// 이혁재 - 케릭터 씬 일때 거리 세팅
				{
#ifdef PJH_NEW_SERVER_SELECT_MAP
					Width = (float)GetScreenWidth()/640.f * 9.1f * 0.404998f;
#else //PJH_NEW_SERVER_SELECT_MAP
					Width = (float)GetScreenWidth()/640.f * 10.0f * 0.575995f;
#endif //PJH_NEW_SERVER_SELECT_MAP
				}
				else if(g_Direction.m_CKanturu.IsMayaScene() )			// 이혁재 - 칸투르 3차 맵일때
				{
					Width = (float)GetScreenWidth()/640.f * 10.0f * 0.115f;
				}
				else
				{
					Width = (float)GetScreenWidth()/640.f;// * 0.1f;
				}

				if(SceneFlag == LOG_IN_SCENE)				// 이혁재 - 로그인 씬 일때 거리 세팅
				{
				}
				else if(SceneFlag == CHARACTER_SCENE)	// 이혁재 - 케릭터 씬 일때 거리 세팅
				{
#ifdef PJH_NEW_SERVER_SELECT_MAP
					CameraViewFar = 2000.f * 9.1f * 0.404998f;
#else //PJH_NEW_SERVER_SELECT_MAP
					CameraViewFar = 2400.f * 10.0f * 0.575995f;
#endif //PJH_NEW_SERVER_SELECT_MAP
				}
				else if(World == WD_39KANTURU_3RD /* && MayaScene */)	// 이혁재 - 칸투르 3차 맵일때
				{
					CameraViewFar = 2000.f * 10.0f * 0.115f;
				}
				else
				{
	                CameraViewFar    = 2400.f;// * 0.1f;
				}

				if(SceneFlag == LOG_IN_SCENE)
				{
					Width = (float)GetScreenWidth()/640.f;
#ifdef PJH_NEW_SERVER_SELECT_MAP
	                CameraViewFar    = 2400.f * 17.0f*13.0f;
					CameraViewNear   = 2400.f * 17.0f*0.5f;
					CameraViewTarget = 2400.f * 17.0f*0.5f;
					WidthFar  = 5000.f*Width;
					WidthNear = 300.f*Width;
#else	// PJH_NEW_SERVER_SELECT_MAP
	                CameraViewFar    = 2400.f * 17.0f*1.1f;
					CameraViewNear   = 2400.f * 17.0f*0.5f;//0.22
					CameraViewTarget = 2400.f * 17.0f*0.48f;//0.47
					WidthFar  = 6300.f*Width; // 6300 3950.f
					WidthNear = 690.f*Width; // 690 690.f
#endif	// PJH_NEW_SERVER_SELECT_MAP
				}
				else
				{
					CameraViewNear   = CameraViewFar*0.19f;//0.22
					CameraViewTarget = CameraViewFar*0.47f;//0.47
					WidthFar  = 1190.f*Width; // 1140.f
					WidthNear = 540.f*Width; // 540.f
				}
                break;
            case 1:
                Width = (float)GetScreenWidth()/640.f+0.1f;// * 0.1f;
                CameraViewFar    = 2700.f;// * 0.1f;
                CameraViewNear   = CameraViewFar*0.19f;//0.22
                CameraViewTarget = CameraViewFar*0.47f;//0.47
                WidthFar  = 1200.f*Width; // 1140.f
                WidthNear = 540.f*Width; // 540.f
                break;
            case 2:
                Width = (float)GetScreenWidth()/640.f+0.1f;// * 0.1f;
                CameraViewFar    = 3000.f;// * 0.1f;
                CameraViewNear   = CameraViewFar*0.19f;//0.22
                CameraViewTarget = CameraViewFar*0.47f;//0.47
                WidthFar  = 1300.f*Width; // 1140.f
                WidthNear = 540.f*Width; // 540.f
                break;
            case 3:
                Width = (float)GetScreenWidth()/640.f+0.1f;// * 0.1f;
                CameraViewFar    = 3300.f;// * 0.1f;
                CameraViewNear   = CameraViewFar*0.19f;//0.22
                CameraViewTarget = CameraViewFar*0.47f;//0.47
                WidthFar  = 1500.f*Width; // 1140.f
                WidthNear = 580.f*Width; // 540.f
                break;
			case 4:
			case 5:
                Width = (float)GetScreenWidth()/640.f+0.1f;// * 0.1f;
                CameraViewFar    = 3400.f;// * 0.1f;
                CameraViewNear   = CameraViewFar*0.19f;//0.22
                CameraViewTarget = CameraViewFar*0.47f;//0.47
                WidthFar  = 1600.f*Width; // 1140.f
                WidthNear = 660.f*Width; // 540.f
                break;
            }
        }
    }

	vec3_t p[4];
	Vector(-WidthFar ,CameraViewFar -CameraViewTarget,0.f,p[0]);
	Vector( WidthFar ,CameraViewFar -CameraViewTarget,0.f,p[1]);
	Vector( WidthNear,CameraViewNear-CameraViewTarget,0.f,p[2]);
	Vector(-WidthNear,CameraViewNear-CameraViewTarget,0.f,p[3]);
	vec3_t Angle;
	float Matrix[3][4];

#ifdef BATTLE_SOCCER_EVENT
    if ( World==WD_6STADIUM && ( FindText( Hero->ID, "webzen" ) || FindText( Hero->ID, "webzen2" ) )  )
    {
	    Vector ( 0.f, 0.f, -CameraAngle[2], Angle );
    }
    else
#endif// BATTLE_SOCCER_EVENT
#ifdef PJH_NEW_SERVER_SELECT_MAP
	if (World == WD_73NEW_LOGIN_SCENE)
#else
	if (World == WD_77NEW_LOGIN_SCENE)
#endif //PJH_NEW_SERVER_SELECT_MAP
    {
		VectorScale(CameraAngle, -1.0f, Angle);
#ifdef PJH_NEW_SERVER_SELECT_MAP
		CCameraMove::GetInstancePtr()->SetFrustumAngle(89.5f);
		vec3_t _Temp = { CCameraMove::GetInstancePtr()->GetFrustumAngle(), 0.0f, 0.0f};
		VectorAdd(Angle,_Temp,Angle);
#endif //PJH_NEW_SERVER_SELECT_MAP
//		Angle[2] -= 180.0f;
	}
	else
	{
		Vector(0.f,0.f,45.f,Angle);
	}

	AngleMatrix(Angle,Matrix);
    vec3_t Frustrum[4];
	for(int i=0;i<4;i++)
	{
		VectorRotate(p[i],Matrix,Frustrum[i]);
		VectorAdd(Frustrum[i],Position,Frustrum[i]);
		FrustrumX[i] = Frustrum[i][0]*0.01f;
		FrustrumY[i] = Frustrum[i][1]*0.01f;
	}
	/*DisableTexture();
	glColor3f(0.5f,0.5f,0.5f);
	glBegin(GL_LINE_LOOP);
	for(i=0;i<4;i++)
	{
		glVertex3fv(Frustrum[i]);
	}
	glEnd();*/
}

bool TestFrustrum2D(float x,float y,float Range)
{
	if ( SceneFlag == SERVER_LIST_SCENE || SceneFlag == WEBZEN_SCENE || SceneFlag == LOADING_SCENE)
		return true;

	int j = 3;
	for(int i=0;i<4;j=i,i++)
	{
		float d = (FrustrumX[i]-x) * (FrustrumY[j]-y) -
	      		  (FrustrumX[j]-x) * (FrustrumY[i]-y);
		if(d <= Range)
		{
			return false;
		}
	}
	return true;
}

void CreateFrustrum(float Aspect, vec3_t position)
{
	float Distance = CameraViewFar*0.9f;
	float Width    = tanf(CameraFOV*0.5f*3.141592f/180.f) * Distance * Aspect + 100.f;
	float Height   = Width * 3.f / 4.f;
	vec3_t Temp[5];
	Vector( 0.f  , 0.f   , 0.f     ,Temp[0]);
	Vector(-Width, Height,-Distance,Temp[1]);
	Vector( Width, Height,-Distance,Temp[2]);
	Vector( Width,-Height,-Distance,Temp[3]);
	Vector(-Width,-Height,-Distance,Temp[4]);
	
	float FrustrumMinX = (float)TERRAIN_SIZE*TERRAIN_SCALE;
	float FrustrumMinY = (float)TERRAIN_SIZE*TERRAIN_SCALE;
	float FrustrumMaxX = 0.f;
	float FrustrumMaxY = 0.f;
	float Matrix[3][4];
	GetOpenGLMatrix(Matrix);
	for(int i=0;i<5;i++)
	{
		vec3_t t;
		VectorIRotate(Temp[i],Matrix,t);
		VectorAdd(t,CameraPosition,FrustrumVertex[i]);
		if(FrustrumMinX > FrustrumVertex[i][0]) FrustrumMinX = FrustrumVertex[i][0];
		if(FrustrumMinY > FrustrumVertex[i][1]) FrustrumMinY = FrustrumVertex[i][1];
		if(FrustrumMaxX < FrustrumVertex[i][0]) FrustrumMaxX = FrustrumVertex[i][0];
		if(FrustrumMaxY < FrustrumVertex[i][1]) FrustrumMaxY = FrustrumVertex[i][1];
	}

    int tileWidth = 4;

	FrustrumBoundMinX = (int)(FrustrumMinX/TERRAIN_SCALE)/tileWidth*tileWidth-tileWidth;
	FrustrumBoundMinY = (int)(FrustrumMinY/TERRAIN_SCALE)/tileWidth*tileWidth-tileWidth;
	FrustrumBoundMaxX = (int)(FrustrumMaxX/TERRAIN_SCALE)/tileWidth*tileWidth+tileWidth;
	FrustrumBoundMaxY = (int)(FrustrumMaxY/TERRAIN_SCALE)/tileWidth*tileWidth+tileWidth;
	if(FrustrumBoundMinX < 0) FrustrumBoundMinX = 0;
	if(FrustrumBoundMinY < 0) FrustrumBoundMinY = 0;
	if(FrustrumBoundMaxX < 0) FrustrumBoundMaxX = 0;
	if(FrustrumBoundMaxY < 0) FrustrumBoundMaxY = 0;
	if(FrustrumBoundMinX > TERRAIN_SIZE_MASK-tileWidth) FrustrumBoundMinX = TERRAIN_SIZE_MASK-tileWidth;
	if(FrustrumBoundMinY > TERRAIN_SIZE_MASK-tileWidth) FrustrumBoundMinY = TERRAIN_SIZE_MASK-tileWidth;
	if(FrustrumBoundMaxX > TERRAIN_SIZE_MASK-tileWidth) FrustrumBoundMaxX = TERRAIN_SIZE_MASK-tileWidth;
	if(FrustrumBoundMaxY > TERRAIN_SIZE_MASK-tileWidth) FrustrumBoundMaxY = TERRAIN_SIZE_MASK-tileWidth;

	FaceNormalize(FrustrumVertex[0],FrustrumVertex[1],FrustrumVertex[2],FrustrumFaceNormal[0]);
	FaceNormalize(FrustrumVertex[0],FrustrumVertex[2],FrustrumVertex[3],FrustrumFaceNormal[1]);
	FaceNormalize(FrustrumVertex[0],FrustrumVertex[3],FrustrumVertex[4],FrustrumFaceNormal[2]);
	FaceNormalize(FrustrumVertex[0],FrustrumVertex[4],FrustrumVertex[1],FrustrumFaceNormal[3]);
	FaceNormalize(FrustrumVertex[3],FrustrumVertex[2],FrustrumVertex[1],FrustrumFaceNormal[4]);
    FrustrumFaceD[0] = -DotProduct(FrustrumVertex[0],FrustrumFaceNormal[0]);
    FrustrumFaceD[1] = -DotProduct(FrustrumVertex[0],FrustrumFaceNormal[1]);
    FrustrumFaceD[2] = -DotProduct(FrustrumVertex[0],FrustrumFaceNormal[2]);
    FrustrumFaceD[3] = -DotProduct(FrustrumVertex[0],FrustrumFaceNormal[3]);
    FrustrumFaceD[4] = -DotProduct(FrustrumVertex[1],FrustrumFaceNormal[4]);

    CreateFrustrum2D(position);
}

bool TestFrustrum(vec3_t Position,float Range)
{
	for(int i=0;i<5;i++)
	{
		float Value;
		Value = FrustrumFaceD[i] + DotProduct(Position,FrustrumFaceNormal[i]);
		if(Value < -Range) return false;
	}
	return true;
}

#ifdef DYNAMIC_FRUSTRUM

void CFrustrum::Make(vec3_t vEye, float fFov, float fAspect, float fDist)
{
	float Width    = tanf(fFov*0.5f*3.141592f/180.f) * fDist * fAspect + 100.f;
	float Height   = Width * 3.f / 4.f;
	vec3_t Temp[5];
	vec3_t FrustrumVertex[5];
	Vector(0.f, 0.f, 0.f, Temp[0]);
	Vector(-Width, Height,-fDist,Temp[1]);
	Vector( Width, Height,-fDist,Temp[2]);
	Vector( Width,-Height,-fDist,Temp[3]);
	Vector(-Width,-Height,-fDist,Temp[4]);
	
	float Matrix[3][4];
	GetOpenGLMatrix(Matrix);
	for(int i=0;i<5;i++)
	{
		vec3_t t;
		VectorIRotate(Temp[i],Matrix,t);
		VectorAdd(t,vEye,FrustrumVertex[i]);
	}

	FaceNormalize(FrustrumVertex[0],FrustrumVertex[1],FrustrumVertex[2],m_FrustrumNorm[0]);
	FaceNormalize(FrustrumVertex[0],FrustrumVertex[2],FrustrumVertex[3],m_FrustrumNorm[1]);
	FaceNormalize(FrustrumVertex[0],FrustrumVertex[3],FrustrumVertex[4],m_FrustrumNorm[2]);
	FaceNormalize(FrustrumVertex[0],FrustrumVertex[4],FrustrumVertex[1],m_FrustrumNorm[3]);
	FaceNormalize(FrustrumVertex[3],FrustrumVertex[2],FrustrumVertex[1],m_FrustrumNorm[4]);
    m_FrustrumD[0] = -DotProduct(FrustrumVertex[0],m_FrustrumNorm[0]);
    m_FrustrumD[1] = -DotProduct(FrustrumVertex[0],m_FrustrumNorm[1]);
    m_FrustrumD[2] = -DotProduct(FrustrumVertex[0],m_FrustrumNorm[2]);
    m_FrustrumD[3] = -DotProduct(FrustrumVertex[0],m_FrustrumNorm[3]);
    m_FrustrumD[4] = -DotProduct(FrustrumVertex[1],m_FrustrumNorm[4]);
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
	for(int i=0; i<5; ++i)
	{
		float fValue;
		fValue = m_FrustrumD[i] + DotProduct(vPos, m_FrustrumNorm[i]);
		if(fValue < -fRange) return false;
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
	for(; iter != g_FrustrumMap.end(); ++iter)
	{
		CFrustrum* pData = iter->second;
		if(!pData) continue;
		pData->SetEye(CameraPosition);
		pData->Reset();
	}
}

CFrustrum* FindFrustrum(unsigned int iID)
{
	FrustrumMap_t::iterator iter = g_FrustrumMap.find(iID);
	if(iter != g_FrustrumMap.end()) return (CFrustrum*)iter->second;
	return NULL;
}

void DeleteAllFrustrum()
{
	FrustrumMap_t::iterator iter = g_FrustrumMap.begin();
	for(; iter != g_FrustrumMap.end(); ++iter)
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

extern int RainCurrent;
#ifdef USE_EVENT_ELDORADO
extern int EnableEvent;
#endif

void InitTerrainLight()
{
    int xi,yi;
	yi = FrustrumBoundMinY;
    for(;yi<=FrustrumBoundMaxY+3;yi+=1)
	{
		xi = FrustrumBoundMinX;
		for(;xi<=FrustrumBoundMaxX+3;xi+=1)
        {
			int Index = TERRAIN_INDEX_REPEAT(xi,yi);
			VectorCopy(BackTerrainLight[Index],PrimaryTerrainLight[Index]);
		}
	}
	float WindScale;
	float WindSpeed;
#ifdef USE_EVENT_ELDORADO
	if(EnableEvent==0)
#else
	if(!EnableEvent)
#endif
	{
		WindScale = 10.f;
		WindSpeed = (int)WorldTime%(360000*2)*(0.002f);
	}
	else
	{
		WindScale = 10.f;
		WindSpeed = (int)WorldTime%36000*(0.01f);
	}
#ifdef ASG_ADD_MAP_KARUTAN
	float WindScale1 = 0.f;
	float WindSpeed1 = 0.f;
	if (IsKarutanMap())	// 칼루탄에서만 풀 흔들리는 정도가 다름. 물 흐르는 속도는 다른 맵이랑 동일하므로 어쩔 수 없음.
	{
		WindScale1 = 15.f;
		WindSpeed1 = (int)WorldTime%36000*(0.008f);
	}
#endif	// ASG_ADD_MAP_KARUTAN
	yi = FrustrumBoundMinY;
#ifdef YDG_FIX_INVALID_TERRAIN_LIGHT
    for(;yi<=min(FrustrumBoundMaxY+3, TERRAIN_SIZE_MASK);yi+=1)
#else	// YDG_FIX_INVALID_TERRAIN_LIGHT
    for(;yi<=FrustrumBoundMaxY+3;yi+=1)
#endif	// YDG_FIX_INVALID_TERRAIN_LIGHT
	{
		xi = FrustrumBoundMinX;
		float xf = (float)xi;
#ifdef YDG_FIX_INVALID_TERRAIN_LIGHT
		for(;xi<=min(FrustrumBoundMaxX+3, TERRAIN_SIZE_MASK);xi+=1,xf+=1.f)
#else	// YDG_FIX_INVALID_TERRAIN_LIGHT
		for(;xi<=FrustrumBoundMaxX+3;xi+=1,xf+=1.f)
#endif	// YDG_FIX_INVALID_TERRAIN_LIGHT
        {
			int Index = TERRAIN_INDEX(xi,yi);
			if(World == WD_8TARKAN)
			{
     			TerrainGrassWind[Index] = sinf(WindSpeed+xf*50.f)*WindScale;
			}
#ifdef ASG_ADD_MAP_KARUTAN
			else if (IsKarutanMap())
			{
				TerrainGrassWind[Index] = sinf(WindSpeed+xf*50.f)*WindScale;
				g_fTerrainGrassWind1[Index] = sinf(WindSpeed1+xf*50.f)*WindScale1;
			}
#endif	// ASG_ADD_MAP_KARUTAN
#ifdef CSK_ADD_MAP_ICECITY	
			else if(World == WD_57ICECITY || World == WD_58ICECITY_BOSS)
			{
				WindScale = 60.f;
				TerrainGrassWind[Index] = sinf(WindSpeed+xf*50.f)*WindScale;
			}
#endif // CSK_ADD_MAP_ICECITY
			else
			{
     			TerrainGrassWind[Index] = sinf(WindSpeed+xf*5.f)*WindScale;
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

void Ray(int sx1,int sy1,int sx2,int sy2)
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

void InitTerrainRay(int HeroX,int HeroY)
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

void RenderTerrainBlock(float xf,float yf,int xi,int yi,bool EditFlag)
{
	//int x = ((xi/4)&63);
	//int y = ((yi/4)&63);
	//int lodi = LodBuffer[y*64+x];
	int lodi = 1;
	float lodf = (float)lodi;
	for(int i=0;i<4;i+=lodi)
	{
		float temp = xf;
		for(int j=0;j<4;j+=lodi)
		{
            if(TestFrustrum2D(xf+0.5f,yf+0.5f,0.f) || CameraTopViewEnable)
			{
				RenderTerrainTile(xf,yf,xi+j,yi+i,lodf,lodi,EditFlag);
			}
			xf+=lodf;
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
    float   yf = (float)yi;

    for(;yi<=FrustrumBoundMaxY;yi+=4,yf+=4.f)
	{
		xi = FrustrumBoundMinX;
		xf = (float)xi;
		for(;xi<=FrustrumBoundMaxX;xi+=4,xf+=4.f)
        {
            if(TestFrustrum2D(xf+2.f,yf+2.f,g_fFrustumRange) || CameraTopViewEnable)
			{
#ifdef PJH_NEW_SERVER_SELECT_MAP
				if (World == WD_73NEW_LOGIN_SCENE)
#else
				if (World == WD_77NEW_LOGIN_SCENE)
#endif //PJH_NEW_SERVER_SELECT_MAP
				{
					float fDistance_x = CameraPosition[0] - xf / 0.01f;
					float fDistance_y = CameraPosition[1] - yf / 0.01f;
					float fDistance = sqrtf(fDistance_x * fDistance_x + fDistance_y * fDistance_y);

					if (fDistance > 5200.f) // 컬링 최적화 가능한 부분☆
						continue;
				}
    			RenderTerrainBlock(xf,yf,xi,yi,EditFlag);
			}
        }
	}
}

void RenderTerrainBlock_After(float xf,float yf,int xi,int yi,bool EditFlag)
{
	int lodi = 1;
	float lodf = (float)lodi;
	for(int i=0;i<4;i+=lodi)
	{
		float temp = xf;
		for(int j=0;j<4;j+=lodi)
		{
            if(TestFrustrum2D(xf+0.5f,yf+0.5f,0.f) || CameraTopViewEnable)
			{
				RenderTerrainTile_After(xf,yf,xi+j,yi+i,lodf,lodi,EditFlag);
			}
			xf+=lodf;
		}
		xf = temp;
		yf += lodf;
	}
}

void RenderTerrainFrustrum_After(bool EditFlag)
{
    int   xi,yi;
    float xf,yf;
	yi = FrustrumBoundMinY;
	yf = (float)yi;
    for(;yi<=FrustrumBoundMaxY;yi+=4,yf+=4.f)
	{
		xi = FrustrumBoundMinX;
		xf = (float)xi;
		for(;xi<=FrustrumBoundMaxX;xi+=4,xf+=4.f)
        {
            if(TestFrustrum2D(xf+2.f,yf+2.f,-80.f) || CameraTopViewEnable)
			{
    			RenderTerrainBlock_After(xf,yf,xi,yi,EditFlag);
			}
        }
	}
}

extern int SelectMapping;
extern void RenderCharactersClient();
extern void RenderCharactersShadow();

//  지형을 그린다.
void RenderTerrain(bool EditFlag)
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_TERRAIN, PROFILING_RENDER_TERRAIN );
#endif // DO_PROFILING	
    if( !EditFlag )
    {
        if(World == WD_8TARKAN)		// 타르칸이면 
		{
            WaterMove = (float)((int)(WorldTime)%40000)*0.000025f;
		}
		// 3차체인지업 2차맵은 용암속도 느리게
		else if(World == WD_42CHANGEUP3RD_2ND)
		{
			int iWorldTime = (int)WorldTime;
			int iRemainder = iWorldTime % 50000;
			WaterMove = (float)iRemainder * 0.00002f;
		}
        else
		{
            WaterMove = (float)((int)(WorldTime)%20000)*0.00005f;
		}
    }

	if(!EditFlag)
	{
     	DisableAlphaBlend();
	}
	else
	{
     	SelectFlag = false;
		InitCollisionDetectLineToFace();
	}

    //  지형을 화면에 찍는다.
	TerrainFlag = TERRAIN_MAP_NORMAL;
    RenderTerrainFrustrum ( EditFlag );
    //  
	if ( EditFlag && SelectFlag )
	{
        //  현재 선택한 커서의 지형 위치를 다시 한번 검색한다.
		RenderTerrainTile ( SelectXF, SelectYF, (int)SelectXF, (int)SelectYF, 1.f, 1, EditFlag );
	}
	if ( !EditFlag )
	{
		EnableAlphaTest();
		if ( TerrainGrassEnable && World != WD_7ATLANSE 
#ifdef YDG_ADD_MAP_DOPPELGANGER3
			&& !IsDoppelGanger3()
#endif	// YDG_ADD_MAP_DOPPELGANGER3
			)
		{
			//  지형에 존재하는 잔디를 찍는다.
			TerrainFlag = TERRAIN_MAP_GRASS;
			RenderTerrainFrustrum ( EditFlag );
		}
		DisableDepthTest();
		EnableCullFace();
		RenderPointers();
		EnableDepthTest();
	}
#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_TERRAIN );
#endif // DO_PROFILING	
}

void RenderTerrain_After(bool EditFlag)
{
	if(World != WD_39KANTURU_3RD)
		return;

	TerrainFlag = TERRAIN_MAP_NORMAL;
	RenderTerrainFrustrum_After(EditFlag);
}

///////////////////////////////////////////////////////////////////////////////
// Sky 관련 함수
///////////////////////////////////////////////////////////////////////////////

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
	AngleIMatrix(Angle,Matrix);
	vec3_t p,Position;
	Vector(-900.f,CameraViewFar*0.9f,0.f,p);
	VectorRotate(p,Matrix,Position);
    VectorAdd(CameraPosition,Position,Sun.Position);
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
	AngleIMatrix(Angle,Matrix);
	float Aspect = (float)(WindowWidth)/(float)(WindowWidth);
	float Width  = 1780.f * Aspect;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	float Height = 700.f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

	BeginSprite();
	vec3_t p,Position;
	float Num = 20.f;
	vec3_t LightTable[21];
#ifdef _VS2008PORTING
	for(int i=0;i<=Num;i++)
#else // _VS2008PORTING
	int i;
	for(i=0;i<=Num;i++)
#endif // _VS2008PORTING
	{
		Vector(((float)i-Num*0.5f)*(Width/Num),CameraViewFar*0.99f,0.f,p);
		VectorRotate(p,Matrix,Position);
		VectorAdd(CameraPosition,Position,Position);
		RequestTerrainLight(Position[0],Position[1],LightTable[i]);
	}
#ifdef _VS2008PORTING
	for(int i=1;i<=(int)Num;i++)
#else // _VS2008PORTING
	for(i=1;i<=(int)Num;i++)
#endif // _VS2008PORTING
	{
		if(LightTable[i][0]<=0.f)
		{
			Vector(0.f,0.f,0.f,LightTable[i-1]);
		}
	}
#ifdef _VS2008PORTING
	for(int i=(int)Num-1;i>=0;i--)
#else // _VS2008PORTING
	for(i=(int)Num-1;i>=0;i--)
#endif // _VS2008PORTING
	{
		if(LightTable[i][0]<=0.f)
		{
			Vector(0.f,0.f,0.f,LightTable[i+1]);
		}
	}
	for(float x=0.f;x<Num;x+=1.f)
	{
     	float UV[4][2];
		TEXCOORD(UV[0],(x    )*(1.f/Num),1.f);
		TEXCOORD(UV[1],(x+1.f)*(1.f/Num),1.f);
		TEXCOORD(UV[2],(x+1.f)*(1.f/Num),0.f);
		TEXCOORD(UV[3],(x    )*(1.f/Num),0.f);

		vec3_t Light[4];
		VectorCopy(LightTable[(int)x  ],Light[0]);
		VectorCopy(LightTable[(int)x+1],Light[1]);
		//VectorCopy(LightTable[(int)x+1],Light[2]);
		//VectorCopy(LightTable[(int)x  ],Light[3]);
		Vector(1.f,1.f,1.f,Light[2]);
		Vector(1.f,1.f,1.f,Light[3]);

		Vector((x-Num*0.5f+0.5f)*(Width/Num),CameraViewFar*0.9f,0.f,p);
		VectorRotate(p,Matrix,Position);
		VectorAdd(CameraPosition,Position,Position);
       	Position[2] = 400.f;
		//RenderSpriteUV(BITMAP_SKY,Position,Width/Num,Height,UV,Light);
	}
	EndSprite();
}


