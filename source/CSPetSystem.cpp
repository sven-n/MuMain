//////////////////////////////////////////////////////////////////////////
//  
//  이  름 : CSPetSystem.
//
//  내  용 : 캐릭터가 소환, 한정적 상용이 아닌 팻들의 시스템.
//           다크로드 ( 다크 스피릿 ),
//
//  날  짜 : 2004.03.29.
//
//  작성자 : 조 규 하.
//  
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef PET_SYSTEM

//////////////////////////////////////////////////////////////////////////
//  INCLUDE.
//////////////////////////////////////////////////////////////////////////
#include "CSPetSystem.h"
#include "zzzlodterrain.h"
#include "zzzAi.h"
#include "ZzzOpenglUtil.h"
#include "zzzInfomation.h"
#include "zzzBmd.h"
#include "zzzObject.h"
#include "zzztexture.h"
#include "zzzCharacter.h"
#include "zzzscene.h"
#include "zzzInterface.h"
#include "zzzinventory.h"
#include "zzzeffect.h"
#include "dsplaysound.h"
#include "wsclientinline.h"
#include "UIManager.h"
#ifdef YDG_ADD_NEW_DUEL_SYSTEM
#include "DuelMgr.h"
#endif	// YDG_ADD_NEW_DUEL_SYSTEM

#ifdef PJH_FIX_SPRIT
#include "CSItemOption.h"
#include "GIPetManager.h"
#endif //PJH_FIX_SPRIT

#ifndef YDG_ADD_NEW_DUEL_SYSTEM		// 정리할때 삭제해야 함
extern bool g_PetEnableDuel;	// LHJ - 결투중에 다크로드의 다크스피릿이 공격중 인지 판단하는 변수
#endif	// YDG_ADD_NEW_DUEL_SYSTEM	// 정리할때 삭제해야 함

//////////////////////////////////////////////////////////////////////////
//  EXTERN.
//////////////////////////////////////////////////////////////////////////
extern  int     MoveSceneFrame;
extern  float   WorldTime;
extern	char    TextList[30][100];
extern	int     TextListColor[30];
extern	int     TextBold[30];
extern  float   g_fScreenRate_x;	// ※
extern  float   g_fScreenRate_y;

//////////////////////////////////////////////////////////////////////////
//  
//////////////////////////////////////////////////////////////////////////
#ifdef YDG_FIX_MEMORY_LEAK_0905
CSPetSystem::~CSPetSystem()
{
	SAFE_DELETE_ARRAY(m_PetCharacter.Object.BoneTransform);
}
#endif	// YDG_FIX_MEMORY_LEAK_0905

