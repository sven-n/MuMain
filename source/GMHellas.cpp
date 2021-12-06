//////////////////////////////////////////////////////////////////////////
//  GMHellas.cpp
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "UIWindows.h"
#include "ZzzOpenglUtil.h"
#include "zzztexture.h"
#include "ZzzBMD.h"
#include "zzzlodterrain.h"
#include "ZzzScene.h"
#include "zzzEffect.h"
#include "zzzAi.h"
#include "zzzOpenData.h"
#include "ZzzInterface.h"
#include "PhysicsManager.h"
#include "CSWaterTerrain.h"
#include "GMHellas.h"
#include "DSPlaySound.h"
#include "NewUISystem.h"
#include "MapManager.h"
#include "CharacterManager.h"
#include "SkillManager.h"

extern  int  WaterTextureNumber;
extern	char TextList[30][100];
extern  int  TextListColor[30];
extern	int  TextBold[30];

static  CSWaterTerrain* g_pCSWaterTerrain = NULL;
static  std::queue<ObjectDescript> g_qObjDes;

static  const BYTE  ACTION_DESTROY_PHY_DEF = 33;
static  const BYTE  ACTION_DESTROY_DEF = 34;

#define NUM_HELLAS	7

static  const int g_iKalimaLevel[14][2] = { { 40, 999 }, { 131, 999 }, { 181, 999 }, { 231, 999 }, { 281, 999 }, { 331, 999 }, { 350, 999 },
                                            { 20, 999 }, { 111, 999 }, { 161, 999 }, { 211, 999 }, { 261, 999 }, { 311, 999 }, { 350, 999 } };

#define KUNDUN_ZONE NUM_HELLAS

bool CreateWaterTerrain ( int mapIndex )
{
    if ( gMapManager.InHellas( mapIndex ) )
    {
        DeleteWaterTerrain ();

        g_pCSWaterTerrain = new CSWaterTerrain ( mapIndex );

        return true;
    }
    else
    {
        DeleteWaterTerrain ();
    }

    return false;
}

bool IsWaterTerrain ( void )
{
    if ( g_pCSWaterTerrain!=NULL )
    {
        return true;
    }
    return false;
}

void AddWaterWave ( int x, int y, int range, int height )
{
    if ( g_pCSWaterTerrain!=NULL )
    {
        int WaveX = (x*2);
        int WaveY = (y*2);
        g_pCSWaterTerrain->addSineWave( WaveX, WaveY, range, range, height );
    }
}

void MoveWaterTerrain ( void )
{
    if ( g_pCSWaterTerrain!=NULL )
    {
        g_pCSWaterTerrain->Update ();
    }
}

bool RenderWaterTerrain ( void )
{
    if ( g_pCSWaterTerrain!=NULL )
    {
        g_pCSWaterTerrain->Update ();
        g_pCSWaterTerrain->Render ();
        return true;
    }
    return false;
}

void DeleteWaterTerrain ( void )
{
    if ( g_pCSWaterTerrain!=NULL )
    {
        delete g_pCSWaterTerrain;
        g_pCSWaterTerrain = NULL;
    }
}

float GetWaterTerrain(float x, float y )
{
    if ( x<0 ) x = 0;
    if ( y<0 ) y = 0;
    if ( g_pCSWaterTerrain!=NULL )
    {
        return g_pCSWaterTerrain->GetWaterTerrain ( x, y );
    }
    return 0.f;
}

void RenderWaterTerrain(int Texture, float xf, float yf, float SizeX, float SizeY, vec3_t Light, float Rotation, float Alpha, float Height )
{
    if ( g_pCSWaterTerrain!=NULL )
    {
        g_pCSWaterTerrain->RenderWaterAlphaBitmap ( Texture, xf, yf, SizeX, SizeY, Light, Rotation, Alpha, Height);
    }
}

void SettingHellasColor()
{
    glClearColor ( 0.f, 0.f, 0.f, 1.f );
}

BYTE GetHellasLevel(int Class, int Level)
{
    int startIndex = 0;
    if (gCharacterManager.GetBaseClass( Class )==CLASS_DARK || gCharacterManager.GetBaseClass( Class )==CLASS_DARK_LORD   
#ifdef PBG_ADD_NEWCHAR_MONK
		|| GetBaseClass( Class )==CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK
		)
    {
        startIndex = NUM_HELLAS;
    }

    int byLevel = 0;
    for ( int i=0; i<NUM_HELLAS; i++ )
    {
        byLevel++;
        if ( Level>=g_iKalimaLevel[startIndex+i][0] && Level<=g_iKalimaLevel[startIndex+i][1] )
        {
            break;
        }
    }
    return byLevel;
}

bool EnableKalima(int Class, int Level, int ItemLevel)
{
    int startIndex = 0;

    if(gCharacterManager.GetBaseClass( Class ) == CLASS_DARK || gCharacterManager.GetBaseClass( Class ) == CLASS_DARK_LORD
#ifdef PBG_ADD_NEWCHAR_MONK
		|| GetBaseClass( Class ) == CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK
		)
    {
        startIndex = NUM_HELLAS;
    }
	
	if ( Level < g_iKalimaLevel[startIndex+ItemLevel-1][0] )
	{
		return false;
	}

	return true;
}

bool GetUseLostMap ( bool bDrawAlert )
{
    int Level = CharacterAttribute->Level;

    int startIndex = 0;

    if(gCharacterManager.GetBaseClass( Hero->Class )==CLASS_DARK || gCharacterManager.GetBaseClass( Hero->Class )==CLASS_DARK_LORD  
#ifdef PBG_ADD_NEWCHAR_MONK
		|| GetBaseClass( Hero->Class )==CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK
		)
    {
        startIndex = NUM_HELLAS;
    }

    if(bDrawAlert && Hero->SafeZone)
    {
		g_pChatListBox->AddText("", GlobalText[1238], SEASON3B::TYPE_ERROR_MESSAGE);
        return false;
    }

    if ( Level >= g_iKalimaLevel[startIndex][0] )
	{
        return true;
	}

    if ( bDrawAlert )
    {
        char Text[100];
        sprintf ( Text, GlobalText[1123], g_iKalimaLevel[startIndex][0] );
		g_pChatListBox->AddText( "", Text, SEASON3B::TYPE_ERROR_MESSAGE);
    }

    return false;
}

int RenderHellasItemInfo(ITEM* ip, int textNum )
{
    int TextNum = textNum;
    switch ( ip->Type )
    {
    case ITEM_POTION+28:
        {
            int startIndex = 0;
            if (gCharacterManager.GetBaseClass( Hero->Class )==CLASS_DARK || gCharacterManager.GetBaseClass( Hero->Class )==CLASS_DARK_LORD  
#ifdef PBG_ADD_NEWCHAR_MONK
					|| GetBaseClass( Hero->Class )==CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK
				)
            {
                startIndex = NUM_HELLAS;
            }

            int HeroLevel = CharacterAttribute->Level;
            int ItemLevel = (ip->Level>>3)&15;
            
            TextListColor[TextNum] = TEXT_COLOR_WHITE;
            sprintf ( TextList[TextNum], "%s %s       %s    ", GlobalText[58], GlobalText[368], GlobalText[935] ); TextListColor[TextNum] = TEXT_COLOR_WHITE; TextBold[TextNum] = false; TextNum++;
            for ( int i=0; i<NUM_HELLAS; i++ )
            {
                sprintf(TextList[TextNum],"        %d             %3d~%3d     ", i+1,  g_iKalimaLevel[startIndex+i][0],  min( 400, g_iKalimaLevel[startIndex+i][1] ) ); 
                
                if ( ItemLevel==i+1 )
                {
                    TextListColor[TextNum] = TEXT_COLOR_DARKYELLOW; 
                }
                else
                {
                    TextListColor[TextNum] = TEXT_COLOR_WHITE; 
                }
                TextBold[TextNum] = false; TextNum++;
            }
            
            sprintf ( TextList[TextNum], "\n" ); TextNum++;
            sprintf ( TextList[TextNum], GlobalText[1184] );  TextListColor[TextNum] = TEXT_COLOR_DARKBLUE; TextNum++;

            if ( HeroLevel<g_iKalimaLevel[startIndex][0] )
            {
                sprintf ( TextList[TextNum], "\n" ); TextNum++;
                sprintf ( TextList[TextNum], GlobalText[1123], g_iKalimaLevel[startIndex][0] );  TextListColor[TextNum] = TEXT_COLOR_DARKRED; TextNum++;
            }
        }
        break;

    case ITEM_POTION+29 :
        {
            sprintf ( TextList[TextNum], GlobalText[1181], ip->Durability, 5 ); TextNum++;
            if ( ip->Durability>=5 )
            {
                sprintf ( TextList[TextNum], GlobalText[1182] ); TextListColor[TextNum] = TEXT_COLOR_YELLOW; TextNum++;
            }
            else
            {
                sprintf ( TextList[TextNum], GlobalText[1183], (5-ip->Durability) ); TextListColor[TextNum] = TEXT_COLOR_YELLOW; TextNum++;
            }
        }
        break;
    }

    return TextNum;
}

void AddObjectDescription(char* Text, vec3_t position )
{
    ObjectDescript QD;
    
    memcpy ( QD.m_strName, Text, sizeof( char )*64 );
    VectorCopy ( position, QD.m_vPos );
    
    g_qObjDes.push ( QD );
}

void RenderObjectDescription()
{
    glColor3f ( 1.f, 1.f, 1.f );
    while ( !g_qObjDes.empty() )
    {
        ObjectDescript QD = g_qObjDes.front ();

        int    x, y;
        vec3_t Position;
	    Vector ( QD.m_vPos[0], QD.m_vPos[1], QD.m_vPos[2]+370.f, Position );
	    Projection ( Position, &x, &y );

        if ( x>=0 && y>=0 )
        {
			g_pRenderText->SetFont(g_hFontBold);
			g_pRenderText->SetTextColor(255, 230, 200, 255);
			g_pRenderText->SetBgColor(100, 0, 0, 255);
            g_pRenderText->RenderText(x, y, QD.m_strName, 0 ,0, RT3_WRITE_CENTER);
        }

        g_qObjDes.pop ();
    }
}

