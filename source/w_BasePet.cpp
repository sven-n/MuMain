// w_BasePet.cpp: implementation of the BasePet class.
// LDK_2008/07/08
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef LDK_ADD_NEW_PETPROCESS

#include "w_BasePet.h"

#include "ZzzLodTerrain.h"
#include "ZzzObject.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"

extern float EarthQuake;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
PetObjectPtr PetObject::Make()
{
	PetObjectPtr PetObject( new PetObject );
	PetObject->Init();
	return PetObject;
}

PetObject::PetObject():
m_moveType(eAction_Stand),
#ifdef LDK_MOD_PETPROCESS_SYSTEM
m_oldMoveType(eAction_End),
#endif //LDK_MOD_PETPROCESS_SYSTEM
m_startTick(0),
m_targetKey(-1),
m_pOwner(NULL),
m_itemType(-1)
{
#ifdef LDK_MOD_NUMBERING_PETCREATE
	m_regstKey = 0;
#endif //LDK_MOD_NUMBERING_PETCREATE
}

PetObject::~PetObject()
{
	Release();
}

void PetObject::Init()
{
	m_obj = new OBJECT();
}

//////////////////////////////////////////////////////////////////////
// 초기 설정 scale값 등록
//////////////////////////////////////////////////////////////////////
void PetObject::SetScale( float scale )
{
	if ( NULL == m_obj ) return;

    m_obj->Scale = scale;
}
//////////////////////////////////////////////////////////////////////
// 초기 설정 blendMesh값 등록
//////////////////////////////////////////////////////////////////////
void PetObject::SetBlendMesh( int blendMesh )
{
	if ( NULL == m_obj ) return;

    m_obj->BlendMesh = blendMesh;
}

//////////////////////////////////////////////////////////////////////
// 생성 : 정확히는 초기화 및 링크??
//////////////////////////////////////////////////////////////////////
bool PetObject::Create( int itemType, int modelType, vec3_t Position, CHARACTER *Owner, int SubType, int LinkBone )
{
	assert( Owner );
	if( m_obj->Live ) return FALSE;

	m_startTick = GetTickCount();

	m_pOwner = Owner;
	m_itemType = itemType;

	m_obj->Type             = modelType;
	m_obj->Live             = TRUE;
	m_obj->Visible          = FALSE;
	m_obj->LightEnable      = TRUE;
	m_obj->ContrastEnable   = FALSE;
	m_obj->AlphaEnable      = FALSE;
	m_obj->EnableBoneMatrix = FALSE;
	m_obj->Owner            = &m_pOwner->Object;
	m_obj->SubType          = SubType;
	m_obj->HiddenMesh       = -1;
	m_obj->BlendMesh        = -1;
	m_obj->BlendMeshLight   = 1.f;
    m_obj->Scale            = 0.7f;
	m_obj->LifeTime         = 30;
	m_obj->Alpha            = 0.f;
	m_obj->AlphaTarget      = 1.f;

	VectorCopy ( Position, m_obj->Position );
	VectorCopy ( m_obj->Owner->Angle, m_obj->Angle );
	Vector ( 3.f, 3.f, 3.f, m_obj->Light );

	m_obj->PriorAnimationFrame = 0.f;
	m_obj->AnimationFrame      = 0.f;
	m_obj->Velocity            = 0.5f;

#ifdef YDG_FIX_SKELETON_PET_CREATE_POSITION
	switch(m_obj->Type)
	{
	case MODEL_HELPER+123:	// 스켈레톤 팻 초기 위치 수정
		m_obj->Position[1] += (60.0f * Owner->Object.Scale);
		break;
	}
#endif	// YDG_FIX_SKELETON_PET_CREATE_POSITION

//  	m_obj->BoneTransform = new vec34_t [Models[modelType].NumBones];
// 
//  	BMD* b = &Models[modelType];
// 	for(int i=0; i<b->NumBones; i++)
// 	{
// 		memcpy( &m_obj->BoneTransform[i], &b->Bones[i], sizeof(Bone_t));
// 	}
// 
// 	memcpy( m_obj->BoneTransform, b, sizeof(m_obj->BoneTransform));
// 	VectorCopy(m_obj->Position,b->BodyOrigin);
// 
	return TRUE;
}
//////////////////////////////////////////////////////////////////////
// 삭제 : PetAction에서 effect생성시 삭제 잘하자.
//////////////////////////////////////////////////////////////////////
void PetObject::Release()
{
	if(NULL != m_obj)
	{
		m_obj->Live = FALSE;
		delete m_obj;
		m_obj = NULL;
	}
	
	for( ActionMap::iterator iter = m_actionMap.begin(); iter != m_actionMap.end(); iter ++ )
	{
  		BoostWeak_Ptr(PetAction) petAction = (*iter).second;

		if( petAction.expired() == FALSE ) 
		{
 			petAction.lock()->Release( m_obj, m_pOwner );
 		}
	}
	m_actionMap.clear();

	m_speedMap.clear();
}