void CSPetSystem::CreatePetPointer ( int Type, unsigned char PositionX, unsigned char PositionY, float Rotation )
{
    CHARACTER*  c = &m_PetCharacter;
	OBJECT*     o = &c->Object;

    m_PetTarget = NULL;             //  팻 공격 목표.
    m_byCommand = PET_CMD_DEFAULT;  //  명령상태.
#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
	m_pPetInfo	= NULL;				// 펫정보
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM

	o->Initialize();
	
#if defined MR0 || defined OBJECT_ID
	if(o->m_iID == 0) o->m_iID = GenID();
#endif //MR0

	c->PositionX      = PositionX;
	c->PositionY      = PositionY;
	c->TargetX      = PositionX;
	c->TargetY      = PositionY;

	c->byExtensionSkill = 0;
    c->m_pPet = NULL;

	int Index = TERRAIN_INDEX_REPEAT ( c->PositionX, c->PositionY );
	if ( ( TerrainWall[Index]&TW_SAFEZONE)==TW_SAFEZONE )
		c->SafeZone = true;
	else
		c->SafeZone = false;
	
	c->Path.PathNum     = 0;
	c->Path.CurrentPath = 0;
	c->Movement         = false;
	o->Live             = true;
	o->Visible          = false;
	o->AlphaEnable      = true;
	o->LightEnable      = true;
	o->ContrastEnable   = false;
	o->EnableBoneMatrix = true;
	o->EnableShadow     = false;
	c->Dead             = false;
	c->Blood            = false;
	c->GuildTeam        = 0;
	c->Run              = 0;
	c->GuildMarkIndex	= -1;
	c->PK               = PVP_NEUTRAL;
	o->Type             = Type;
	o->Scale            = 0.7f;
	o->Timer            = 0.f;
	o->Alpha            = 1.f;
	o->AlphaTarget      = 1.f;
	o->Velocity         = 0.f;
	o->ShadowScale      = 0.f;
	o->m_byHurtByOneToOne = 0;
    o->AI               = 0;
	o->Velocity         = 1.f;
    o->Gravity          = 13;
    c->ExtendState = 0;
    c->ExtendStateTime = 0;

	c->GuildStatus		= -1;	// 길드내에서의 직책
	c->GuildType		= 0;	// 길드타입
	c->ProtectGuildMarkWorldTime = 0.0f;

	c->m_byDieType		= 0;
	o->m_bActionStart	= false;
	o->m_bySkillCount	= 0;

    c->Class            = 0;
    o->PriorAction      = 0;
    o->CurrentAction    = 0;
    o->AnimationFrame   = 0.f;
	o->PriorAnimationFrame   = 0;
	c->JumpTime         = 0;
	o->HiddenMesh       = -1;
	c->MoveSpeed        = 10;

	g_CharacterClearBuff(o);
	
	o->Teleport         = TELEPORT_NONE;
	o->Kind             = KIND_PET;
	c->Change           = false;
	o->SubType          = 0;
	c->MonsterIndex     = -1;
    o->BlendMeshTexCoordU = 0.f;
    o->BlendMeshTexCoordV = 0.f;
	o->Position[0] = (float)( PositionX*TERRAIN_SCALE ) + 0.5f*TERRAIN_SCALE;
	o->Position[1] = (float)( PositionY*TERRAIN_SCALE ) + 0.5f*TERRAIN_SCALE;

	o->InitialSceneFrame = MoveSceneFrame;
    o->Position[2] = RequestTerrainHeight ( o->Position[0], o->Position[1] );

	Vector ( 0.f, 0.f, Rotation, o->Angle );
	Vector ( 0.5f, 0.5f, 0.5f, o->Light );
	Vector ( -60.f, -60.f, 0.f, o->BoundingBoxMin );
	switch ( Type )
	{
    case MODEL_DARK_SPIRIT:
      	Vector( 50.f, 50.f,150.f,o->BoundingBoxMax);
        break;
    default:
      	Vector( 50.f, 50.f,150.f,o->BoundingBoxMax);
		break;
	}

	if ( o->BoneTransform!=NULL )
	{
		delete[] o->BoneTransform;
		o->BoneTransform = NULL;
	}
	o->BoneTransform = new vec34_t [Models[Type].NumBones];
	
	int i;
	for ( i=0; i<2; i++ )
	{
		c->Weapon[i].Type = -1;
		c->Weapon[i].Level = 0;
        c->Weapon[i].Option1 = 0;
	}
	for ( i=0; i<MAX_BODYPART; i++ )
	{
		c->BodyPart[i].Type     = -1;
		c->BodyPart[i].Level    = 0;
        c->BodyPart[i].Option1  = 0;
        c->BodyPart[i].ExtOption = 0;
	}

    c->Wing.Type        = -1;
	c->Helper.Type      = -1;
	c->Flag.Type        = -1;
	
	c->LongRangeAttack  = -1;
	c->CollisionTime    = 0;
	o->CollisionRange   = 200.f;
	c->Rot              = 0.f;
	c->Level            = 0;//m_PetInfo.m_wLevel;
	c->Item             = -1;
	
	o->BlendMesh        = -1;
	o->BlendMeshLight   = 1.f;

    switch ( Type )
	{
    case MODEL_DARK_SPIRIT:
		c->Weapon[0].LinkBone = 0; 
		c->Weapon[1].LinkBone = 0;
        break;
	default:
		c->Weapon[0].LinkBone = 0; 
		c->Weapon[1].LinkBone = 0;
		break;
	}

    //  명령어 초기화.
    m_byCommand = PET_CMD_DEFAULT;

#ifndef KJH_FIX_DARKLOAD_PET_SYSTEM							//## 소스정리 대상임.
    //  다크스피릿 정보를 얻는다.
    SendRequestPetInfo ( 0, 0, EQUIPMENT_WEAPON_LEFT );
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM						//## 소스정리 대상임.
}


//////////////////////////////////////////////////////////////////////////
//  팻을 애니메이션 시킨다.
//////////////////////////////////////////////////////////////////////////
bool    CSPetSystem::PlayAnimation ( OBJECT* o )
{
    BMD*    b = &Models[o->Type];
    float   playSpeed = 0.1f;

    switch ( m_PetType )
    {
    case PET_TYPE_DARK_SPIRIT:
        playSpeed = 0.4f;
        break;
    }

    b->CurrentAction = o->CurrentAction;
    return b->PlayAnimation ( &o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, playSpeed, o->Position, o->Angle );
}