bool MoveHellasObjectSetting(int& objCount, int object)
{
    if ( gMapManager.InHellas()==false ) return false;

    PlayBuffer ( SOUND_KALIMA_AMBIENT );
    if ( (MoveSceneFrame%100)==0 )
    {
        PlayBuffer ( SOUND_KALIMA_AMBIENT2+rand()%2 );
    }

    if ( GetHellasLevel( Hero->Class, CharacterAttribute->Level )==KUNDUN_ZONE)
    {
        int CurrX =  ( Hero->PositionX );
        int CurrY =  ( Hero->PositionY );

        if ( ( CurrX>=25 && CurrY>=44 ) && ( CurrX<=51 && CurrY<=119 ) && (MoveSceneFrame%50)==0 )
        {
            PlayBuffer ( SOUND_KUNDUN_AMBIENT1+rand()%2 );
        }
    }

    if ( (rand()%10)==0 && object )
    {
        objCount = rand()%object;
        return true;
    }

    if ( (rand()%5)==0 )
    {
        vec3_t Position, Light;

        Vector ( 0.3f, 0.8f, 1.f, Light );

        Position[0] = Hero->Object.Position[0]+rand()%800-400.f;
        Position[1] = Hero->Object.Position[1]+rand()%800-400.f;
        Position[2] = Hero->Object.Position[2]+50.f;

        CreateParticle ( BITMAP_LIGHT, Position, Hero->Object.Angle, Light, 7, 1.f, &Hero->Object );

        if ( (rand()%15)==0 )
        {
            vec3_t Angle = { 0.f, 0.f, 0.f };
            Position[2] = Hero->Object.Position[2]+800.f;
            CreateEffect ( 9, Position, Angle, Light );

            PlayBuffer ( SOUND_KALIMA_FALLING_STONE );
        }
    }

    return true;
}

bool MoveHellasObject(OBJECT* o, int& object, int& visibleObject)
{
    if ( gMapManager.InHellas()==true )
    {
        return true;
    }

    return false;
}

bool MoveHellasAllObject(OBJECT* o)
{
    if ( gMapManager.InHellas()==false ) return false;

    return true;
}

bool CreateHellasObject(OBJECT* o)
{
    return false;
}

bool MoveHellasVisual(OBJECT* o)
{
    if ( gMapManager.InHellas()==false ) return false;

    switch(o->Type)
    {
    case    37 :
    case    38 :
    case    39 :
    case    40 :
        o->HiddenMesh = -2;
        break;
    }
    return true;
}

void CheckGrass(OBJECT* o)
{
	vec3_t Position;
	VectorCopy ( Hero->Object.Position, Position );
	if ( Hero->Object.CurrentAction>=PLAYER_WALK_MALE && Hero->Object.CurrentAction<=PLAYER_RUN_RIDE_WEAPON
		|| Hero->Object.CurrentAction>=PLAYER_FENRIR_RUN && Hero->Object.CurrentAction<=PLAYER_FENRIR_RUN_ONE_LEFT_ELF
		|| Hero->Object.CurrentAction>=PLAYER_FENRIR_WALK && Hero->Object.CurrentAction<=PLAYER_FENRIR_WALK_ONE_LEFT
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
		|| Hero->Object.CurrentAction>=PLAYER_RAGE_FENRIR_RUN && Hero->Object.CurrentAction<=PLAYER_RAGE_FENRIR_RUN_ONE_LEFT
		|| Hero->Object.CurrentAction>=PLAYER_RAGE_FENRIR_WALK && Hero->Object.CurrentAction<=PLAYER_RAGE_FENRIR_WALK_TWO_SWORD
		|| Hero->Object.CurrentAction>=PLAYER_RAGE_UNI_RUN && Hero->Object.CurrentAction>=PLAYER_RAGE_UNI_RUN_ONE_RIGHT
#endif //PBG_ADD_NEWCHAR_MONK_ANI
		)
	{
		if ( o->Direction[0]<0.1f )
		{
			float dx = Position[0]-o->Position[0];
			float dy = Position[1]-o->Position[1];
			float Distance = sqrtf ( dx*dx + dy*dy );
			if ( Distance<50.f )
			{
				Vector ( -dx*0.6f, -dy*0.6f, 0.f, o->Direction );
			}
		}

	    VectorScale(o->Direction,0.6f,o->Direction);
	    VectorScale(o->HeadAngle,0.6f,o->HeadAngle);
	    VectorAdd(o->Position,o->Direction,o->Position);
	    VectorAdd(o->Angle,o->HeadAngle,o->Angle);
	}
}

bool RenderHellasVisual(OBJECT* o,BMD* b)
{
    if ( gMapManager.InHellas()==false ) return false;

    vec3_t p, Position;
    vec3_t Light;
    float  Luminosity = 0.f;

    switch(o->Type)
    {
    case 12:
        Luminosity = sinf ( WorldTime*0.001f )*0.3f + 0.7f;
        
        Vector ( 0.6f, 0.6f, 1.f, Light );
        Vector ( 0.f, 0.f, 0.f, p );
        b->TransformPosition ( BoneTransform[5], p, Position );
        CreateSprite ( BITMAP_LIGHT, Position, Luminosity+0.2f, Light, o );
        break;
    case 15:
    case 29:
        CheckGrass ( o );
        o->Position[2] = GetWaterTerrain ( o->Position[0], o->Position[1] )+180;
        break;
    case 32:
        CheckGrass ( o );
        Luminosity = sinf ( WorldTime*0.001f )*0.3f + 0.7f;
        
        Vector ( 0.6f, 0.6f, 1.f, Light );
        Vector ( 0.f, 0.f, 0.f, p );
        b->TransformPosition ( BoneTransform[5], p, Position );
        CreateSprite ( BITMAP_LIGHT, Position, Luminosity+0.2f, Light, o );
        o->Position[2] = GetWaterTerrain ( o->Position[0], o->Position[1] )+180;
        break;
    case 35:
        Vector ( 0.3f, 0.6f, 1.f, Light );
        CreateParticle( BITMAP_LIGHT, o->Position, o->Angle, Light, 6, 1.f, o );
        o->HiddenMesh = -2;
        break;
    case 36:
        Vector ( 1.f, 1.f, 1.f, Light );
        CreateParticle ( BITMAP_TRUE_BLUE, o->Position, o->Angle, Light, 0 );

        o->Scale = 0.5f;
        o->HiddenMesh = -2;
        break;
        
    case 37 :
        Vector ( 1.f, 1.f, 1.f, Light );
        CreateParticle ( BITMAP_WATERFALL_5, o->Position, o->Angle, Light, 0 );
        o->Scale = 0.5f;
        PlayBuffer ( SOUND_KALIMA_WATER_FALL );
        break;
    case 38 :
        Vector ( 1.f, 1.f, 1.f, Light );
        if ( rand()%2==0 )
        {
            CreateParticle ( BITMAP_WATERFALL_1, o->Position, o->Angle, Light, 0 );
        }
        PlayBuffer ( SOUND_KALIMA_WATER_FALL );
        o->Scale = 0.5f;
        break;
    case 39 :
        Vector ( 1.f, 1.f, 1.f, Light );
        CreateParticle ( BITMAP_WATERFALL_3+(rand()%2), o->Position, o->Angle, Light, 0 );
        o->Scale = 0.5f;
        break;
    case 40 :
        Vector ( 1.f, 1.f, 1.f, Light );
        if ( rand()%4==0 )
        {
            CreateParticle ( BITMAP_WATERFALL_2, o->Position, o->Angle, Light, 0 );
        }
        o->Scale = 0.5f;
        break;
    }

    return true;
}

bool RenderHellasObjectMesh ( OBJECT* o, BMD* b )
{
    if ( o->Type==MODEL_MONSTER01+33 && gMapManager.InHellas() )
    {
		Vector ( 0.0f, 0.0f, 0.0f, b->BodyLight );
        b->RenderBody( RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh );
		Vector ( 1.f, 1.f, 1.f, b->BodyLight );
        return true;
    }
    else if ( o->Type==MODEL_WARCRAFT )
    {
		if(o->SubType == 1)
		{
			Vector ( 1.0f, 0.1f, 0.1f, b->BodyLight );
		}
		else
		{
			Vector ( 1.0f, 1.0f, 1.0f, b->BodyLight );
		}
		b->RenderBody ( RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);		
        return true;
    }
 	else if ( o->Type==MODEL_CUNDUN_DRAGON_HEAD )
	{
		Vector ( 0.3f, 0.3f, 0.3f, b->BodyLight );
		b->RenderBody( RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh );

		float Luminosity = (float)sin ( WorldTime*0.003f )*0.2f+0.8f;
		vec3_t p, Light, Position;
		Vector(0,0,0,p);
		Vector ( Luminosity*1.0f, Luminosity*0.2f, Luminosity*0.1f, Light );
		b->TransformPosition ( BoneTransform[3], p, Position, false );
		CreateSprite(BITMAP_ENERGY,Position,0.2f,Light,o,0.f);
		Vector ( Luminosity*1.0f, Luminosity*0.2f, Luminosity*0.1f, Light );
		CreateSprite(BITMAP_SHINY+1,Position,0.5f,Light,o,0.f);

		Vector ( Luminosity*1.0f, Luminosity*0.2f, Luminosity*0.1f, Light );
		b->TransformPosition ( BoneTransform[4], p, Position, false );
		CreateSprite(BITMAP_ENERGY,Position,0.2f,Light,o,0.f);
		Vector ( Luminosity*1.0f, Luminosity*0.2f, Luminosity*0.1f, Light );
		CreateSprite(BITMAP_SHINY+1,Position,0.5f,Light,o,0.f);
		
		return true;
	}
	else if ( o->Type==MODEL_CUNDUN_GHOST )
	{
		Vector ( 1.0f, 1.0f, 1.0f, b->BodyLight );
		if (o->AnimationFrame > 3 && o->Alpha > 0.2f)
			o->Alpha -= 0.02f;
		b->RenderBody( RENDER_TEXTURE|RENDER_DARK,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh );
		b->RenderBody( RENDER_TEXTURE|RENDER_DARK,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh );
		Vector ( 0.0f, 0.0f, 0.0f, b->BodyLight );
        RenderPartObjectEdge ( b, o, RENDER_COLOR, true, 0.7f );

		float Luminosity = (float)sin ( WorldTime*0.003f )*0.2f+0.8f;
		vec3_t p, Light, Position;

		Vector(0,0,0,p);
		Vector ( Luminosity*1.0f, Luminosity*0.2f, Luminosity*0.1f, Light );
		b->TransformPosition ( BoneTransform[8], p, Position, false );
		CreateSprite(BITMAP_ENERGY,Position,0.2f,Light,o,0.f);
		Vector ( Luminosity*1.0f, Luminosity*0.2f, Luminosity*0.1f, Light );
		CreateSprite(BITMAP_SHINY+1,Position,0.5f,Light,o,0.f);

		Vector ( Luminosity*1.0f, Luminosity*0.2f, Luminosity*0.1f, Light );
		b->TransformPosition ( BoneTransform[9], p, Position, false );
		CreateSprite(BITMAP_ENERGY,Position,0.2f,Light,o,0.f);
		Vector ( Luminosity*1.0f, Luminosity*0.2f, Luminosity*0.1f, Light );
		CreateSprite(BITMAP_SHINY+1,Position,0.5f,Light,o,0.f);
		
        {
            Vector(2.f,10.f,0.f,p);
            b->TransformPosition(BoneTransform[6],p,Position,false);
            Vector(1.0f,0.2f,0.0f,Light);
            CreateParticle(BITMAP_SMOKE,Position,o->Angle,Light,17,3.0f);
        }
		return true;
	}
    else if ( gMapManager.InHellas()==true && ( o->Type>=MODEL_WORLD_OBJECT && o->Type<MAX_WORLD_OBJECTS ) )
    {
        b->RenderBody( RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh );
        
        if ( o->Type>=0 && o->Type<=66 ) 
        {
            if ( o->Type!=2  && o->Type!=4  && o->Type!=12 && o->Type!=14 && o->Type!=15 && o->Type!=18 && o->Type!=20 && o->Type!=21 && o->Type!=27 && o->Type!=29 && o->Type!=30 && 
                o->Type!=31 && o->Type!=32 && o->Type!=41 && o->Type!=43 && o->Type!=52 && o->Type!=54 && o->Type!=55 )
            {
                float Luminosity = sinf(WorldTime*0.002f)*0.1f+0.3f;
                Vector( Luminosity, Luminosity, Luminosity, b->BodyLight );
                b->RenderBody( RENDER_TEXTURE,0.3f,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh, BITMAP_WATER+WaterTextureNumber  );
            }
            
            if ( o->Type==34 )
            {
                if ( b->NumMeshs>1 )
                {
                    b->BeginRender(o->Alpha);
                    b->RenderMesh( 1, RENDER_CHROME|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh );
                    b->EndRender();
                }
            }
            
            if ( o->Type==15 || o->Type==29 || o->Type==32 )
            {
                DisableAlphaBlend();
                Vector ( 0.1f, 0.1f, 0.1f, b->BodyLight );
                b->RenderBody( RENDER_TEXTURE|RENDER_SHADOWMAP,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh );
                Vector ( 1.f, 1.f, 1.f, b->BodyLight );
            }
        }
        return true;
    }
    else
    {
        return RenderHellasMonsterObjectMesh ( o, b );
    }

    return false;
}

