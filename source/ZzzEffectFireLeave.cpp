///////////////////////////////////////////////////////////////////////////////
// 3D 특수효과 관련 함수
//
// *** 함수 레벨: 3
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzTexture.h"
#include "ZzzAi.h"
#include "ZzzEffect.h"
#include "DSPlaySound.h"
#include "WSClient.h"
#include "CSChaosCastle.h"
#include "GMBattleCastle.h"
#include "GMHuntingGround.h"
#include "GMAida.h"
#include "GMCrywolf1st.h"
#include "GM3rdChangeUp.h"
// 맵 관련 include
#ifdef PSW_ADD_MAPSYSTEM
#include "w_MapHeaders.h"
#endif // PSW_ADD_MAPSYSTEM

//----------------------------------------------------------------------------------------------------------------------------


int RainTarget = 0;
int RainCurrent = 0;

// 악마의 광장 비
static  int RainSpeed = 30;
static  int RainAngle = 0;
static  int RainPosition = 0;


///////////////////////////////////////////////////////////////////////////////
// 불 효과 생성(배경의 횟불 같은데 쓰임)
///////////////////////////////////////////////////////////////////////////////

void CreateBonfire(vec3_t Position,vec3_t Angle)
{
	Position[0] += (float)(rand()%16-8);
	Position[1] += (float)(rand()%16-8);
	Position[2] += (float)(rand()%16-8);
	vec3_t Light;
	Vector(1.f,1.f,1.f,Light);
	CreateParticle(BITMAP_FIRE,Position,Angle,Light,rand()%4);
	if(rand()%4==0)
	{
		CreateParticle(BITMAP_SPARK,Position,Angle,Light);
		vec3_t a;
		Vector(-(float)(rand()%60+30),0.f,(float)(rand()%360),a);
		CreateJoint(BITMAP_JOINT_SPARK,Position,Position,a);
	}
	float Luminosity = (float)(rand()%6+6)*0.1f;
	Vector(Luminosity,Luminosity*0.6f,Luminosity*0.4f,Light);
	AddTerrainLight(Position[0],Position[1],Light,4,PrimaryTerrainLight);
}