//////////////////////////////////////////////////////////////////////////
//  행동을 변경한다.
//////////////////////////////////////////////////////////////////////////
void    CSPetSystem::SetAI ( int AI )
{
    m_PetCharacter.Object.AI = AI;
    m_PetCharacter.Object.LifeTime = 0;
}


//////////////////////////////////////////////////////////////////////////
//  팻의 명령을 설정한다.
//////////////////////////////////////////////////////////////////////////
void    CSPetSystem::SetCommand ( int Key, BYTE cmd ) 
{ 
    m_byCommand = cmd; 
    if ( m_PetCharacter.Object.AI!=PET_ATTACK && m_PetCharacter.Object.AI!=PET_ATTACK_MAGIC )
    {
        m_PetCharacter.Object.m_bActionStart = false;
    }
    if ( cmd==PET_CMD_TARGET )
    {
        int Index = FindCharacterIndex ( Key );
        m_PetTarget = &CharactersClient[Index];
        
        m_PetCharacter.Object.m_bActionStart = true;
    }
}


//////////////////////////////////////////////////////////////////////////
//  팻에게 공격을 명령한다.
//////////////////////////////////////////////////////////////////////////
void    CSPetSystem::SetAttack ( int Key, int attackType )
{
    int Index = FindCharacterIndex ( Key );
    m_PetTarget = &CharactersClient[Index];
	OBJECT* Owner = &m_PetOwner->Object;

	if ( g_isCharacterBuff( Owner, eDeBuff_Stun) )
	{
		return;
	}
	else if ( g_isCharacterBuff( Owner, eBuff_Cloaking) )
	{
		m_PetCharacter.TargetCharacter = Index;
		SetAI ( PET_ATTACK+attackType );
		return;
	}

    m_PetCharacter.TargetCharacter = Index;
    m_PetCharacter.AttackTime = 0;    
    SetAI ( PET_ATTACK+attackType );

    //  근접 공격시 속도를 계산한다.
    if ( m_PetCharacter.Object.AI==PET_ATTACK )
    {
        OBJECT* o = &m_PetCharacter.Object;

        o->m_bActionStart = true;
        o->Velocity = rand()%10+20.f;
        o->Gravity  = 0.5f;

        PlayBuffer ( SOUND_DSPIRIT_RUSH );
    }
    else if ( m_PetCharacter.Object.AI==PET_ATTACK_MAGIC )
    {
        PlayBuffer ( SOUND_DSPIRIT_MISSILE );
    }
}



//////////////////////////////////////////////////////////////////////////
//  pet의 인벤토리를 처리한다.
//////////////////////////////////////////////////////////////////////////
void    CSPetSystem::MoveInventory ( void )
{
}


//////////////////////////////////////////////////////////////////////////
//  다크 스피릿.
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
CSPetDarkSpirit::CSPetDarkSpirit ( CHARACTER* c )
{
    m_PetType  = PET_TYPE_DARK_SPIRIT;
    m_PetOwner = c;

    m_PetCharacter.Object.BoneTransform = NULL;
    CreatePetPointer ( MODEL_DARK_SPIRIT, ( c->PositionX ), ( c->PositionY ), 0.f );

    m_PetCharacter.Object.Position[2] += 300.f;
    m_PetCharacter.Object.CurrentAction = 0;
}

#ifdef YDG_FIX_MEMORY_LEAK_0905_2ND
CSPetDarkSpirit::~CSPetDarkSpirit ( void )
{
	SAFE_DELETE_ARRAY(m_PetCharacter.Object.BoneTransform);
}
#endif	// YDG_FIX_MEMORY_LEAK_0905_2ND