//////////////////////////////////////////////////////////////////////
// 갱신
//////////////////////////////////////////////////////////////////////
void PetObject::Update(bool bForceRender)
{
	if ( !m_obj->Live || NULL == m_obj->Owner || NULL == m_obj ) return;

	if ( SceneFlag==MAIN_SCENE )
    {
      	if ( !m_obj->Owner->Live || m_obj->Owner->Kind!=KIND_PLAYER )
		{
	    	m_obj->Live = FALSE;
			return;
		}
    }

	//공통 기능들 처리
	Alpha ( m_obj );

	//-----------------------------//
	DWORD tick = GetNowTick();
	UpdateModel( tick, bForceRender );
	UpdateMove( tick, bForceRender );
#ifdef LDK_ADD_NEW_PETPROCESS_ADD_SOUND
	UpdateSound( tick, bForceRender );
#endif //LDK_ADD_NEW_PETPROCESS_ADD_SOUND
}

//////////////////////////////////////////////////////////////////////
// RenderUpdate 함수
//////////////////////////////////////////////////////////////////////
void PetObject::Render( bool bForceRender )
{
	if(m_obj->Live)
	{
		m_obj->Visible = (bForceRender == FALSE ? TestFrustrum2D(m_obj->Position[0]*0.01f,m_obj->Position[1]*0.01f,-20.f) : TRUE);

		if(m_obj->Visible)
		{
#ifndef LDK_FIX_HIDE_PET_TO_NOT_MODEL_PLAYER //not defined 
 			if ( m_obj->Owner->Type!=MODEL_PLAYER ) return;
#endif LDK_FIX_HIDE_PET_TO_NOT_MODEL_PLAYER

			//State가 10이면 Draw_RenderObject에 공성전 디버프(클럭킹??) 처리하는부분이 있음.. ㅡ.ㅡ;;;	
			int State = g_isCharacterBuff(m_obj->Owner, eBuff_Cloaking)? 10 : 0;

#ifdef LDS_MR0_FIX_ALLPETOBJECT_RENDERINGPASSEDWAY
			EngineGate::SetOn( false );		// 기존 방법으로 그리기.
#endif // LDS_MR0_FIX_ALLPETOBJECT_RENDERINGPASSEDWAY

			RenderObject ( m_obj , FALSE, 0, State);

#ifdef LDS_MR0_FIX_ALLPETOBJECT_RENDERINGPASSEDWAY
			EngineGate::SetOn( true );
#endif // LDS_MR0_FIX_ALLPETOBJECT_RENDERINGPASSEDWAY
			
			// 순서에 따라서 본좌표 뽑을때 문제 생길수있음...
			DWORD tick = GetNowTick();
			CreateEffect( tick, bForceRender );
		}
	}
}

bool PetObject::IsSameOwner( OBJECT *Owner )
{
	assert( Owner );
	
	return ( Owner == m_obj->Owner )? TRUE : FALSE;
}

bool PetObject::IsSameObject( OBJECT *Owner, int itemType )
{
	assert( Owner );

	return ( Owner == m_obj->Owner && itemType == m_itemType )? TRUE : FALSE;
}

//////////////////////////////////////////////////////////////////////
// action의 주소를 등록
//////////////////////////////////////////////////////////////////////
void PetObject::SetActions( ActionType type, BoostWeak_Ptr(PetAction) action, float speed )
{
	if( action.expired() == TRUE ) return;
	
	m_actionMap.insert(std::make_pair( type, action ) );
	m_speedMap.insert(std::make_pair( type, speed ) );
}