int CreateBigMon( OBJECT* o )
{
	if ( gMapManager.InHellas()==false ) return 0;

	if ((MoveSceneFrame % 100) == 0)
	{
		o->Live  = true;
        OpenMonsterModel(33);
		o->Type  = MODEL_MONSTER01+33;
		o->Scale = 2.5f+(float)(rand()%3+6)*0.05f;
		o->Alpha = 1.f;
		o->AlphaTarget = o->Alpha;
        o->LightEnable = false;
		o->Velocity    = (float)(rand()%10+10)*0.04f;
		o->Gravity     = rand()%3-1.5f;
		o->LightEnable = true;
		o->AlphaEnable = false;
		o->SubType     = 0;
		o->HiddenMesh  = 5;
		o->BlendMesh   = -1;
        o->LifeTime    = 200;
		o->CurrentAction = MONSTER01_WALK;
		SetAction ( o, o->CurrentAction );
		Vector ( 0.f, 0.f, 90.f-rand()%30-15, o->Angle );
		Vector ( Hero->Object.Position[0]-1000-rand()%200,
			     Hero->Object.Position[1]-500+rand()%200,
			     Hero->Object.Position[2]-800.f, o->Position );
    }
	else
	{
		o->Live = false;
	}
    return 1;
}

void MoveBigMon(OBJECT* o)
{
	o->Angle[2] += o->Gravity;

    if ( rand()%5==0 )
    {
        o->Gravity *= -1;
    }

    if ( o->LifeTime<20 )
    {
        o->Alpha /= 1.2f;
        o->Velocity += 0.5f;
        o->Angle[0] += 2.f;
    }

    if ( o->LifeTime<0 ) o->Live = false;
}

void CreateMonsterSkill_ReduceDef(OBJECT* o, int AttackTime, BYTE time, float Height)
{
    if ( AttackTime>=time )
    {
        vec3_t Angle, Light, Position, p;

        Vector ( 0.f, 0.f, 0.f, Light );
        Vector ( 0.f, 0.f, 0.f, p );
        VectorCopy ( o->Position, Position );

        Position[2] += Height;
        for ( int i=0;i<3; i++ )
        {
            Vector ( 0.f, 0.f, i*120.f, Angle );
            CreateEffect ( MODEL_SKULL, Position, Angle, Light, 1, o );
        }

        PlayBuffer ( SOUND_SKILL_SKULL );
    }
}

void CreateMonsterSkill_Poison ( OBJECT* o, int AttackTime, BYTE time )
{
    if ( AttackTime>=time )
    {
        float  Matrix[3][4];
        vec3_t Angle, Light, Position, p;
        
        Vector ( 0.f, 0.f, (float)(rand()%360), Angle );
        Vector ( 0.f, 300.f, 0.f, p );
        Vector ( 0.8f, 0.5f, 0.1f, Light );
        for ( int i=0; i<5; i++ )
        {
            Angle[2] += 72.f;
            AngleMatrix ( Angle, Matrix );
            VectorRotate ( p, Matrix, Position );
            VectorAdd ( o->Position, Position, Position );
            
            CreateEffect ( MODEL_FIRE, Position, o->Angle, Light, 8, NULL, 0 );
        }

        PlayBuffer ( SOUND_GREAT_POISON );
    }
}

void CreateMonsterSkill_Summon ( OBJECT* o, int AttackTime, BYTE time )
{
    if ( AttackTime>=time )
    {
        CreateEffect ( MODEL_CIRCLE, o->Position, o->Angle, o->Light, 3, o );
        CreateEffect ( MODEL_CIRCLE_LIGHT, o->Position, o->Angle, o->Light, 4 );
    }
}

void SetActionDestroy_Def ( OBJECT* o )
{
    if ( o->Type!=MODEL_PLAYER )
    {
		if( g_isCharacterBuff(o, eBuff_PhysDefense ) )
		{
            o->AI = ACTION_DESTROY_PHY_DEF;
			g_CharacterUnRegisterBuff( o, eBuff_PhysDefense );
		}
		else if( g_isCharacterBuff(o, eBuff_Defense) )
		{
            o->AI = ACTION_DESTROY_DEF;
            g_CharacterUnRegisterBuff( o, eBuff_Defense );
		}	
    }
}

void RenderDestroy_Def ( OBJECT* o, BMD* b )
{
    if ( o->Type!=MODEL_PLAYER )
    {
        if ( o->AI==ACTION_DESTROY_PHY_DEF )
        {
            b->RenderMeshEffect ( 6, MODEL_STONE_COFFIN, 1 );
            o->AI = 0;

            PlayBuffer(SOUND_HIT_CRISTAL);
        }
        else if ( o->AI==ACTION_DESTROY_DEF )
        {
            b->RenderMeshEffect ( 6, MODEL_STONE_COFFIN, 2 );
            o->AI = 0;

            PlayBuffer(SOUND_HIT_CRISTAL);
        }
    }
}

CHARACTER* CreateHellasMonster ( int Type, int PositionX, int PositionY, int Key )
{
    CHARACTER*  c = NULL;
    OBJECT*     o = &c->Object;
    switch ( Type )
    {
    case 144 :
    case 174:
    case 182:
    case 190:
    case 260:
    case 268:
	case 334:
    	OpenMonsterModel(63);
		c = CreateCharacter(Key,MODEL_MONSTER01+63,PositionX,PositionY);
		c->Weapon[0].Type = -1;
		c->Weapon[0].Level = 0;
		c->Object.Scale = 1.2f;
		o = &c->Object;
        o->BlendMesh = 1;
		strcpy(c->ID,"Àå¼ö°ÅºÏ");
        break;
    case 145 :
    case 175:
    case 183:
    case 191:
    case 261:
    case 269:
	case 336:
    	OpenMonsterModel(67);
		c = CreateCharacter(Key,MODEL_MONSTER01+67,PositionX,PositionY);
		c->Weapon[0].Type = MODEL_SPEAR+10;
		c->Weapon[0].Level = 7;
		c->Object.Scale = 1.5f;
		o = &c->Object;
        o->SubType   = 9;
        o->BlendMesh = 0;
		strcpy(c->ID,"´ëÇü ºí·ç³ªÀÌÆ®");
        break;
    case 146 :
    case 176:
    case 184:
    case 192:
    case 262:
    case 270:
	case 333:
    	OpenMonsterModel(65);
		c = CreateCharacter(Key,MODEL_MONSTER01+65,PositionX,PositionY);
		c->Weapon[0].Type = -1;
		c->Weapon[0].Level = 0;
		c->Object.Scale = 0.8f;
		o = &c->Object;
		strcpy(c->ID,"¶ø½ºÅÍ");
        break;
    case 147:
    case 177:
    case 185:
    case 193:
    case 263:
    case 271:
	case 331:
    	OpenMonsterModel(66);
		c = CreateCharacter(Key,MODEL_MONSTER01+66,PositionX,PositionY);
		c->Weapon[0].Type = -1;
		c->Weapon[0].Level = 0;
		c->Object.Scale = 1.4f;
		o = &c->Object;
        o->BlendMesh = 1;
		strcpy(c->ID,"°¡¿À¸®");
        break;
    case 148:
    case 178:
    case 186:
    case 194:
    case 264:
    case 272:
	case 332:
    	OpenMonsterModel(67);
		c = CreateCharacter(Key,MODEL_MONSTER01+67,PositionX,PositionY);
		c->Weapon[0].Type = MODEL_SPEAR+10;
		c->Weapon[0].Level = 7;
		c->Object.Scale = 1.f;
		o = &c->Object;
        o->BlendMesh = 0;
		strcpy(c->ID,"ºí·ç³ªÀÌÆ®");
        break;
    case 149:
    case 179:
    case 187:
    case 195:
    case 265:
    case 273:
	case 335:
    	OpenMonsterModel(68);
		c = CreateCharacter(Key,MODEL_MONSTER01+68,PositionX,PositionY);
		c->Weapon[0].Type = -1;
		c->Weapon[0].Level = 7;
		c->Object.Scale = 1.2f;
		o = &c->Object;
        o->BlendMesh = 3;
		strcpy(c->ID,"¸¶¸°º¸ÀÌ");
        break;
    case 160:
    case 180:
    case 188:
    case 196:
    case 266:
    case 274:
	case 337:
    	OpenMonsterModel(69);
		c = CreateCharacter(Key,MODEL_MONSTER01+69,PositionX,PositionY);
		c->Weapon[0].Type = MODEL_SWORD+13;
		c->Weapon[0].Level = 0;
		c->Weapon[1].Type = MODEL_SWORD+13;
		c->Weapon[1].Level = 0;
		c->Object.Scale = 1.2f;
		o = &c->Object;
		strcpy(c->ID,"ÄïµÐÈÄº¸");
        break;
    case 161:
    case 181:
    case 189:
    case 197:
    case 267:
	case 338:
    	OpenMonsterModel(69);
		c = CreateCharacter(Key,MODEL_MONSTER01+69,PositionX,PositionY);
		c->Weapon[0].Type = MODEL_SWORD+13;
		c->Weapon[0].Level = 0;
		c->Weapon[1].Type = MODEL_SWORD+13;
		c->Weapon[1].Level = 0;
		o = &c->Object;
        o->SubType  = 9;
		o->Scale    = 1.5f;
		strcpy(c->ID,"ÄïµÐÈÄº¸");
        break;

    case 275:
    	OpenMonsterModel(64);
		c = CreateCharacter(Key,MODEL_MONSTER01+64,PositionX,PositionY);
		c->Weapon[1].Type = MODEL_STAFF+11;
		c->Weapon[1].Level = 0;
		c->Object.Scale = 2.0f;
//		c->Object.Scale = 1.9f;
		o = &c->Object;
		strcpy(c->ID,"ÁøÂ¥ÄïµÐ");
		o->LifeTime = 100;
        break;
    }

    return c;
}