void CreateFire(int Type,OBJECT *o,float x,float y,float z)
{
	vec3_t Light;
	vec3_t p,Position;
	float Luminosity;
	float Matrix[3][4];
	AngleMatrix(o->Angle,Matrix);
	Vector(x,y,z,p);
	VectorRotate(p,Matrix,Position);
	VectorAdd(Position,o->Position,Position)
	Position[0] += (float)(rand()%16-8);
	Position[1] += (float)(rand()%16-8);
	Position[2] += (float)(rand()%16-8);
	switch(Type)
	{
	case 0:
		Luminosity = (float)(rand()%6+6)*0.1f;
		Vector(Luminosity,Luminosity*0.6f,Luminosity*0.4f,Light);
     	if(rand()%2==0)
     		CreateParticle(BITMAP_FIRE,Position,o->Angle,Light,rand()%4);
		AddTerrainLight(Position[0],Position[1],Light,4,PrimaryTerrainLight);
		break;
	case 1:
     	if(rand()%2==0)
	     	CreateParticle(BITMAP_SMOKE,Position,o->Angle,o->Light);
		break;
	case 2:
     	if(rand()%2==0)
        	CreateParticle(BITMAP_SMOKE,Position,o->Angle,o->Light,2);
		break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// 주인공이 지나갈때 해골 구르는 효과 생성(로스트 타워에서 쓰임)
///////////////////////////////////////////////////////////////////////////////

void CheckSkull(OBJECT *o)
{
	vec3_t Position;
	VectorCopy(Hero->Object.Position,Position);
	if(Hero->Object.CurrentAction>=PLAYER_WALK_MALE && Hero->Object.CurrentAction<=PLAYER_RUN_RIDE_WEAPON
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
		|| (Hero->Object.CurrentAction==PLAYER_RAGE_UNI_RUN || Hero->Object.CurrentAction==PLAYER_RAGE_UNI_RUN_ONE_RIGHT)
#endif //PBG_ADD_NEWCHAR_MONK_ANI
		)
	{
		if(o->Direction[0] < 0.1f)
		{
			float dx = Position[0]-o->Position[0];
			float dy = Position[1]-o->Position[1];
			float Distance = sqrtf(dx*dx+dy*dy);
			if(Distance < 50.f)
			{
				Vector(-dx*0.4f,-dy*0.4f,0.f,o->Direction);
				o->HeadAngle[1] = -dx*4.f;
				o->HeadAngle[0] = -dy*4.f;
     			PlayBuffer(SOUND_BONE2,o);
			}
		}
	}
	VectorScale(o->Direction,0.6f,o->Direction);
	VectorScale(o->HeadAngle,0.6f,o->HeadAngle);
	VectorAdd(o->Position,o->Direction,o->Position);
	VectorAdd(o->Angle,o->HeadAngle,o->Angle);
}

///////////////////////////////////////////////////////////////////////////////
// 월드에 날아다니는 낙엽, 먼지 효과 처리
///////////////////////////////////////////////////////////////////////////////
// 악마의 광장 비
//////////////////////////////////////////////////////////////////////////
bool CreateDevilSquareRain ( PARTICLE* o, int Index )
{
	if ( InDevilSquare() == false 
		&& World != WD_34CRYWOLF_1ST
		) 
	{
		return false;
	}

	if(World == WD_34CRYWOLF_1ST && weather != 1)
		return false;

    o->Type = BITMAP_RAIN;
	if ( Index<300 )
	{
		Vector(
			Hero->Object.Position[0]+(float)(rand()%1600-800),
			Hero->Object.Position[1]+(float)(rand()%1400-500),
			Hero->Object.Position[2]+(float)(rand()%200+300),
			o->Position);
	}
	else
	{
		Vector(
			Hero->Object.Position[0]+(float)(rand()%1600-800),
			Hero->Object.Position[1]+(float)(rand()%300+1000)-RainPosition,
			Hero->Object.Position[2]+(float)(rand()%200+300),
			o->Position);
	}
	if(rand()%2==0)
	{
		Vector((float)(-(rand()%20+20)),0.f,0.f,o->Angle);
	}
	else
	{
		Vector((float)(-(rand()%20+30+RainAngle)),0.f,0.f,o->Angle);
	}
	vec3_t Velocity;
	Vector(0.f,0.f,-(float)(rand()%40+RainSpeed),Velocity);
	float Matrix[3][4];
	AngleMatrix(o->Angle,Matrix);
	VectorRotate(Velocity,Matrix,o->Velocity);

    return true;
}


//  카오스 캐슬 비
//////////////////////////////////////////////////////////////////////////
bool CreateChaosCastleRain ( PARTICLE* o, int Index )
{
    if ( InChaosCastle()==false ) return false;

    o->Type = BITMAP_RAIN;
    o->TurningForce[0] = 1.f;
    o->TurningForce[1] = 30.f+rand()%10;
	if ( Index<300 )
	{
		Vector(
			Hero->Object.Position[0]+(float)(rand()%1600-800),
			Hero->Object.Position[1]+(float)(rand()%1400-500),
			Hero->Object.Position[2]+(float)(rand()%200+300),
			o->Position);
	}
	else
	{
		Vector(
			Hero->Object.Position[0]+(float)(rand()%1600-800),
			Hero->Object.Position[1]+(float)(rand()%300+1000)-RainPosition,
			Hero->Object.Position[2]+(float)(rand()%200+300),
			o->Position);
	}
	if(rand()%2==0)
	{
		Vector((float)(-(rand()%20+20)),0.f,0.f,o->Angle);
	}
	else
	{
		Vector((float)(-(rand()%20+30+RainAngle)),0.f,0.f,o->Angle);
	}
	vec3_t Velocity;
	Vector(0.f,0.f,-(float)(rand()%40+RainSpeed+20),Velocity);
	float Matrix[3][4];
	AngleMatrix(o->Angle,Matrix);
	VectorRotate(Velocity,Matrix,o->Velocity);

    return true;
}


//  로랜시아 날리는 낙엽잎.
//////////////////////////////////////////////////////////////////////////
bool CreateLorenciaLeaf ( PARTICLE* o )
{
    if ( World!=WD_0LORENCIA ) return false;

	o->Type = BITMAP_LEAF1;
	vec3_t Position;
	Vector(Hero->Object.Position[0]+(float)(rand()%1600-800),Hero->Object.Position[1]+(float)(rand()%1400-500),
		Hero->Object.Position[2]+(float)(rand()%300+50),
		Position);
	VectorCopy(Position,o->Position);
	VectorCopy(Position,o->StartPosition);
	o->Velocity[0] = -(float)(rand()%64+64)*0.1f;
	if(Position[1] < CameraPosition[1]+400.f) o->Velocity[0] = -o->Velocity[0]+3.2f;
	o->Velocity[1] = (float)(rand()%32-16)*0.1f;
	o->Velocity[2] = (float)(rand()%32-16)*0.1f;
	o->TurningForce[0] = (float)(rand()%16-8)*0.1f;
	o->TurningForce[1] = (float)(rand()%64-32)*0.1f;
	o->TurningForce[2] = (float)(rand()%16-8)*0.1f;

    return true;
}


//  천공에 날리는 빛.
//////////////////////////////////////////////////////////////////////////
bool CreateHeavenRain ( PARTICLE* o, int index )
{
    if ( World!=WD_10HEAVEN ) return false;

    int Rainly = RainCurrent*MAX_LEAVES/100;
	if ( index<Rainly )
	{
		o->Type = BITMAP_RAIN;
		Vector(Hero->Object.Position[0]+(float)(rand()%1600-800),Hero->Object.Position[1]+(float)(rand()%1400-500),
			Hero->Object.Position[2]+(float)(rand()%200+200),
			o->Position);
		Vector(-30.f,0.f,0.f,o->Angle);
		vec3_t Velocity;
		Vector(0.f,0.f,-(float)(rand()%24+20),Velocity);
		float Matrix[3][4];
		AngleMatrix(o->Angle,Matrix);
		VectorRotate(Velocity,Matrix,o->Velocity);
	}
    return true;
}


//  데비아스의 눈.
//////////////////////////////////////////////////////////////////////////
bool CreateDeviasSnow ( PARTICLE* o )
{
    if ( World!=WD_2DEVIAS ) return false;

    o->Type = BITMAP_LEAF1;
    o->Scale = 5.f;
#ifdef DEVIAS_XMAS_EVENT
	if(rand()%5 == 0)
#else // DEVIAS_XMAS_EVENT
	if(rand()%10 == 0)
#endif // DEVIAS_XMAS_EVENT
	{
		o->Type = BITMAP_LEAF2;
		o->Scale = 10.f;
	}
	Vector(Hero->Object.Position[0]+(float)(rand()%1600-800),Hero->Object.Position[1]+(float)(rand()%1400-500),
		Hero->Object.Position[2]+(float)(rand()%200+200),
		o->Position);
	Vector(-30.f,0.f,0.f,o->Angle);
	vec3_t Velocity;
	Vector(0.f,0.f,-(float)(rand()%16+8),Velocity);
	float Matrix[3][4];
	AngleMatrix(o->Angle,Matrix);
	VectorRotate(Velocity,Matrix,o->Velocity);

    return true;
}

//  아틀란스 날리는 효과.
//////////////////////////////////////////////////////////////////////////
bool CreateAtlanseLeaf ( PARTICLE*o )
{
    if ( World!=WD_3NORIA && World!=WD_7ATLANSE ) return false;

    o->Type = BITMAP_LEAF1;
	vec3_t Position;
	Vector(Hero->Object.Position[0]+(float)(rand()%1600-800),Hero->Object.Position[1]+(float)(rand()%1400-500),
		Hero->Object.Position[2]+(float)(rand()%300+50),
		Position);
	VectorCopy(Position,o->Position);
	VectorCopy(Position,o->StartPosition);
	o->Velocity[0] = -(float)(rand()%64+64)*0.1f;
	if(Position[1] < CameraPosition[1]+400.f) o->Velocity[0] = -o->Velocity[0]+3.2f;
	o->Velocity[1] = (float)(rand()%32-16)*0.1f;
	o->Velocity[2] = (float)(rand()%32-16)*0.1f;
	o->TurningForce[0] = (float)(rand()%16-8)*0.1f;
	o->TurningForce[1] = (float)(rand()%64-32)*0.1f;
	o->TurningForce[2] = (float)(rand()%16-8)*0.1f;

    return true;
}


//  악마의 광장 비
//////////////////////////////////////////////////////////////////////////
bool MoveDevilSquareRain ( PARTICLE* o )
{
    if ( InDevilSquare() == false 
		&& World != WD_34CRYWOLF_1ST
		) return false;

//	if(o->Type != BITMAP_RAIN)
	if(World == WD_34CRYWOLF_1ST && weather != 1)
		return false;


	VectorAdd(o->Position,o->Velocity,o->Position);
	float Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
	if(o->Position[2] < Height)
	{
		o->Live = false;
		o->Position[2] = Height+10.f;
		if(rand()%4==0)
			CreateParticle(BITMAP_RAIN_CIRCLE,o->Position,o->Angle,o->Light);
		else
			CreateParticle(BITMAP_RAIN_CIRCLE+1,o->Position,o->Angle,o->Light);
	}
    return true;
}


//  카오스 캐슬 비.
//////////////////////////////////////////////////////////////////////////
bool MoveChaosCastleRain ( PARTICLE* o )
{
    if ( InChaosCastle()==false ) return false;

	VectorAdd(o->Position,o->Velocity,o->Position);
	float Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
	if ( o->Position[2]<Height && (TERRAIN_ATTRIBUTE( o->Position[0],o->Position[1])&TW_NOGROUND)!=TW_NOGROUND )
	{
		o->Live = false;
		o->Position[2] = Height+10.f;
		if(rand()%4==0)
			CreateParticle(BITMAP_RAIN_CIRCLE,o->Position,o->Angle,o->Light);
		else
			CreateParticle(BITMAP_RAIN_CIRCLE+1,o->Position,o->Angle,o->Light);
	}
    return true;
}


//  천공, 로랜시아
//////////////////////////////////////////////////////////////////////////
bool MoveHeavenRain ( PARTICLE* o )
{
    if ( World!=WD_0LORENCIA && World!=WD_10HEAVEN ) return false;

    if(o->Type==BITMAP_RAIN)
	{
		VectorAdd(o->Position,o->Velocity,o->Position);
		float Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
		if(o->Position[2] < Height)
		{
			o->Live = false;
			o->Position[2] = Height+10.f;
            if( World!=WD_10HEAVEN )
				CreateParticle(BITMAP_RAIN_CIRCLE,o->Position,o->Angle,o->Light);
		}
	}
	else
	{
		o->Velocity[0] += (float)(rand()%16-8)*0.1f;
		o->Velocity[1] += (float)(rand()%16-8)*0.1f;
		o->Velocity[2] += (float)(rand()%16-8)*0.1f;
		VectorAdd(o->Position,o->Velocity,o->Position);
		
		o->TurningForce[0] += (float)(rand()%8-4)*0.02f;
		o->TurningForce[1] += (float)(rand()%16-8)*0.02f;
		o->TurningForce[2] += (float)(rand()%8-4)*0.02f;
		VectorAdd(o->Angle,o->TurningForce,o->Angle);
		
		vec3_t Range;
		VectorSubtract(o->StartPosition,o->Position,Range);
		float Length = Range[0]*Range[0]+Range[1]*Range[1]+Range[2]*Range[2];
		if(Length >= 200000.f) o->Live = false;
	}

    return true;
}


//  기타 효과.
//////////////////////////////////////////////////////////////////////////
void MoveEtcLeaf ( PARTICLE* o )
{
	float Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
	if(o->Position[2] <= Height)
	{
		o->Position[2] = Height;
		o->Light[0] -= 0.05f;
		o->Light[1] -= 0.05f;
		o->Light[2] -= 0.05f;
		if(o->Light[0] <= 0.f)
     		o->Live = false;
	}
	else
	{
		o->Velocity[0] += (float)(rand()%16-8)*0.1f;
		o->Velocity[1] += (float)(rand()%16-8)*0.1f;
		o->Velocity[2] += (float)(rand()%16-8)*0.1f;
		VectorAdd(o->Position,o->Velocity,o->Position);
	}
}


//////////////////////////////////////////////////////////////////////////
//  윌드에 날아다니는 낙엽, 먼지, 비, 누등의 효과 처리.
//////////////////////////////////////////////////////////////////////////
bool MoveLeaves()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_MOVE_LEAVES, PROFILING_MOVE_LEAVES );
#endif // DO_PROFILING
	int iMaxLeaves = ( InDevilSquare() == true ) ? MAX_LEAVES : 80;

    if( World==WD_10HEAVEN )
    {
        RainTarget = MAX_LEAVES/2;
    }
    else if ( InChaosCastle()==true )
    {
        RainTarget = MAX_LEAVES/2;
        iMaxLeaves = 80;
    }
    else if ( battleCastle::InBattleCastle() )
    {
        iMaxLeaves = 40;
    }
    else if ( World == WD_55LOGINSCENE )
    {
        iMaxLeaves = 80;
    }
	else if( M34CryWolf1st::IsCyrWolf1st())
	{
		if(weather == 1)
			iMaxLeaves = 60;
		else 
		if(weather == 2)
			iMaxLeaves = 50;
	}
	if(RainCurrent > RainTarget)
		RainCurrent--;
	else if(RainCurrent < RainTarget)
		RainCurrent++;

	// 악마의 광장 비
	RainSpeed = (int)sinf(WorldTime*0.001f)*10+30;
    RainAngle = (int)sinf(WorldTime*0.0005f+50.f)*20;
	RainPosition += 20;
	RainPosition %= 2000;

#ifdef DEVIAS_XMAS_EVENT
	if(World == WD_2DEVIAS)
	{
		iMaxLeaves = MAX_LEAVES_DOUBLE;
	}
#endif // DEVIAS_XMAS_EVENT
	
	for(int i=0;i<iMaxLeaves;i++)
	{
		PARTICLE *o = &Leaves[i];
		if(!o->Live)
		{
			Vector ( 1.f, 1.f, 1.f, o->Light );
			o->Live = true;

            if ( CreateDevilSquareRain( o, i ) )continue;
            if ( CreateChaosCastleRain( o, i ) )continue;
            if ( CreateLorenciaLeaf( o ) )      continue;
            if ( CreateHeavenRain( o, i ) )     continue;
			if ( CreateDeviasSnow( o ) )        continue;
            if ( CreateAtlanseLeaf( o ) )       continue;
            if ( battleCastle::CreateFireSnuff( o ) ) continue;
			if ( M31HuntingGround::CreateMist( o ) ) continue;
			if ( M33Aida::CreateMist( o ) ) continue;
			if(weather == 2)
				if(M34CryWolf1st::CreateMist(o)) continue;
			if ( SEASON3A::CGM3rdChangeUp::Instance().CreateFireSnuff( o ) ) continue;
#ifdef CSK_ADD_MAP_ICECITY			
			if(g_Raklion.CreateSnow( o )) continue;
#endif //CSK_ADD_MAP_ICECITY
#ifdef YDG_ADD_MAP_SANTA_TOWN
			if(g_SantaTown.CreateSnow( o )) continue;
#endif	// YDG_ADD_MAP_SANTA_TOWN
#ifdef PBG_ADD_PKFIELD
			if(g_PKField.CreateFireSpark(o)) continue;
#endif //PBG_ADD_PKFIELD
#ifdef YDG_ADD_MAP_DOPPELGANGER2
			if(g_DoppelGanger2.CreateFireSpark(o)) continue;
#endif	// YDG_ADD_MAP_DOPPELGANGER2
#ifdef LDS_ADD_EMPIRE_GUARDIAN
			if(g_EmpireGuardian1.CreateRain(o)) continue;
			if(g_EmpireGuardian2.CreateRain(o)) continue;
			if(g_EmpireGuardian3.CreateRain(o)) continue;
#endif //LDS_ADD_EMPIRE_GUARDIAN
#ifdef LDS_ADD_MAP_UNITEDMARKETPLACE
			if(g_UnitedMarketPlace.CreateRain(o)) continue;
#endif	// LDS_ADD_MAP_UNITEDMARKETPLACE
		}
		else
		{
            if ( MoveDevilSquareRain( o ) ) continue;
            if ( MoveChaosCastleRain( o ) ) continue;
            if ( MoveHeavenRain( o ) )      continue;
#ifdef LDS_ADD_MAP_UNITEDMARKETPLACE
			if(g_UnitedMarketPlace.MoveRain(o)) continue;
#endif	// LDS_ADD_MAP_UNITEDMARKETPLACE
            MoveEtcLeaf ( o );
		}
	}
#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_MOVE_LEAVES );
#endif // DO_PROFILING	
	return true;
}