//////////////////////////////////////////////////////////////////////////
//  Pet의 행동을 처리한다.
//////////////////////////////////////////////////////////////////////////
void    CSPetDarkSpirit::MovePet ( void )
{
    bool    Play;
    CHARACTER* c = &m_PetCharacter;
    OBJECT* o = &c->Object;
    OBJECT* Owner = &m_PetOwner->Object;

	if ( g_isCharacterBuff( Owner, eDeBuff_Stun) )
		return;

    BMD* b = &Models[Owner->Type];
    o->WeaponLevel = (BYTE)c->Level;

	// LHJ - 결투가 끝난 후 다크스피릿이 여전히 공격중이라면 다크 스피릿 공격 안하게 초기화
#ifdef YDG_ADD_NEW_DUEL_SYSTEM
	if(!g_DuelMgr.IsPetDuelEnabled())
#else	// YDG_ADD_NEW_DUEL_SYSTEM
	if(!g_PetEnableDuel)
#endif	// YDG_ADD_NEW_DUEL_SYSTEM
	{
		m_byCommand = 0;
		CSPetSystem* pPet = (CSPetSystem*)m_PetOwner->m_pPet;
		SendRequestPetCommand ( pPet->GetPetType(), AT_PET_COMMAND_DEFAULT, 0xffff );	// LHJ - 서버에 펫 공격 안하게 요청함
#ifdef YDG_ADD_NEW_DUEL_SYSTEM
		g_DuelMgr.EnablePetDuel(TRUE);
#else	// YDG_ADD_NEW_DUEL_SYSTEM
		g_PetEnableDuel = true;
#endif	// YDG_ADD_NEW_DUEL_SYSTEM
	}
	
	if ( c->TargetCharacter >= 0 && c->TargetCharacter<MAX_CHARACTERS_CLIENT )
	{
		CHARACTER* tc = &CharactersClient[c->TargetCharacter];

	if ( (g_isCharacterBuff((&tc->Object), eBuff_Cloaking)||g_isCharacterBuff(Owner,eBuff_Cloaking)) 
		&& ( o->AI==PET_ATTACK || o->AI==PET_ESCAPE || o->AI == PET_ATTACK_MAGIC ) )
		{
			CSPetSystem* pPet = (CSPetSystem*)m_PetOwner->m_pPet;
			//  주인과의 거리를 계산한다.
			float dx = o->Position[0]-Owner->Position[0];
			float dy = o->Position[1]-Owner->Position[1];
			float Distance = sqrtf(dx*dx+dy*dy);

			c->TargetCharacter = -1;
			SendRequestPetCommand ( pPet->GetPetType(), AT_PET_COMMAND_DEFAULT, 0xffff );
			SetAI ( PET_STAND );
			if ( Distance>50 || ( o->AI!=PET_STAND_START && o->AI>=PET_FLYING && o->AI<=PET_STAND ) )
			{
				SetAI ( PET_STAND_START );
				o->Velocity = 3.f;
			}
		}
	}
	else if((g_isCharacterBuff(Owner,eBuff_Cloaking))
		&& ( o->AI==PET_ATTACK || o->AI==PET_ESCAPE || o->AI == PET_ATTACK_MAGIC ))
	{
		CSPetSystem* pPet = (CSPetSystem*)m_PetOwner->m_pPet;
		//  주인과의 거리를 계산한다.
		float dx = o->Position[0]-Owner->Position[0];
		float dy = o->Position[1]-Owner->Position[1];
		float Distance = sqrtf(dx*dx+dy*dy);

		c->TargetCharacter = -1;
		SendRequestPetCommand ( pPet->GetPetType(), AT_PET_COMMAND_DEFAULT, 0xffff );
		SetAI ( PET_STAND );
		if ( Distance>50 || ( o->AI!=PET_STAND_START && o->AI>=PET_FLYING && o->AI<=PET_STAND ) )
		{
			SetAI ( PET_STAND_START );
			o->Velocity = 3.f;
		}
	}



    //  주인이 안전지대에 있다.
    if ( m_PetOwner->SafeZone==true )
    {
        if ( o->AI!=PET_STAND && o->AI!=PET_STAND_START )
        {
            //  주인과의 거리를 계산한다.
		    float dx = o->Position[0]-Owner->Position[0];
		    float dy = o->Position[1]-Owner->Position[1];
		    float Distance = sqrtf(dx*dx+dy*dy);

            SetAI ( PET_STAND );
            if ( Distance>50 || ( o->AI!=PET_STAND_START && o->AI>=PET_FLYING && o->AI<=PET_STAND ) )
            {
                SetAI ( PET_STAND_START );
                o->Velocity = 3.f;
            }
        }
    }
    else if ( o->AI==PET_STAND || o->AI==PET_STAND_START )
    {
        SetAI ( PET_FLYING );
    }

    //  애니메이션을 한다.
    Play = PlayAnimation ( o );
    if ( Play==false )
    {
        switch ( o->AI )
        {
        case PET_FLY:           SetAction ( o, 0 ); break;
        case PET_FLYING:        SetAction ( o, 1 ); break;
        case PET_STAND:         SetAction ( o, 2 ); break;
        case PET_STAND_START:   SetAction ( o, 1 ); break;
        case PET_ATTACK:
        case PET_ESCAPE:        SetAction ( o, 3 ); break;
        default :               SetAction ( o, 0 ); break;
        }
    }

	vec3_t  Range, TargetPosition;
    float   FlyRange = 150.f;

    if ( o->m_bActionStart==true )
    {
        OBJECT* to = &m_PetTarget->Object;

        VectorCopy ( to->Position, TargetPosition );
	    VectorSubtract ( TargetPosition, o->Position, Range );
    }
    else
    {
        VectorCopy ( Owner->Position, TargetPosition );
	    VectorSubtract ( TargetPosition, o->Position, Range );
    }

    //  일반 날기.
    if ( o->AI==PET_FLY || o->AI==PET_FLYING )
    {
        o->m_bActionStart = false;

        //  먼지 발생.
        vec3_t p, Pos, Light;
        Vector ( 0.3f, 0.4f, 0.7f, Light );
        Vector ( 0.f, 0.f, 0.f, p );

		if( !g_isCharacterBuff( Owner,eBuff_Cloaking ) )
		{
			for ( int i=0; i<1; ++i )
			{
				b->TransformPosition ( o->BoneTransform[rand()%66], p, Pos);
            
				CreateParticle ( BITMAP_SPARK+1, Pos, o->Angle, Light, 5, 0.8f );
			}
		}

	    float Distance = Range[0]*Range[0]+Range[1]*Range[1];
	    if ( Distance>=FlyRange*FlyRange )
	    {
		    float Angle = CreateAngle ( o->Position[0], o->Position[1], TargetPosition[0], TargetPosition[1] );
            o->Angle[2] = TurnAngle2 ( o->Angle[2], Angle, rand()%15+5.f );
	    }
	    AngleMatrix ( o->Angle, o->Matrix );
	    
        vec3_t Direction;
	    VectorRotate ( o->Direction, o->Matrix, Direction );
	    VectorAdd ( o->Position, Direction, o->Position );

        int speedRandom = 28;
        int CharacterHeight = 250;    

        if ( m_PetOwner->Helper.Type==ITEM_HELPER+4 || World == WD_55LOGINSCENE)
        {
            CharacterHeight = 350;
        }

        float Height = TargetPosition[2]+CharacterHeight;
        if ( o->Position[2]<Height )
        {
            speedRandom = 10;
            o->Angle[0]-=2.f;
            if ( o->Angle[0]<-15.f ) o->Angle[0] = -15.f;
        }
        else if ( o->Position[2]>Height+100 )
        {
            speedRandom = 20;
            o->Angle[0]+=2.f;
            if ( o->Angle[0]>15.f ) o->Angle[0] = 15.f;
        }

		float Speed = 0;
	    if ( rand()%speedRandom==0 )
	    {
     	    if ( Distance>=FlyRange*FlyRange )  //  주인과 멀리 떨어져있다.
            {
                Speed = -(float)(rand()%64+128)*0.1f;
            }
		    else
		    {
			    Speed = -(float)(rand()%8+32)*0.1f;
    		    o->Angle[2] += (float)(rand()%60);
		    }

            Speed += o->Direction[1];
            Speed = Speed/2.f;

		    o->Direction[0] = 0.f;
		    o->Direction[1] = Speed;
		    o->Direction[2] = (float)(rand()%64-32)*0.1f;
	    }

     	if ( o->Direction[1]<-12.f )  //  주인과 멀리 떨어져있다.
        {
            if ( o->AI!=PET_FLYING )
            {
                SetAI ( PET_FLYING );
            }
        }
        else if ( o->AI!=PET_FLY )
        {
            SetAI ( PET_FLY );
        }
    }
    else if ( o->AI==PET_ATTACK || o->AI==PET_ESCAPE )
    {
        vec3_t p, Pos;

        AngleMatrix ( o->Angle, o->Matrix );
        Vector(0.f,-o->Velocity,0.f,p);
        VectorRotate(p,o->Matrix,Pos );
        VectorAdd(o->Position,Pos,o->Position);

        if ( o->AI==PET_ATTACK )
        {
            TargetPosition[2] += 50.f;
            float Distance = MoveHumming ( o->Position, o->Angle, TargetPosition, o->Velocity );
            if ( Distance<20 || o->LifeTime>20 )
            {
                SetAI ( PET_ESCAPE );
                o->Angle[0] = -45.f;
                if ( m_byCommand!=PET_CMD_TARGET )
                {
                    o->m_bActionStart = false;
                }
            }
            o->Velocity += o->Gravity;
            o->Gravity += 0.2f;
            o->LifeTime++;
            }
        else if ( o->AI==PET_ESCAPE )
        {
	        float Distance = Range[0]*Range[0]+Range[1]*Range[1];
            if ( Distance>=(FlyRange+100)*(FlyRange+100) )
            {
                SetAI ( PET_FLYING );
            }
            o->Velocity -= 1.f;
        }
        SetAction ( o, 3 );
    }
    else if ( o->AI==PET_ATTACK_MAGIC )
    {
        //  마법 공격을 한다.
        if ( c->TargetCharacter!=-1 )
        {
            CHARACTER* tc = &CharactersClient[c->TargetCharacter];
            OBJECT* to = &tc->Object;

            float Angle = CreateAngle ( o->Position[0], o->Position[1], to->Position[0], to->Position[1] );
            o->Angle[2] = TurnAngle2 ( o->Angle[2], Angle, 40.f );
        }
    }
    else if ( o->AI==PET_STAND )
    {
        vec3_t p;

        Vector ( -10.f, 0.f, 10.f, p );
        b->TransformPosition ( Owner->BoneTransform[37], p, o->Position, true );
        VectorCopy ( Owner->Angle, o->Angle );
        o->Angle[2] -= 120.f;
    }
    //  손에 가기 위한 준비.
    else if ( o->AI==PET_STAND_START )
    {
        vec3_t p, Pos;

        AngleMatrix ( o->Angle, o->Matrix );
        if ( o->Velocity!=0.0f )
        {
            Vector(0.f,-o->Velocity,0.f,p);
            VectorRotate(p,o->Matrix,Pos );
            VectorAdd(o->Position,Pos,o->Position);
        }
        Vector ( 0.f, 0.f, 0.f, p );
        b->TransformPosition ( Owner->BoneTransform[42], p, Pos, true );

        float Distance = MoveHumming ( o->Position, o->Angle, Pos, o->Velocity );
        o->Velocity++;
        if ( Distance<50 )
        {
            SetAI ( PET_STAND );
        }
    }

    //  공격.
    if ( o->AI>=PET_ATTACK && o->AI<=PET_ATTACK_MAGIC )
    {
        c->AttackTime++;
        if ( c->AttackTime>=15 )
        {
            c->AttackTime = 15;
        }
    }

    //  사운드 처리.
    if ( (rand()%100)==0 && (MoveSceneFrame%60)==0 )
    {
        PlayBuffer ( SOUND_DSPIRIT_SHOUT, o );
    }

    //  다크스피릿이 사라졌을때의 처리.
    VectorCopy ( m_PetOwner->Object.Position, TargetPosition );
	VectorSubtract ( TargetPosition, o->Position, Range );
	float Distance = Range[0]*Range[0]+Range[1]*Range[1];
    if ( o->Position[2]<(TargetPosition[2]-200.f) || Distance>409600.f )
    {
        o->LifeTime++;
    }
    if ( o->LifeTime>90 )
    {
        o->LifeTime = 0;
        VectorCopy ( TargetPosition, o->Position );
        o->Position[2] += 250.f;
    }
}