bool    SettingHellasMonsterLinkBone ( CHARACTER* c, int Type )
{
    switch ( Type )
    {
    case MODEL_MONSTER01+64:
		c->Weapon[0].LinkBone = 29;
		c->Weapon[1].LinkBone = 49;
        return true;
    case MODEL_MONSTER01+66:
		c->Weapon[0].LinkBone = 13;
		c->Weapon[1].LinkBone = 14;
        return true;
    case MODEL_MONSTER01+67:
		c->Weapon[0].LinkBone = 56;
		c->Weapon[1].LinkBone = 42;
        return true;
    case MODEL_MONSTER01+68:
		c->Weapon[0].LinkBone = 60;
		c->Weapon[1].LinkBone = 60;
        return true;
    case MODEL_MONSTER01+69:
		c->Weapon[0].LinkBone = 41;
		c->Weapon[1].LinkBone = 51;
        return true;
    }

    return false;
}

bool SetCurrentAction_HellasMonster ( CHARACTER* c, OBJECT* o )
{
    switch ( c->MonsterIndex )
    {
    case 145:
    case 175:
    case 183:
    case 191:
    case 261:
    case 269:
        switch ( ( c->Skill ) )
        {
        case AT_SKILL_ENERGYBALL:
		case AT_SKILL_BLOOD_ATT_UP:
		case AT_SKILL_BLOOD_ATT_UP+1:
		case AT_SKILL_BLOOD_ATT_UP+2:
		case AT_SKILL_BLOOD_ATT_UP+3:
		case AT_SKILL_BLOOD_ATT_UP+4:
        case AT_SKILL_REDUCEDEFENSE:
        case AT_SKILL_POISON:
        case AT_SKILL_MONSTER_SUMMON :
        case AT_SKILL_MONSTER_MAGIC_DEF :
        case AT_SKILL_MONSTER_PHY_DEF :
            SetAction ( o, MONSTER01_ATTACK2 );
            break;

        default :
            SetAction ( o, MONSTER01_ATTACK1 );
            break;
        }
        return true;

    case 147:
    case 177:
    case 185:
    case 193:
    case 263:
    case 271:
        switch ( ( c->Skill ) )
        {
        case AT_SKILL_ENERGYBALL:
            SetAction ( o, MONSTER01_ATTACK2 );
            break;

        default :
            SetAction ( o, MONSTER01_ATTACK1 );
            break;
        }
        return true;

    case 148:
    case 178:
    case 186:
    case 194:
    case 264:
    case 272:
        switch ( ( c->Skill ) )
        {
        case AT_SKILL_ENERGYBALL:
            SetAction ( o, MONSTER01_ATTACK2 );
            break;

        default :
            SetAction ( o, MONSTER01_ATTACK1 );
            break;
        }
        return true;

    case 149:
    case 179:
    case 187:
    case 195:
    case 265:
    case 273:
        switch ( ( c->Skill ) )
        {
        case AT_SKILL_POISON:
            SetAction ( o, MONSTER01_ATTACK2 );
            break;

        case AT_SKILL_ENERGYBALL:
            SetAction ( o, MONSTER01_ATTACK1 );
            break;
        }
        return true;

    case 160:
    case 180:
    case 188:
    case 196:
    case 266:
    case 274:
    case 161:
    case 181:
    case 189:
    case 197:
    case 267:
        SetAction ( o, MONSTER01_ATTACK1+rand()%2 );
        return true;

    case 275:
        SetAction ( o, MONSTER01_ATTACK1+rand()%2 );
        return true;
    }
    return false;
}

bool AttackEffect_HellasMonster ( CHARACTER* c, CHARACTER* tc, OBJECT* o, OBJECT* to, BMD* b )
{
	vec3_t Light;
	vec3_t p, Position;
	Vector ( 0.f, 0.f, 0.f, p );
	Vector ( 1.f, 1.f, 1.f, Light );
    switch ( c->MonsterIndex )
    {
    case 144:
    case 174:
    case 182:
    case 190:
    case 260:
    case 268:
    case 276:
        if ( c->AttackTime==14 )
        {
            Vector ( 1.f, 1.f, 1.f, Light );

            if ( to!=NULL )
            {
                VectorCopy ( to->Position, Position );
            }
            else
            {
                VectorCopy ( Hero->Object.Position, Position );
            }
            Position[2] += 150.f;
            CreateParticle ( BITMAP_SHINY+4, Position, o->Angle, Light, 1, 1.f );
        }
        return true;

    case 145:
    case 175:
    case 183:
    case 191:
    case 261:
    case 269:
    case 277:
        switch ( ( c->Skill ) )
        {
		case AT_SKILL_BLOOD_ATT_UP:
		case AT_SKILL_BLOOD_ATT_UP+1:
		case AT_SKILL_BLOOD_ATT_UP+2:
		case AT_SKILL_BLOOD_ATT_UP+3:
		case AT_SKILL_BLOOD_ATT_UP+4:
        case AT_SKILL_REDUCEDEFENSE:
            CreateMonsterSkill_ReduceDef ( o, c->AttackTime, 13, 200.f );
            break;

        case AT_SKILL_POISON:
            CreateMonsterSkill_Poison ( o, c->AttackTime, 13 );
            break;

        case AT_SKILL_MONSTER_SUMMON:
            CreateMonsterSkill_Summon ( o, c->AttackTime, 13 );
            break;

        case AT_SKILL_MONSTER_MAGIC_DEF:
            if ( c->AttackTime>=13 )
            {
				g_CharacterRegisterBuff(o, eBuff_PhysDefense);
                c->AttackTime = 15;
                PlayBuffer ( SOUND_GREAT_SHIELD );
            }
            break;

        case AT_SKILL_MONSTER_PHY_DEF:
            if ( c->AttackTime>=13 )
            {
				g_CharacterRegisterBuff(o, eBuff_Defense);
                c->AttackTime = 15;
                PlayBuffer ( SOUND_GREAT_SHIELD );
            }
            break;

        default :
            break;
        }
        if ( o->CurrentAction==MONSTER01_ATTACK2 && c->AttackTime==14 )
        {
      		CreateEffect ( MODEL_SKILL_FURY_STRIKE, o->Position, o->Angle, o->Light, 0, o, -1, 0, 1 );
      		CreateEffect ( BITMAP_JOINT_THUNDER+1, o->Position, o->Angle, o->Light, 0, o, -1, 0, 1 );
            c->AttackTime = 15;
        }
        return true;

    case 146:
    case 176:
    case 184:
    case 192:
    case 262:
    case 270:
    case 278:
        if ( c->AttackTime==14 )
        {
            Vector ( 1.f, 1.f, 1.f, Light );

            if ( to!=NULL )
            {
                VectorCopy ( to->Position, Position );
            }
            else
            {
                VectorCopy ( Hero->Object.Position, Position );
            }
            Position[2] += 150.f;
            CreateParticle ( BITMAP_SHINY+4, Position, o->Angle, Light, 1, 1.f );
        }
        return true;

    case 147:
    case 177:
    case 185:
    case 193:
    case 263:
    case 271:
    case 279:
        if ( o->CurrentAction==MONSTER01_ATTACK2 && c->AttackTime==14 )
        {
			CreateEffect ( MODEL_WATER_WAVE, o->Position, o->Angle, o->Light );
        }
        return true;

    case 148:
    case 178:
    case 186:
    case 194:
    case 264:
    case 272:
    case 280:
        switch ( ( c->Skill ) )
        {
        case AT_SKILL_ENERGYBALL:
            if ( c->AttackTime==14 )
            {
                CreateEffect ( MODEL_SKILL_FURY_STRIKE, o->Position, o->Angle, o->Light, 1, o, -1, 0, 1 );
            }
            break;

        default:
            break;
        }
        return true;

    case 149:
    case 179:
    case 187:
    case 195:
    case 265:
    case 273:
    case 281:
        switch ( ( c->Skill ) )
        {
        case AT_SKILL_POISON:
            if ( c->AttackTime==14 )
            {
                vec3_t Light, Position;
                
                Vector ( 0.8f, 0.5f, 0.1f, Light );
                
                for ( int i=0; i<3; ++i )
                {
                    Position[0] = Hero->Object.Position[0] + (rand()%200-100);
                    Position[1] = Hero->Object.Position[1] + (rand()%200-100);
                    Position[2] = Hero->Object.Position[2];
                    
                    CreateEffect(MODEL_FIRE, Position,o->Angle,Light,7,NULL,0);
                }
            }
            break;

        case AT_SKILL_ENERGYBALL:
            if ( c->AttackTime==14 )
            {
                if ( c->TargetCharacter>=0 && c->TargetCharacter<MAX_CHARACTERS_CLIENT )
                {
                    CHARACTER *tc = &CharactersClient[c->TargetCharacter];
                    OBJECT *to = &tc->Object;
                    
                    vec3_t Angle;
                    Vector ( 0.f, 0.f, 0.f, p );
                    VectorCopy ( o->Angle, Angle );
                    b->TransformPosition ( o->BoneTransform[60], p, Position, true );
                    
                    CreateJoint ( BITMAP_FLARE+1, Position, to->Position, Angle, 6, to, 30.f, 50 );
                    Angle[2] -= 30.f;                                                   
                    CreateJoint ( BITMAP_FLARE+1, Position, to->Position, Angle, 6, to, 30.f, 50 );
                    Angle[2] += 15.f;
                    CreateJoint ( BITMAP_FLARE+1, Position, to->Position, Angle, 6, to, 30.f, 50 );
                }
            }
            break;
        }
        return true;

    case 160:
    case 180:
    case 188:
    case 196:
    case 266:
    case 274:
        if ( o->CurrentAction==MONSTER01_ATTACK1 && c->AttackTime>=13 )
        {
            CreateEffect ( MODEL_SKILL_INFERNO, o->Position, o->Angle, o->Light, 5, o );
            CreateEffect ( BITMAP_FLAME, o->Position, o->Angle, o->Light, 2, o );
            c->AttackTime = 15;
        }
        return true;

    case 161:
    case 181:
    case 189:
    case 197:
    case 267:
        switch ( ( c->Skill ) )
        {
		case AT_SKILL_BLOOD_ATT_UP:
		case AT_SKILL_BLOOD_ATT_UP+1:
		case AT_SKILL_BLOOD_ATT_UP+2:
		case AT_SKILL_BLOOD_ATT_UP+3:
		case AT_SKILL_BLOOD_ATT_UP+4:
        case AT_SKILL_REDUCEDEFENSE:
            CreateMonsterSkill_ReduceDef ( o, c->AttackTime, 13, 200.f );
            break;

        case AT_SKILL_POISON:
            CreateMonsterSkill_Poison ( o, c->AttackTime, 13 );
            break;

        case AT_SKILL_MONSTER_SUMMON:
            CreateMonsterSkill_Summon ( o, c->AttackTime, 13 );
            break;

        case AT_SKILL_MONSTER_MAGIC_DEF:
            if ( c->AttackTime>=13 )
            {
				g_CharacterRegisterBuff(o, eBuff_PhysDefense);
                c->AttackTime = 15;
                PlayBuffer ( SOUND_GREAT_SHIELD );
            }
            break;

        case AT_SKILL_MONSTER_PHY_DEF:
            if ( c->AttackTime>=13 )
            {
				g_CharacterRegisterBuff(o, eBuff_Defense);
                c->AttackTime = 15;
                PlayBuffer ( SOUND_GREAT_SHIELD );
            }
            break;

        default :
            break;
        }

        if ( o->CurrentAction==MONSTER01_ATTACK1 )
        {
            if ( c->AttackTime==7 )
            {
                CreateEffect ( MODEL_SKILL_FURY_STRIKE, o->Position, o->Angle, o->Light, 0, o, -1, 0, 0 );
            }
            else if ( c->AttackTime>=13 )
            {
                CreateEffect ( MODEL_SKILL_INFERNO, o->Position, o->Angle, o->Light, 0, o );
                CreateEffect ( BITMAP_FLAME, o->Position, o->Angle, o->Light, 1, o );
                c->AttackTime = 15;
            }
        }
        return true;
    case 275:
        switch ( ( c->Skill ) )
        {
		case AT_SKILL_BLOOD_ATT_UP:
		case AT_SKILL_BLOOD_ATT_UP+1:
		case AT_SKILL_BLOOD_ATT_UP+2:
		case AT_SKILL_BLOOD_ATT_UP+3:
		case AT_SKILL_BLOOD_ATT_UP+4:
        case AT_SKILL_REDUCEDEFENSE:
            CreateMonsterSkill_ReduceDef ( o, c->AttackTime, 13, 200.f );
            break;

        case AT_SKILL_POISON:
            CreateMonsterSkill_Poison ( o, c->AttackTime, 13 );
            break;

        case AT_SKILL_MONSTER_SUMMON:
            CreateMonsterSkill_Summon ( o, c->AttackTime, 13 );
            break;

        case AT_SKILL_MONSTER_MAGIC_DEF:
            if ( c->AttackTime>=13 )
            {
				g_CharacterRegisterBuff(o, eBuff_PhysDefense);
                c->AttackTime = 15;

                PlayBuffer ( SOUND_GREAT_SHIELD );
            }
            break;

        case AT_SKILL_MONSTER_PHY_DEF:
            if ( c->AttackTime>=13 )
            {
				g_CharacterRegisterBuff(o, eBuff_Defense);
                c->AttackTime = 15;

                PlayBuffer ( SOUND_GREAT_SHIELD );
            }
            break;

        default :
            break;
        }
        return true;
    }
    return false;
}