//////////////////////////////////////////////////////////////////////////
//  
//////////////////////////////////////////////////////////////////////////
void RenderLeaves()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_LEAVES, PROFILING_RENDER_LEAVES );
#endif // DO_PROFILING

    if(World==WD_2DEVIAS || World==WD_7ATLANSE || World==WD_10HEAVEN
#ifdef CSK_ADD_MAP_ICECITY
		|| IsIceCity()
#endif // CSK_ADD_MAP_ICECITY
#ifdef YDG_ADD_MAP_SANTA_TOWN
		|| IsSantaTown()
#endif	// YDG_ADD_MAP_SANTA_TOWN
#ifdef PBG_ADD_PKFIELD
		|| IsPKField()
#endif //PBG_ADD_PKFIELD
#ifdef YDG_ADD_MAP_DOPPELGANGER2
		|| IsDoppelGanger2()	// 불카누스 베이스
#endif	// YDG_ADD_MAP_DOPPELGANGER2
#ifdef LDS_ADD_EMPIRE_GUARDIAN
		|| IsEmpireGuardian1()
		|| IsEmpireGuardian2()
		|| IsEmpireGuardian3()
#endif //LDS_ADD_EMPIRE_GUARDIAN
#ifdef LDS_ADD_MAP_UNITEDMARKETPLACE
		|| IsUnitedMarketPlace()
#endif	// LDS_ADD_MAP_UNITEDMARKETPLACE
		)
        EnableAlphaBlend();
    else if ( InChaosCastle()==true )
        EnableAlphaTest();
    else if ( battleCastle::InBattleCastle()==true )
        EnableAlphaBlend();
    else if ( World == WD_55LOGINSCENE)
        EnableAlphaBlend();
	else if ( World == WD_42CHANGEUP3RD_2ND )
        EnableAlphaBlend();
	else
	{
        EnableAlphaTest();
	}

	glColor3f(1.f,1.f,1.f);