//////////////////////////////////////////////////////////////////////////
//  팻의 정보를 계산한다.
//////////////////////////////////////////////////////////////////////////
#ifdef KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER
void CSPetDarkSpirit::CalcPetInformation ( const PET_INFO& Petinfo )
#else // KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER
void    CSPetDarkSpirit::CalcPetInformation ( BYTE Level, int exp )
#endif // KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER
{
#ifndef KJH_FIX_DARKLOAD_PET_SYSTEM															//## 소스정리 대상임.
    //  주인 정보.
    int Charisma = CharacterAttribute->Charisma+CharacterAttribute->AddCharisma;
    int CharLevel= CharacterAttribute->Level;

    //  기본 정보 설정.
#ifdef KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER
	int Level2              = Petinfo.m_wLevel+1;
    m_PetCharacter.Level    = Petinfo.m_wLevel;
    m_PetInfo.m_wLevel      = Petinfo.m_wLevel;
    m_PetInfo.m_wLife       = Petinfo.m_wLife;
    m_PetInfo.m_dwExp1      = Petinfo.m_dwExp1;
#else // KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER
    int Level2              = Level+1;
    m_PetCharacter.Level    = Level;
    m_PetInfo.m_wLevel      = Level;

    m_PetInfo.m_wLife       = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Durability;


    m_PetInfo.m_dwExp1      = exp;
#endif // KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER

    m_PetInfo.m_dwExp2      = ((10+Level2)*Level2*Level2*Level2*100);
    m_PetInfo.m_wDamageMax  = (200+(m_PetInfo.m_wLevel*15)+(Charisma/4));
    m_PetInfo.m_wDamageMin  = (180+(m_PetInfo.m_wLevel*15)+(Charisma/8));
    m_PetInfo.m_wAttackSpeed= (20+(m_PetInfo.m_wLevel*4/5)+(Charisma/50));
    m_PetInfo.m_wAttackSuccess = (1000+m_PetInfo.m_wLevel)+(m_PetInfo.m_wLevel*15);
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM														//## 소스정리 대상임.
}