void MonsterMoveWaterSmoke ( OBJECT* o )
{
	if(o->CurrentAction == MONSTER01_WALK)
	{
		vec3_t Position;
		Vector(o->Position[0]+rand()%200-100,o->Position[1]+rand()%200-100,o->Position[2],Position);
		CreateParticle(BITMAP_SMOKE+1,Position,o->Angle,o->Light);
	}
}
void MonsterDieWaterSmoke(OBJECT *o)
{
	if(o->CurrentAction==MONSTER01_DIE && o->AnimationFrame>=8.f && o->AnimationFrame<9.f)
	{
		vec3_t Position;
		for(int i=0;i<20;i++)
		{
			Vector(1.f,1.f,1.f,o->Light);
			Vector(o->Position[0]+(float)(rand()%64-32),
				o->Position[1]+(float)(rand()%64-32),
				o->Position[2]+(float)(rand()%32-16),Position);
			CreateParticle(BITMAP_SMOKE+1,Position,o->Angle,o->Light,1);
		}
	}
}

bool MoveHellasMonsterVisual( OBJECT* o, BMD* b )
{
    vec3_t Position, p;
    vec3_t Light;

    switch ( o->Type )
    {
    case MODEL_WARCRAFT:
        if ( o->CurrentAction==0 )
        {
            Position[0] = o->Position[0]+rand()%200-100;
            Position[1] = o->Position[1]+rand()%100-50;
            Position[2] = o->Position[2];
            
            CreateParticle ( BITMAP_SMOKE+1, Position, o->Angle, o->Light );
            CreateParticle ( BITMAP_SMOKE+1, Position, o->Angle, o->Light );
            CreateEffect ( MODEL_STONE1,o->Position,o->Angle,o->Light);
            CreateEffect ( MODEL_STONE2,o->Position,o->Angle,o->Light);
        }
        o->BlendMesh = 1;
        o->BlendMeshLight = sinf ( WorldTime*0.001f )*0.5f+0.5f;
		return true;
        
    case MODEL_MONSTER01+63:
        o->BlendMeshLight = sinf ( WorldTime*0.001f )*0.7f+0.3f;
        return true;

    case MODEL_MONSTER01+64:
        if(rand()%2==0)
        {
            Vector(2.f,30.f,0.f,p);
            b->TransformPosition(o->BoneTransform[6],p,Position,true);
            Vector(1.0f,0.2f,0.0f,Light);
            CreateParticle(BITMAP_SMOKE,Position,o->Angle,Light,17);
        }
        return true;

    case MODEL_MONSTER01+65:
        return true;

    case MODEL_MONSTER01+66:
        return true;

    case MODEL_MONSTER01+67:
        return true;

    case MODEL_MONSTER01+68:
        o->BlendMeshLight = sinf ( WorldTime*0.001f )*0.7f+0.3f;
        return true;

    case MODEL_MONSTER01+69:
        if ( o->CurrentAction!=MONSTER01_DIE )
        {
            if(rand()%2==0)
            {
                Vector ( 2.f, 30.f, 0.f, p );
                b->TransformPosition ( o->BoneTransform[31], p, Position, true );
                if ( o->SubType==9 )
                {
                    Vector ( 1.0f, 0.0f, 0.0f, Light );
                }
                else
                {
                    Vector ( 0.0f, 0.3f, 1.0f, Light );
                }
                CreateParticle ( BITMAP_SMOKE, Position, o->Angle, Light, 17 );
            }
        }
        return true;
    }

    return false;
}