#ifdef DEVIAS_XMAS_EVENT
	int iMaxLeaves;
	if(World == WD_2DEVIAS)
		iMaxLeaves = MAX_LEAVES_DOUBLE;
	else
		iMaxLeaves = MAX_LEAVES;

	for(int i=0; i<iMaxLeaves; i++)
#else // DEVIAS_XMAS_EVENT
	for(int i=0;i<MAX_LEAVES;i++)
#endif // DEVIAS_XMAS_EVENT
	{
		PARTICLE *o = &Leaves[i];
		if(o->Live 
			&& Bitmaps.FindTexture(o->Type)
			)
		{
            BindTexture(o->Type);
			if(World == WD_2DEVIAS 
#ifdef CSK_ADD_MAP_ICECITY	
				|| IsIceCity()
#endif // CSK_ADD_MAP_ICECITY
#ifdef YDG_ADD_MAP_SANTA_TOWN
				|| IsSantaTown()
#endif	// YDG_ADD_MAP_SANTA_TOWN
				)
			{
				RenderSprite(o->Type,o->Position,o->Scale,o->Scale,o->Light);
			}
			else
			{
				glPushMatrix();
				glTranslatef(o->Position[0],o->Position[1],o->Position[2]);
				float Matrix[3][4];
				AngleMatrix(o->Angle,Matrix);
                
                if ( InChaosCastle()==true )
                    RenderPlane3D ( o->TurningForce[0], o->TurningForce[1], Matrix );
                else
                {
				    if ( o->Type==BITMAP_RAIN )
					{
						if(World == WD_34CRYWOLF_1ST)
						{
							if(weather == 1)
								RenderPlane3D ( 1.f, 20.f, Matrix );
						}
						else
							RenderPlane3D ( 1.f, 20.f, Matrix );
					}
                    else if ( o->Type==BITMAP_FIRE_SNUFF )
					    RenderPlane3D ( o->Scale*2.f, o->Scale*4.f, Matrix );
				    else
					{
					    RenderPlane3D ( 3.f, 3.f, Matrix );
					}
                }
				glPopMatrix();
			}
		}
	}
#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_LEAVES );
#endif // DO_PROFILING
}