//////////////////////////////////////////////////////////////////////////
//  Pet에 대한 정보 화면에 표시.
//////////////////////////////////////////////////////////////////////////
void    CSPetDarkSpirit::RenderPetInventory ( void )
{
    //  현재 명령 상태를 화면에 나타낸다.
    RenderCmdType ();
}


//////////////////////////////////////////////////////////////////////////
//  Pet을 화면에 출력한다.
//////////////////////////////////////////////////////////////////////////
void    CSPetDarkSpirit::RenderPet ( int PetState )
{
    CHARACTER* c = &m_PetCharacter;
    OBJECT* o    = &c->Object;

#ifdef PBG_FIX_DARKPET_RENDER
	if(m_pPetInfo)
		o->WeaponLevel = m_pPetInfo->m_wLevel;
#endif //PBG_FIX_DARKPET_RENDER
	if ( o->Live )
	{
		o->Visible = TestFrustrum2D ( o->Position[0]*0.01f, o->Position[1]*0.01f, -20.f );
		if ( o->Visible )
		{
			if ( m_PetOwner->Object.Type!=MODEL_PLAYER && o->Type!=MODEL_DARK_SPIRIT )
				return;

			RenderObject ( o, false,0, PetState);
            AttackEffect ( c, o );
		}
        else
        {
            o->Velocity = 3.f;
            if ( o->AI!=PET_STAND_START )
            {
                o->AI = PET_FLYING;
            }
        }
	}
}