bool RenderHellasMonsterVisual ( CHARACTER* c, OBJECT* o, BMD* b )
{
    vec3_t Position, Light, p;
    float  Luminosity;
    int    i;

    switch ( o->Type )
    {
    case MODEL_WARCRAFT:
        if ( o->CurrentAction==1 )
        {
            VectorCopy ( o->Position, Position );
            Position[2] += 20.f;

            if ( o->LifeTime==0 )
            {
                Vector ( 0.f, 0.f, 0.f, p );
			    b->TransformPosition ( o->BoneTransform[8], p, Position, true );
				Vector(1.f, 0.1f, 0.1f, Light);
				CreateParticle ( BITMAP_HOLE, Position, o->Angle, Light, 0, 3.f );
            }
            o->LifeTime++;
            if ( o->LifeTime>=5 )
            {
                o->LifeTime = 0;
            }
        }
        return true;

    case MODEL_MONSTER01+63:
		Vector ( 0.f, 0.f, 0.f, p );

        if ( o->CurrentAction!=MONSTER01_DIE )
        {
            Luminosity = 0.f;
            for ( i=0; i<5; ++i )
            {
                Luminosity += 1/4.f;
			    Vector ( Luminosity*0.1f, Luminosity*0.4f, Luminosity, Light );

    			b->TransformPosition ( o->BoneTransform[i+74], p, Position, true );
			    CreateSprite(BITMAP_LIGHT,Position,1.5f-(0.2f*i),Light,o,WorldTime);
			    CreateSprite(BITMAP_LIGHT,Position,1.5f-(0.2f*i),Light,o,WorldTime);
                if ( (rand()%2)==0 )
                {
                    CreateParticle(BITMAP_BUBBLE,Position,o->Angle,Light, 1);
                }

                b->TransformPosition ( o->BoneTransform[i+62], p, Position, true );
			    CreateSprite(BITMAP_LIGHT,Position,1.5f-(0.2f*i),Light,o,WorldTime);
			    CreateSprite(BITMAP_LIGHT,Position,1.5f-(0.2f*i),Light,o,WorldTime);
                if ( (rand()%2)==0 )
                {
        		    CreateParticle(BITMAP_BUBBLE,Position,o->Angle,Light, 1);
                }
            }

		    Vector ( 2.f, 2.f, 0.f, p );
            Vector ( o->BlendMeshLight, o->BlendMeshLight, o->BlendMeshLight, Light );
		    b->TransformPosition ( o->BoneTransform[8], p, Position, true );
		    CreateSprite(BITMAP_ENERGY,Position,0.2f,Light,o,0.f);
            Vector ( o->BlendMeshLight*0.1f, o->BlendMeshLight*0.1f, o->BlendMeshLight, Light );
		    CreateSprite(BITMAP_SHINY+1,Position,1.f,Light,o,0.f);

		    Vector ( 0.f, 0.f, 0.f, p );
		    b->TransformPosition ( o->BoneTransform[22], p, Position, true );
		    CreateSprite(BITMAP_ENERGY,Position,0.2f,Light,o,0.f);
            Vector ( o->BlendMeshLight*0.1f, o->BlendMeshLight*0.1f, o->BlendMeshLight, Light );
		    CreateSprite(BITMAP_SHINY+1,Position,1.f,Light,o,0.f);

            b->TransformPosition ( o->BoneTransform[15], p, Position, true );
		    CreateSprite(BITMAP_ENERGY,Position,0.2f,Light,o,0.f);
            Vector ( o->BlendMeshLight*0.1f, o->BlendMeshLight*0.1f, o->BlendMeshLight, Light );
		    CreateSprite(BITMAP_SHINY+1,Position,1.f,Light,o,0.f);
        }

        Luminosity = sinf ( WorldTime*0.001f )*0.2f+0.4f;
        for ( i=0 ; i<5; ++i )
        {
			Vector ( Luminosity*0.1f, Luminosity*0.4f, Luminosity, Light );

			b->TransformPosition ( o->BoneTransform[i+51], p, Position, true );
			CreateSprite(BITMAP_LIGHT,Position,2.f,Light,o,WorldTime);

			b->TransformPosition ( o->BoneTransform[i+29], p, Position, true );
			CreateSprite(BITMAP_LIGHT,Position,1.5f,Light,o,WorldTime);

        }
        return true;

	case MODEL_MONSTER01+64:

        if ( o->CurrentAction==MONSTER01_DIE )
        {
			c->Weapon[1].Type = -1;
			c->Weapon[1].Level = -1;
		}
		else
		{
			// ´«
			Luminosity = (float)sin ( WorldTime*0.003f )*0.2f+0.8f;
			Vector(0,0,0,p);
			Vector ( Luminosity*1.0f, Luminosity*0.0f, Luminosity*0.0f, Light );
			b->TransformPosition ( o->BoneTransform[8], p, Position, true );
			CreateSprite(BITMAP_ENERGY,Position,0.4f,Light,o,0.f);

			Vector(0,0,0,p);
			Vector ( Luminosity*1.0f, Luminosity*0.0f, Luminosity*0.0f, Light );
			b->TransformPosition ( o->BoneTransform[9], p, Position, true );
			CreateSprite(BITMAP_ENERGY,Position,0.4f,Light,o,0.f);

            Vector(0,0,0,p);
			{
				Vector ( 1.f, 1.f, 1.f, Light );
				b->TransformPosition ( o->BoneTransform[100], p, Position, true );
				float fRoar = 1.0f;
				if ( o->CurrentAction==MONSTER01_SHOCK )
				{
					if (o->AnimationFrame > 10.0f && o->PKKey > 10)
						o->PKKey -= 1;
					else if (o->AnimationFrame > 4.0f)
						o->PKKey += 1;
					else o->PKKey = 10;

					fRoar = o->PKKey * 0.15f;
					if (o->AnimationFrame > 4.0f && o->AnimationFrame < 6.0f)
					{
						if (o->AnimationFrame < 5.0f) o->Skill += 3;
						else if (o->Skill > 3) o->Skill -= 3;
						fRoar += o->Skill * 0.15f;
					}
					else if (o->AnimationFrame > 8.0f && o->AnimationFrame < 10.0f)
					{
						if (o->AnimationFrame < 9.0f) o->Skill += 3;
						else if (o->Skill > 3) o->Skill -= 3;
						fRoar += o->Skill * 0.15f;
					}
					else
					{
						o->Skill = 0;
					}
					CreateSprite ( BITMAP_FLARE_BLUE, Position, (1.2f+(sinf(WorldTime*0.001f)*0.3f)), Light, o, 0.f );
					CreateSprite ( BITMAP_FLARE_RED, Position, (1.2f+(sinf(WorldTime*0.001f)*0.3f))*fRoar, Light, o, 0.f );
				}
				else
				{
					o->PKKey = 0;
					fRoar = 1.0f;
					CreateSprite ( BITMAP_FLARE_BLUE, Position, (1.2f+(sinf(WorldTime*0.001f)*0.3f)), Light, o, 0.f );
				}
				if ( (rand()%2)==0 )
				{
					CreateParticle ( BITMAP_SMOKE, Position, o->Angle, Light, 13, fRoar );
				}
			}
			
			if ( o->CurrentAction==MONSTER01_SHOCK )
			{
				if (o->AnimationFrame < 2.0f)
				{
					if ( o->LifeTime!=100 )
                    {
                        o->LifeTime = 100;

                        PlayBuffer ( SOUND_KUNDUN_ROAR );
                    }
				}
				if (o->LifeTime == 100 && o->AnimationFrame > 4.0f)
				{
					o->LifeTime = 101;
					if ( gMapManager.InHellas() )
					{
		                AddWaterWave( (c->PositionX), (c->PositionY), 2, 2000 );
					}
				}
				if (o->AnimationFrame > 3.0f)
				{
					EarthQuake = (float)(rand()%8-8)*0.2f;
				}
				if (gMapManager.InHellas() && o->AnimationFrame > 3.0f && o->AnimationFrame < 14.0f)
				{
					vec3_t Position, Light;
					Vector ( 0.3f, 0.8f, 1.f, Light );
					vec3_t Angle = { 0.f, 0.f, 0.f };
					for (int i = 0; i < 2; ++i)
					{
						float fAngle = float(rand()%360);
						float fDistance = float(rand()%600+200);
						Position[0] = o->Position[0] + sinf(fAngle)*fDistance;
						Position[1] = o->Position[1] + cosf(fAngle)*fDistance;
						Position[2] = o->Position[2]+800.f;
						CreateEffect ( 9, Position, Angle, Light );
					}
				}
			}
			if ( o->CurrentAction==MONSTER01_ATTACK1 )
			{
				if (o->AnimationFrame < 2.0f)
				{
					o->LifeTime = 100;
				}
				if (o->LifeTime == 100 && o->AnimationFrame > 3.0f)
				{
					o->LifeTime = 101;
					vec3_t Position;
					b->TransformPosition ( o->BoneTransform[49], p, Position, true );
					float fHeight = Position[2];
					Vector(o->Position[0], o->Position[1], fHeight, Position);
					CreateEffect(MODEL_CUNDUN_SKILL, Position, o->Angle, o->Light, 0);
				}
			}
			if ( o->CurrentAction==MONSTER01_ATTACK2 )
			{
				if (o->AnimationFrame < 2.0f)
				{
					o->LifeTime = 100;
				}
				if (o->LifeTime == 100 && o->AnimationFrame > 5.0f)
				{
					o->LifeTime = 101;

					vec3_t Position;
					Vector(0.f,0.f,0.f,p);
					b->TransformPosition ( o->BoneTransform[49], p, Position, true );
					Position[2] = 400;
					CreateEffect(MODEL_CUNDUN_SKILL, Position, o->Angle, o->Light, 1);
				}
				if (o->LifeTime == 101 && o->AnimationFrame > 6.0f)
				{
					o->LifeTime = 102;

					vec3_t Position,Angle;
					Vector(0.f,0.f,0.f,p);
					b->TransformPosition ( o->BoneTransform[49], p, Position, true );
					Position[2] = 400;
					Angle[0] = 0;
					Angle[1] = 0;

					for (i = 0; i < 24; ++i)
					{
						Angle[2] = ( float)(i*30);
						CreateJoint(BITMAP_JOINT_SPIRIT2,Position,Position,Angle,14,NULL,100.f,0,0);
					}
					if ( gMapManager.InHellas() )
					{
		                AddWaterWave( (c->PositionX), (c->PositionY), 2, 2000 );
					}
				}
				if (o->LifeTime == 102 && o->AnimationFrame > 9.0f)
				{
					o->LifeTime = 103;
					if ( gMapManager.InHellas() )
					{
		                AddWaterWave( (c->PositionX), (c->PositionY), 2, 2000 );
					}
				}
				if (o->AnimationFrame > 6.0f)
				{
					EarthQuake = (float)(rand()%16-16)*0.1f;
				}
			}
		}
		return true;

    case MODEL_MONSTER01+65:
        if ( o->CurrentAction!=MONSTER01_DIE )
        {
            Luminosity = (float)sin ( WorldTime*0.003f )*0.2f+0.8f;
		    Vector(20.f,30.f,-7.f,p);
		    Vector ( Luminosity*1.0f, Luminosity*0.9f, Luminosity*0.9f, Light );
		    b->TransformPosition ( o->BoneTransform[6], p, Position, true );
		    CreateSprite(BITMAP_ENERGY,Position,0.2f,Light,o,0.f);
		    Vector ( Luminosity*1.0f, Luminosity*0.3f, Luminosity*0.1f, Light );
		    CreateSprite(BITMAP_SHINY+1,Position,1.f,Light,o,0.f);

		    Vector(20.f,30.f,7.f,p);
		    Vector ( Luminosity*1.0f, Luminosity*0.9f, Luminosity*0.9f, Light );
		    b->TransformPosition ( o->BoneTransform[6], p, Position, true );
		    CreateSprite(BITMAP_ENERGY,Position,0.2f,Light,o,0.f);
		    Vector ( Luminosity*1.0f, Luminosity*0.3f, Luminosity*0.1f, Light );
		    CreateSprite(BITMAP_SHINY+1,Position,1.f,Light,o,0.f);

		    Vector(0.f,0.f,7.f,p);
		    Vector ( Luminosity*1.0f, Luminosity*0.9f, Luminosity*0.9f, Light );
		    b->TransformPosition ( o->BoneTransform[13], p, Position, true );
		    CreateSprite(BITMAP_ENERGY,Position,0.1f,Light,o,0.f);
		    Vector ( Luminosity*1.0f, Luminosity*0.5f, Luminosity*0.1f, Light );
		    CreateSprite(BITMAP_SHINY+1,Position,0.7f,Light,o,0.f);

		    Vector(0.f,0.f,-7.f,p);
		    Vector ( Luminosity*1.0f, Luminosity*0.9f, Luminosity*0.9f, Light );
		    b->TransformPosition ( o->BoneTransform[19], p, Position, true );
		    CreateSprite(BITMAP_ENERGY,Position,0.1f,Light,o,0.f);
		    Vector ( Luminosity*1.0f, Luminosity*0.5f, Luminosity*0.1f, Light );
		    CreateSprite(BITMAP_SHINY+1,Position,0.7f,Light,o,0.f);

		    Vector ( Luminosity*1.0f,Luminosity*0.8f,Luminosity*0.8f, Light );
		    Vector ( 0,0,0, p );
		    b->TransformPosition ( o->BoneTransform[23], p, Position, true );
		    CreateSprite(BITMAP_LIGHT,Position,3,Light,o,0.f);
		    Vector ( -30,-25,0, p );
		    b->TransformPosition ( o->BoneTransform[25], p, Position, true );
		    CreateSprite(BITMAP_LIGHT,Position,2.5f,Light,o,0.f);
		    Vector ( 0,0,0, p );
		    b->TransformPosition ( o->BoneTransform[31], p, Position, true );
		    CreateSprite(BITMAP_LIGHT,Position,3,Light,o,0.f);
		    Vector ( 30,25,0, p );
		    b->TransformPosition ( o->BoneTransform[33], p, Position, true );
		    CreateSprite(BITMAP_LIGHT,Position,2.5f,Light,o,0.f);
        }
		return true;

    case MODEL_MONSTER01+66:
        if ( o->CurrentAction!=MONSTER01_DIE )
        {
            Luminosity = (float)sin ( WorldTime*0.005f )*0.15f+0.85f;
		    Vector ( 4.f, 0.f, 5.f, p );
		    Vector ( Luminosity*1.0f, Luminosity*1.0f, Luminosity*1.0f, Light );
		    b->TransformPosition ( o->BoneTransform[9], p, Position, true );
		    CreateSprite(BITMAP_ENERGY,Position,0.1f,Light,o,0.f);
		    Vector ( Luminosity*0.3f, Luminosity*0.6f, Luminosity*1.f, Light );
		    CreateSprite(BITMAP_SHINY+1,Position,1.f,Light,o,0.f);

		    Vector ( 4.f, 0.f, 5.f, p );
		    Vector ( Luminosity*1.0f, Luminosity*1.0f, Luminosity*1.0f, Light );
		    b->TransformPosition ( o->BoneTransform[10], p, Position, true );
		    CreateSprite(BITMAP_ENERGY,Position,0.1f,Light,o,0.f);
		    Vector ( Luminosity*0.3f, Luminosity*0.6f, Luminosity*1.f, Light );
		    CreateSprite(BITMAP_SHINY+1,Position,1.f,Light,o,0.f);

		    Vector ( 0, 0, 0,  p );
		    Vector ( 0.7f, 0.6f, 1, Light );
		    b->TransformPosition ( o->BoneTransform[3], p, Position, true );
		    CreateSprite ( BITMAP_LIGHT, Position, 1.7f, Light, o, 0.f );
		    CreateSprite ( BITMAP_SHINY+1, Position, 1.f, Light, o, (float)(rand()%360) );
		    b->TransformPosition ( o->BoneTransform[4], p, Position, true );
		    CreateSprite ( BITMAP_LIGHT, Position, 1.7f, Light, o, 0.f );
		    CreateSprite ( BITMAP_SHINY+1, Position, 1.f, Light, o, (float)(rand()%360) );

		    vec3_t pos1, pos2;
		    Vector ( 0,0,0, p );
		    if ( o->CurrentAction == MONSTER01_ATTACK1 )
		    {
			    for( i=10; i<16; ++i )
			    {
				    b->TransformPosition(o->BoneTransform[i],p,pos1,true);
    			    b->TransformPosition(o->BoneTransform[i+1],p,pos2,true);
				    CreateJoint(BITMAP_JOINT_THUNDER,pos1,pos2,o->Angle,7,NULL,30.f);
			    }
			    for( i=31; i<37; ++i )
			    {
				    b->TransformPosition(o->BoneTransform[i],p,pos1,true);
    			    b->TransformPosition(o->BoneTransform[i+1],p,pos2,true);
				    CreateJoint(BITMAP_JOINT_THUNDER,pos1,pos2,o->Angle,7,NULL,30.f);
			    }

		    }
        }
		return true;

    case MODEL_MONSTER01+67:
        if ( o->CurrentAction!=MONSTER01_DIE )
        {
            Vector ( 0.f, 0.f, 30.f, p );
            
            if ( c->MonsterIndex==145 )
            {
                Vector ( 1.f, 0.f, 0.f, Light );
                b->TransformPosition ( o->BoneTransform[0], p, Position, true );
                CreateSprite ( BITMAP_FLARE_BLUE, Position, 2.f+(sinf(WorldTime*0.001f)*0.3f), Light, o, 0.f );
                
                if ( (rand()%2)==0 )
                {
                    CreateParticle ( BITMAP_SMOKE, Position, o->Angle, Light, 13 );
                }
            }
            else
            {
                Vector ( 1.f, 1.f, 1.f, Light );
                b->TransformPosition ( o->BoneTransform[0], p, Position, true );
                CreateSprite ( BITMAP_FLARE_BLUE, Position, 1.2f+(sinf(WorldTime*0.001f)*0.3f), Light, o, 0.f );
            }
            
            Vector ( 5.f, 0.f, 0.f, p );
            Vector ( 1.f, 1.f, 1.f, Light );
            b->TransformPosition ( o->BoneTransform[28], p, Position, true );
            CreateSprite ( BITMAP_LIGHT, Position, 0.3f+(sinf(WorldTime*0.001f)*0.2f), Light, o, 0.f );
            b->TransformPosition ( o->BoneTransform[29], p, Position, true );
            CreateSprite ( BITMAP_LIGHT, Position, 0.3f+(sinf(WorldTime*0.001f)*0.2f), Light, o, 0.f );
        }
        else
        {
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;
        }
        return true;

    case MODEL_MONSTER01+68:
        if ( o->CurrentAction!=MONSTER01_DIE )
        {
            vec3_t pos;
            vec3_t Angle;

			Vector ( 5.f, 0.f, 0.f, p );
            Vector ( 1.f, 1.f, 1.f, Light );
            VectorCopy ( o->Angle, Angle );
            b->TransformPosition ( o->BoneTransform[9], p, Position, true );
			CreateSprite ( BITMAP_LIGHT, Position, 0.3f+(sinf(WorldTime*0.001f)*0.2f), Light, o, 0.f );
			b->TransformPosition ( o->BoneTransform[10], p, Position, true );
			CreateSprite ( BITMAP_LIGHT, Position, 0.3f+(sinf(WorldTime*0.001f)*0.2f), Light, o, 0.f );

			Vector ( 0.f, 0.f, 0.f, p );
            Luminosity = (float)sin ( WorldTime*0.002f )*0.3f+0.6f;

            if ( o->CurrentAction==MONSTER01_ATTACK2 )
            {
                Vector ( Luminosity*0.1f, Luminosity, Luminosity*0.1f, Light );
            }
            else
            {
                Vector ( Luminosity, Luminosity, Luminosity, Light );
            }
			b->TransformPosition ( o->BoneTransform[60], p, pos, true );
			CreateSprite ( BITMAP_ENERGY, pos, 0.5f+(Luminosity*0.2f), Light, o, WorldTime*0.1f );
			CreateSprite ( BITMAP_ENERGY, pos, 0.5f+(Luminosity*0.2f), Light, o, -WorldTime*0.1f );
            CreateParticle( BITMAP_LIGHT, pos, o->Angle, Light, 0, 1.1f);

            Vector ( 0.1f, 0.4f, 1.f, Light );
			CreateSprite ( BITMAP_LIGHT, Position, 1.f, Light, o, 0.f );

            Vector ( 0.1f, 0.3f, 1.f, Light );
            for ( i=0; i<5; ++i )
            {
                b->TransformPosition ( o->BoneTransform[63+i], p, Position, true );
			    CreateSprite ( BITMAP_LIGHT, Position, 0.5f+(sinf(WorldTime*0.001f)*0.2f), Light, o, 0.f );

                if ( ( o->CurrentAction==MONSTER01_STOP1 || o->CurrentAction==MONSTER01_STOP2 ) && (rand()%50)==0 )
                {
                    Angle[0] = (float)(rand()%360);
                    Angle[2] = (float)(rand()%360);
      			    CreateJoint ( BITMAP_FLARE+1, Position, pos, Angle, 5, NULL, 20.f );
                }
            }
        }
        return true;

    case MODEL_MONSTER01+69:
        if ( o->CurrentAction!=MONSTER01_DIE )
        {
            vec3_t Pos1, Pos2;
            Luminosity = (float)sin ( WorldTime*0.003f )*0.2f+0.8f;
		    Vector ( 0, 0, 0, p );
		    b->TransformPosition ( o->BoneTransform[33], p, Pos1, true );
		    b->TransformPosition ( o->BoneTransform[34], p, Pos2, true );
            if ( o->SubType==9 )
            {
                Vector ( Luminosity*1.0f, Luminosity*0.9f, Luminosity*0.9f, Light );
            }
            else
            {
    		    Vector ( Luminosity*0.0f, Luminosity*0.9f, Luminosity*1.f, Light );
            }
		    CreateSprite(BITMAP_ENERGY,Pos1,0.1f,Light,o,0.f);
		    CreateSprite(BITMAP_ENERGY,Pos2,0.1f,Light,o,0.f);
            if ( o->SubType==9 )
            {
    		    Vector ( Luminosity*1.0f, Luminosity*0.3f, Luminosity*0.1f, Light );
            }
            else
            {
    		    Vector ( Luminosity*0.1f, Luminosity*0.3f, Luminosity*1.f, Light );
            }
		    CreateSprite(BITMAP_SHINY+1,Pos1,0.7f,Light,o,0.f);
		    CreateSprite(BITMAP_SHINY+1,Pos2,0.7f,Light,o,0.f);

            if ( o->SubType==9 )
            {
    		    Vector ( Luminosity*1.0f, Luminosity*0.9f, Luminosity*0.9f, Light );
            }
            else
            {
    		    Vector ( Luminosity*0.9f, Luminosity*0.9f, Luminosity*1.f, Light );
            }
		    for (i = 0; i < 6; ++i)
		    {
			    Vector(0.f,-40.f - i * 24.f,0.f,p);
			    b->TransformPosition ( o->BoneTransform[41], p, Pos1, true );
			    b->TransformPosition ( o->BoneTransform[51], p, Pos2, true );
                if ( o->SubType==9 )
                {
			        CreateParticle(BITMAP_FIRE+1,Pos1,o->Angle,Light,1,7.2f/(i/2+6));
			        CreateParticle(BITMAP_FIRE+1,Pos2,o->Angle,Light,1,7.2f/(i/2+6));
                }
                else
                {
			        CreateParticle(BITMAP_FIRE+3,Pos1,o->Angle,Light,12,7.2f/(i/2+6)*0.5f);
			        CreateParticle(BITMAP_FIRE+3,Pos2,o->Angle,Light,12,7.2f/(i/2+6)*0.5f);
                }
		    }
        }
        else
        {
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;
        }
		return true;
    }
    return false;
}