//////////////////////////////////////////////////////////////////////
// 행동 변경값 등록
//////////////////////////////////////////////////////////////////////
void PetObject::SetCommand( int targetKey, ActionType cmdType )
{
	m_targetKey = targetKey;
	m_moveType = cmdType;
}

//////////////////////////////////////////////////////////////////////
// 내부 MoveUpdate 함수 : 이동에 관해서만 정의 하자
//////////////////////////////////////////////////////////////////////
bool PetObject::UpdateMove( DWORD tick, bool bForceRender )
{
#ifdef LDK_MOD_PETPROCESS_SYSTEM
	if( m_oldMoveType != m_moveType )
	{
		m_oldMoveType = m_moveType;

		//speed 변경
		SpeedMap::iterator iter2 = m_speedMap.find(m_moveType);
		if( iter2 == m_speedMap.end() ) return FALSE;
		
		m_obj->Velocity = (*iter2).second;
	}
#else
	//speed 변경
	SpeedMap::iterator iter2 = m_speedMap.find(m_moveType);
	if( iter2 == m_speedMap.end() ) return FALSE;
	
	m_obj->Velocity = (*iter2).second;

#endif //LDK_MOD_PETPROCESS_SYSTEM

	//action 변경
	ActionMap::iterator iter = m_actionMap.find(m_moveType);
	if( iter == m_actionMap.end() )
	{
		m_moveType = eAction_Stand;
		return FALSE;
	}

	//std::tr1::weak_ptr<PetAction> petAction = (*iter).second;

  	BoostWeak_Ptr(PetAction) petAction = (*iter).second;

	if( petAction.expired() == FALSE ) 
	{
 		petAction.lock()->Move( m_obj, m_pOwner, m_targetKey, tick, bForceRender );
 	}
	
	return TRUE;
}
//////////////////////////////////////////////////////////////////////
// 내부 ModelUpdate 함수 : 모델의 변경과 애니에 관해서만 정의 하자
//////////////////////////////////////////////////////////////////////
bool PetObject::UpdateModel( DWORD tick, bool bForceRender )
{
	ActionMap::iterator iter = m_actionMap.find(m_moveType);
	if( iter == m_actionMap.end() ) return FALSE;

  	BoostWeak_Ptr(PetAction) petAction = (*iter).second;

	if( petAction.expired() == FALSE ) 
	{
 		petAction.lock()->Model( m_obj, m_pOwner, m_targetKey, tick, bForceRender );
 	}

	BMD* b = &Models[m_obj->Type];
    b->CurrentAction = m_obj->CurrentAction;
   	b->PlayAnimation(&m_obj->AnimationFrame,&m_obj->PriorAnimationFrame,&m_obj->PriorAction,m_obj->Velocity,m_obj->Position,m_obj->Angle);

	return TRUE;
}

#ifdef LDK_ADD_NEW_PETPROCESS_ADD_SOUND
//////////////////////////////////////////////////////////////////////
// 내부 SoundUpdate 함수 : 사운드 생성에 관해서만 정의 하자
//////////////////////////////////////////////////////////////////////
bool PetObject::UpdateSound( DWORD tick, bool bForceRender )
{
	ActionMap::iterator iter = m_actionMap.find(m_moveType);
	if( iter == m_actionMap.end() ) return FALSE;
	
	BoostWeak_Ptr(PetAction) petAction = (*iter).second;

	if( petAction.expired() == FALSE ) 
	{
 		petAction.lock()->Sound( m_obj, m_pOwner, m_targetKey, tick, bForceRender );
	}

	return TRUE;
}
#endif //LDK_ADD_NEW_PETPROCESS_ADD_SOUND

//////////////////////////////////////////////////////////////////////
// 내부 EffectUpdate 함수 : 이펙트 생성에 관해서만 정의 하자
//////////////////////////////////////////////////////////////////////
bool PetObject::CreateEffect( DWORD tick, bool bForceRender )
{
	ActionMap::iterator iter = m_actionMap.find(m_moveType);
	if( iter == m_actionMap.end() ) return FALSE;
	
	BoostWeak_Ptr(PetAction) petAction = (*iter).second;

	if( petAction.expired() == FALSE ) 
	{
 		petAction.lock()->Effect( m_obj, m_pOwner, m_targetKey, tick, bForceRender );
	}

	return TRUE;
}

#endif //LDK_ADD_NEW_PETPROCESS