//////////////////////////////////////////////////////////////////////////
//  레벨업효과.
//////////////////////////////////////////////////////////////////////////
void    CSPetDarkSpirit::Eff_LevelUp ( void )
{
    OBJECT* o    = &m_PetCharacter.Object;

    vec3_t Angle    = { 0.f, 0.f, 0.f };
    vec3_t Position = { o->Position[0], o->Position[1], o->Position[2] };

    for ( int i=0; i<5; ++i )
    {
        CreateJoint ( BITMAP_FLARE, Position, Position, Angle, 0, o, 40, 2 );
    }
}


//////////////////////////////////////////////////////////////////////////
//  레벨다운 효과.
//////////////////////////////////////////////////////////////////////////
void    CSPetDarkSpirit::Eff_LevelDown ( void )
{
    OBJECT* o    = &m_PetCharacter.Object;

    vec3_t Position = { o->Position[0], o->Position[1], o->Position[2] };

    for ( int i=0; i<15; ++i )
    {
        CreateJoint(BITMAP_FLARE,Position,o->Position,o->Angle,0,o,40,2);
    }
}


//////////////////////////////////////////////////////////////////////////
//  현재 설정된 명령 상태를 보여준다.
//////////////////////////////////////////////////////////////////////////
void    CSPetDarkSpirit::RenderCmdType ( void )
{
    float   x, y, Width, Height;
    float   PartyWidth = 0.f;

	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(220, 220, 220, 255);
	g_pRenderText->SetBgColor(0, 0, 0, 128);

    if ( PartyNumber>0 )
    {
        PartyWidth = 50.f;
    }
    if ( (Hero->Helper.Type>=MODEL_HELPER+0 && Hero->Helper.Type<=MODEL_HELPER+4)
		|| Hero->Helper.Type == MODEL_HELPER+37	//^ 펜릴 인터페이스 관련(다크스피릿 예외)
		)
    {
        PartyWidth += 60.f;
    }


	int Dur = 255;
    Width=50;Height=2;x=GetScreenWidth()-Width-PartyWidth-15;y=4;
	int Life = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Durability*(int)Width/Dur;

    EnableAlphaTest();
    
	g_pRenderText->RenderText((int)x+50, (int)y, GlobalText[1214], 0, 0, RT3_WRITE_RIGHT_TO_LEFT);
    
	RenderBar ( x, y+12, Width, Height, (float)Life );

    glColor3f ( 1.f, 1.f, 1.f );

    Width=20.f; Height=28.f; x=GetScreenWidth()-Width-PartyWidth-65.f; y=5.f; 
    RenderBitmap ( BITMAP_SKILL_INTERFACE+2, (float)x, (float)y, (float)Width-4, (float)Height-8, (((m_byCommand)%8)*32+6.f)/256.f, (((m_byCommand)/8)*Height+3.f)/256.f,Width/256.f,(Height-1.f)/256.f);

    Width -= 8.f; Height -= 8.f;
    //  명령 설명을 표시한다.
    if ( MouseX>=x && MouseX<=x+Width && MouseY>=y && MouseY<=y+Height )
    {
        RenderTipText ( (int)x, (int)(y+Height), GlobalText[1219+m_byCommand] );
    }
}