bool RenderHellasMonsterCloth(CHARACTER* c, OBJECT* o, bool Translate, int Select )
{
    return false;
}

bool RenderHellasMonsterObjectMesh(OBJECT* o, BMD* b )
{
    bool    success = false;
    if ( o->Type==MODEL_MONSTER01+63 )
    {
        if ( o->CurrentAction==MONSTER01_DIE )
        {
            if ( o->Alpha<0.5f )
            {
                b->RenderMeshEffect ( 0, BITMAP_BUBBLE );
            }
        }
        b->RenderBody( RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
        b->RenderMesh( 0, RENDER_METAL|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME );
        success = true;
    }
    else if ( o->Type==MODEL_MONSTER01+64 )
    {
        if ( o->CurrentAction==MONSTER01_DIE && o->AnimationFrame > 14.8f && o->LifeTime == 90)
        {
            if ( o->LifeTime!=10 )
            {
                PlayBuffer ( SOUND_KUNDUN_SHUDDER );
            }
            o->LifeTime = 10;
            b->RenderMesh(0, RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
            EarthQuake = (float)(rand()%8-8)*0.1f;
            
            vec3_t p, Position;
			Vector(39.0f,-7.5f,-0.5,p);
			b->TransformPosition(o->BoneTransform[4], p, Position, true);
			CreateEffect(MODEL_CUNDUN_PART8,Position,o->Angle,o->Light,3,o,-130,3);
			Vector(24.0f,-7.5f,32.5f,p);
			b->TransformPosition(o->BoneTransform[4], p, Position, true);
			CreateEffect(MODEL_CUNDUN_PART1,Position,o->Angle,o->Light,3,o,-130,4);
			Vector(24.0f,-8.5f,-32.5f,p);
			b->TransformPosition(o->BoneTransform[4], p, Position, true);
			CreateEffect(MODEL_CUNDUN_PART2,Position,o->Angle,o->Light,3,o,-130,5);
			Vector(-0.5f,4.0f,0.5f,p);
			b->TransformPosition(o->BoneTransform[4], p, Position, true);
			CreateEffect(MODEL_CUNDUN_PART3,Position,o->Angle,o->Light,2,o,-130,6);
			Vector(-2.5f,-22.0f,54.0f,p);
			b->TransformPosition(o->BoneTransform[4], p, Position, true);
			CreateEffect(MODEL_CUNDUN_PART4,Position,o->Angle,o->Light,3,o,-130,1);
			Vector(-4.5f,-24.5f,-53,p);
			b->TransformPosition(o->BoneTransform[4], p, Position, true);
			CreateEffect(MODEL_CUNDUN_PART5,Position,o->Angle,o->Light,3,o,-130,2);
			Vector(-136.0f,-153.5f,0,p);
			b->TransformPosition(o->BoneTransform[4], p, Position, true);
			CreateEffect(MODEL_CUNDUN_PART6,Position,o->Angle,o->Light,4,o,-10,2);
			Vector(-135.0f,-153.0f,0.0f,p);
			b->TransformPosition(o->BoneTransform[4], p, Position, true);
			CreateEffect(MODEL_CUNDUN_PART7,Position,o->Angle,o->Light,5,o,-130,2);
 
			CreateEffect ( MODEL_CUNDUN_SKILL, o->Position, o->Angle, o->Light, 2 );
		}
        else
        {
            if (o->CurrentAction == MONSTER01_DIE && o->AnimationFrame >= 8 && o->LifeTime >= 100)
            {
                if ( o->LifeTime!=90 )
                {
                    PlayBuffer ( SOUND_KUNDUN_DESTROY );
                }
                o->LifeTime = 90;

                vec3_t Angle = { 0.0f, 0.0f, 0.0f};
                int iCount = 86;
                for ( int i = 0; i < iCount; ++i)
                {
                    Angle[0] = -10.f;
                    Angle[1] = 0.f;
                    Angle[2] = i*(10.f+rand()%10);
                    
                    vec3_t Position;
                    VectorCopy( o->Position, Position);
                    Position[2] += 200.f;
					CreateJoint(BITMAP_JOINT_SPIRIT,Position,Position,Angle,3,NULL,50.f,0,0);
					CreateJoint(BITMAP_JOINT_SPIRIT2,Position,Position,Angle,3,NULL,50.f,0,0);
                }
                EarthQuake = (float)(rand()%8-8)*0.1f;
           }
            
            if (o->LifeTime >= 90)
            {
				o->Alpha = 1.0f;
				b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
				Vector(0.4f,0.4f,0.3f,b->BodyLight);
				b->RenderMesh(1, RENDER_CHROME|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
				b->RenderMesh(2, RENDER_CHROME|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
				b->RenderMesh(3, RENDER_CHROME|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
				b->RenderMesh(4, RENDER_CHROME|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
				b->RenderMesh(5, RENDER_CHROME|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
				b->RenderMesh(6, RENDER_CHROME|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);

				if ( o->CurrentAction==MONSTER01_SHOCK )
				{
					if ((o->AnimationFrame > 3.5f && o->AnimationFrame < 6.0f) ||
						(o->AnimationFrame > 8.0f && o->AnimationFrame < 11.0f))
					{
						b->RenderMesh(1, RENDER_TEXTURE|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
						b->RenderMesh(2, RENDER_TEXTURE|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
						b->RenderMesh(3, RENDER_TEXTURE|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
						b->RenderMesh(5, RENDER_TEXTURE|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
					}
				}
            }
        }
        success = true;
    }
    else if ( o->Type==MODEL_CUNDUN_PART1 || o->Type==MODEL_CUNDUN_PART2 || o->Type==MODEL_CUNDUN_PART3 ||
        o->Type==MODEL_CUNDUN_PART4 || o->Type==MODEL_CUNDUN_PART5 || o->Type==MODEL_CUNDUN_PART8 )
    {
        b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        Vector(0.4f,0.4f,0.3f,b->BodyLight);
        b->RenderBody(RENDER_CHROME|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        success = true;
    }
    else if ( o->Type==MODEL_MONSTER01+65 )
    {
        if ( o->CurrentAction==MONSTER01_DIE )
        {
            if ( o->Alpha<0.5f )
            {
                b->RenderMeshEffect ( 0, BITMAP_BUBBLE );
            }
        }
        b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        Vector(0.5f,0.1f,0.0f,b->BodyLight);
        b->RenderMesh(0, RENDER_CHROME|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        success = true;
    }
    else if ( o->Type==MODEL_MONSTER01+66 )
    {
        if ( o->CurrentAction==MONSTER01_DIE )
        {
            if ( o->Alpha<0.5f )
            {
                b->RenderMeshEffect ( 0, BITMAP_BUBBLE );
            }
        }
        b->RenderBody ( RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
        Vector ( 0.25f, 0.2f, 0.1f, b->BodyLight );
        b->RenderMesh(0, RENDER_CHROME|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        b->RenderMesh(2, RENDER_CHROME|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        success = true;
    }
    else if ( o->Type==MODEL_MONSTER01+67 )
    {
        if ( o->CurrentAction==MONSTER01_DIE )
        {
            if ( o->Alpha<0.5f )
            {
                b->RenderMeshEffect ( 0, BITMAP_BUBBLE );
            }
        }
        if ( o->SubType==9 )
        {
            Vector(0.3f,0.1f,0.1f,b->BodyLight);
        }
        else
        {
        }
        b->RenderMesh(0, RENDER_TEXTURE|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        b->RenderMesh(2, RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        b->RenderMesh(3, RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        if ( o->SubType==9 )
        {
            Vector(1.f,0.6f,0.3f,b->BodyLight);
            b->RenderMesh(1, RENDER_CHROME|RENDER_BRIGHT,0.5f,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
            Vector(1.f,1.f,1.f,b->BodyLight);
            b->RenderMesh(1, RENDER_CHROME2|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
            Vector(1.f,0.6f,0.1f,b->BodyLight);
        }
        else
        {
            Vector(0.1f,0.6f,1.f,b->BodyLight);
        }
        b->RenderMesh(0, RENDER_CHROME|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        b->RenderMesh(2, RENDER_CHROME|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);

        if ( o->SubType==9 )
        {
            RenderDestroy_Def ( o, b );
        }
        success = true;
    }
    else if ( o->Type==MODEL_MONSTER01+68 )
    {
        if ( o->CurrentAction==MONSTER01_DIE )
        {
            if ( o->Alpha<0.5f )
            {
                b->RenderMeshEffect ( 0, BITMAP_BUBBLE );
            }
        }
        b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);

        float Luminosity = sinf(WorldTime*0.002f)*0.3f+0.7f;
		Vector ( 0.5f, Luminosity, 0.5f, b->BodyLight );
        b->RenderMesh(0, RENDER_CHROME|RENDER_BRIGHT, 0.8f,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        success = true;
    }
    else if ( o->Type==MODEL_MONSTER01+69 )
    {
        if ( o->CurrentAction==MONSTER01_DIE )
        {
            if ( o->Alpha<0.5f )
            {
                b->RenderMeshEffect ( 0, BITMAP_BUBBLE );
            }
        }
    	b->BeginRender(1.f);
        b->RenderMesh(0, RENDER_TEXTURE,o->Alpha,-1,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE,o->Alpha,-1,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        b->RenderMesh(3, RENDER_TEXTURE,o->Alpha,-1,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        b->RenderMesh(5, RENDER_TEXTURE,o->Alpha,-1,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);

        float Luminosity = sinf(WorldTime*0.002f)*0.3f+0.7f;
        if ( o->SubType==9 )
        {
            o->BlendMeshLight = 0.5f;

		    Vector ( Luminosity, Luminosity*0.4f, Luminosity*0.4f, b->BodyLight );
        }
        else
        {
            o->BlendMeshLight = 1.f;
		    Vector ( Luminosity*0.3f, Luminosity*0.6f, Luminosity*1.f, b->BodyLight );
        }
        b->RenderMesh(2, RENDER_WAVE|RENDER_BRIGHT,o->Alpha,2,o->BlendMeshLight ,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        b->RenderMesh(4, RENDER_WAVE|RENDER_BRIGHT,o->Alpha,4,o->BlendMeshLight ,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        
        if ( o->SubType==9 )
        {
            Vector ( 1.f, 1.f, 1.f, b->BodyLight);
            b->RenderMesh(0, RENDER_METAL|RENDER_BRIGHT,o->Alpha,-1,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
            b->RenderMesh(1, RENDER_METAL|RENDER_BRIGHT,o->Alpha,-1,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
            b->RenderMesh(3, RENDER_METAL|RENDER_BRIGHT,o->Alpha,-1,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        }
        else
        {
            float Light = sinf(WorldTime*0.002f)*0.3f+0.7f;
            b->RenderMesh(2, RENDER_TEXTURE,o->Alpha,2,Light,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
            b->RenderMesh(4, RENDER_TEXTURE,o->Alpha,4,Light,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        }

        o->BlendMeshLight = sinf(WorldTime*0.002f)*0.1f+0.05f;
        b->RenderMesh(6, RENDER_TEXTURE,o->Alpha,6,o->BlendMeshLight ,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
    	b->EndRender();

        RenderDestroy_Def ( o, b );
        success = true;
    }
    if ( success )
    {
		if( g_isCharacterBuff(o, eBuff_PhysDefense) || g_isCharacterBuff(o, eBuff_Defense) )
        {
            float Luminosity = sinf( WorldTime*0.001f )*0.2f+0.5f;

            if ( g_isCharacterBuff(o, eBuff_PhysDefense) )
            {
                Vector( Luminosity*0.1f, Luminosity*0.3f, Luminosity*0.6f, b->BodyLight );
            }
            else if ( g_isCharacterBuff(o, eBuff_Defense) )
            {
                Vector( Luminosity*0.1f, Luminosity*0.6f, Luminosity*0.3f, b->BodyLight );
            }

        	RenderPartObjectEdge ( b, o, RENDER_CHROME|RENDER_BRIGHT, true, 1.3f );
        }
    }

    return success;
}