//////////////////////////////////////////////////////////////////////////
//  Pet공격 효과.
//////////////////////////////////////////////////////////////////////////
void    CSPetDarkSpirit::AttackEffect ( CHARACTER* c, OBJECT* o )
{
    BMD* b = &Models[o->Type];
    vec3_t p, Pos, Light;

    switch ( o->AI )
    {
    case PET_ATTACK:
		if ( c->AttackTime>=0 && c->AttackTime<=2 && m_PetTarget!=NULL )
        {
            for( int i=0;i<10;i++)
            {
			    CreateJoint ( BITMAP_LIGHT, o->Position, o->Position, o->Angle, 1, NULL, (float)(rand()%40+20) );
            }
            if ( c->AttackTime==1 )
            {
                vec3_t Angle, Light;

                Vector ( 45.f, (float)(rand()%180)-90, 0.f, Angle );
                Vector ( 1.f, 0.8f, 0.6f, Light );
                CreateEffect ( MODEL_DARKLORD_SKILL, o->Position, Angle, Light, 3 );
            }
        }
        if ( c->AttackTime>3 && c->AttackTime%2 )
        {
            if ( o->Position[2]>(m_PetOwner->Object.Position[2]+100.f ) )
            {
                Vector ( 50.f, 0.f, 0.f, p );
                b->TransformPosition ( o->BoneTransform[6], p, Pos);

	            CreateEffect ( MODEL_AIR_FORCE, Pos, o->Angle, o->Light, 0, o );
            }
        }
		break;

    case PET_ATTACK_MAGIC:
        if ( c->AttackTime<=14 )
        {
            if ( o->BoneTransform!=NULL )
            {
                Vector ( 1.f, 0.6f, 0.4f, Light );
                Vector ( 0.f, 0.f, 0.f, p );
                for ( int i=rand()%2; i<66; i+=2 )
                {
                    if ( !b->Bones[i].Dummy && i<b->NumBones )
                    {
                        b->TransformPosition ( o->BoneTransform[i], p, Pos );
                        CreateParticle( BITMAP_LIGHT, Pos, o->Angle, Light, 6, 1.3f );
                    }
                }
            }
        }
        else if ( c->AttackTime==15 )
        {
			if(c->TargetCharacter != -1)
			{
				CHARACTER* tc = &CharactersClient[c->TargetCharacter];
				OBJECT* to = &tc->Object;

				if ( to!=NULL )
				{
#ifdef LDS_ADD_RENDERMESHEFFECT_FOR_VBO
					b->RenderMeshEffect ( 1, BITMAP_LIGHT, 1, o->Angle, to, o );
#else // LDS_ADD_RENDERMESHEFFECT_FOR_VBO
					b->RenderMeshEffect ( 1, BITMAP_LIGHT, 1, o->Angle, to );
#endif // LDS_ADD_RENDERMESHEFFECT_FOR_VBO
				}

				SetAI ( PET_FLYING );
			}
            c->AttackTime = 0;
        }
        break;

    default :
        break;
    }
}


#endif	// PET_SYSTEM
