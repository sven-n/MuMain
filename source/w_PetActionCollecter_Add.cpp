// w_PetActionCollecter.cpp: implementation of the PetActionStand class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef PJH_ADD_PANDA_PET

#include "w_PetActionCollecter_Add.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "ZzzCharacter.h"
#include "ZzzObject.h"
#include "wsclientinline.h"
#include "DSPlaySound.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

PetActionCollecterAddPtr PetActionCollecterAdd::Make()
{
	PetActionCollecterAddPtr temp( new PetActionCollecterAdd );
	return temp;
}

PetActionCollecterAdd::PetActionCollecterAdd()
{
	m_isRooting = false;

	m_dwSendDelayTime = 0;
	m_dwRootingTime = 0;
	m_dwRoundCountDelay = 0;
#ifdef _VS2008PORTING
	m_state = eAction_Stand;
#else // _VS2008PORTING
	m_state = ActionState::eAction_Stand;
#endif // _VS2008PORTING

	m_fRadWidthStand = 0.0f;
	m_fRadWidthGet = 0.0f;
}

PetActionCollecterAdd::~PetActionCollecterAdd()
{

}

bool PetActionCollecterAdd::Release( OBJECT* obj, CHARACTER *Owner )
{
	DeleteEffect(MODEL_NEWYEARSDAY_EVENT_MONEY, obj);

	return TRUE;
}

bool PetActionCollecterAdd::Model( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender )
{
	if( NULL == obj || NULL == Owner ) return FALSE;

	return false;
}

bool PetActionCollecterAdd::Move( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender )
{
	if( NULL == obj || NULL == Owner ) return FALSE;

	FindZen(obj);

#ifdef _VS2008PORTING
	if( eAction_Stand == m_state && m_isRooting )
	{
		m_state = eAction_Move;
	}
#else // _VS2008PORTING
	if( ActionState::eAction_Stand == m_state && m_isRooting )
 	{
		m_state = ActionState::eAction_Move;
	}
#endif // _VS2008PORTING

	//------------------------------------------//
	float FlyRange = 10.0f;
	vec3_t targetPos, Range, Direction;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	bool _isMove = false;
	float fRadHeight = ((2*3.14f)/15000.0f) * (float)(tick%15000);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	m_fRadWidthStand = ((2*3.14f)/4000.0f) * (float)(tick%4000);
	m_fRadWidthGet = ((2*3.14f)/2000.0f) * (float)(tick%2000);
	
	obj->Position[2] = obj->Owner->Position[2] + 20.0f;	
	//------------------------------------------//
	VectorSubtract( obj->Position, obj->Owner->Position, Range );

	float Distance = sqrtf( Range[0]*Range[0] + Range[1]*Range[1] );
	if( Distance > SEARCH_LENGTH*3)
	{
		//맵 이동시 ...
		obj->Position[0] = obj->Owner->Position[0] + (sinf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN);
		obj->Position[1] = obj->Owner->Position[1] + (cosf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN);
		
		VectorCopy ( obj->Owner->Angle, obj->Angle );

#ifdef _VS2008PORTING
		m_state = eAction_Stand;
#else // _VS2008PORTING
		m_state = ActionState::eAction_Stand;
#endif // _VS2008PORTING
		m_isRooting = false;
	}
				
	switch(m_state)
	{
#ifdef _VS2008PORTING
	case eAction_Stand:
#else // _VS2008PORTING
	case ActionState::eAction_Stand:
#endif // _VS2008PORTING
		{
			targetPos[0] = obj->Owner->Position[0] + (sinf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN);
			targetPos[1] = obj->Owner->Position[1] + (cosf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN);
			targetPos[2] = obj->Owner->Position[2];
			
			VectorSubtract( targetPos, obj->Position, Range );
			//------------------------------//
			Distance = sqrtf( Range[0]*Range[0] + Range[1]*Range[1] );
			
			if(80.0f >= FlyRange)
			{
				float Angle = CreateAngle( obj->Position[0],obj->Position[1], targetPos[0],targetPos[1] ); //test
				obj->Angle[2] = TurnAngle2( obj->Angle[2],Angle, 8.0f );
			}
			
			AngleMatrix(obj->Angle,obj->Matrix);
			VectorRotate(obj->Direction,obj->Matrix,Direction);
			VectorAdd(obj->Position,Direction,obj->Position);
			
			float Speed = ( FlyRange >= Distance ) ?  0 : (float)log(Distance) * 2.3f;
						
			obj->Direction[0] = 0.0f;
			obj->Direction[1] = -Speed;
			obj->Direction[2] = 0.0f;
			//*/
		}
		break;

#ifdef _VS2008PORTING
	case eAction_Move:
#else // _VS2008PORTING
	case ActionState::eAction_Move:
#endif // _VS2008PORTING
		{
			if( !m_isRooting )
			{
				m_isRooting = false;
#ifdef _VS2008PORTING
				m_state = eAction_Return;
#else // _VS2008PORTING
				m_state = ActionState::eAction_Return;
#endif // _VS2008PORTING
				break;
			}

			targetPos[0] = m_RootItem.position[0] + (sinf(m_fRadWidthGet) * CIRCLE_STAND_RADIAN);
			targetPos[1] = m_RootItem.position[1] + (cosf(m_fRadWidthGet) * CIRCLE_STAND_RADIAN);
			targetPos[2] = m_RootItem.position[2];// + 70 + (sinf(fRadHeight) * 70.0f);
			
			VectorSubtract( targetPos, obj->Position, Range );
			//------------------------------//

			Distance = sqrtf( Range[0]*Range[0] + Range[1]*Range[1] );
			if(Distance >= FlyRange)
			{
				float Angle = CreateAngle( obj->Position[0],obj->Position[1], targetPos[0],targetPos[1] ); //test
				obj->Angle[2] = TurnAngle2( obj->Angle[2],Angle, 20.0f );
			}
			
			AngleMatrix(obj->Angle,obj->Matrix);
			VectorRotate(obj->Direction,obj->Matrix,Direction);
			VectorAdd(obj->Position,Direction,obj->Position);
			
			float Speed = ( 20.0f >= Distance ) ?  0 : (float)log(Distance) * 2.5f;
			
			obj->Direction[0] = 0.0f;
			obj->Direction[1] = -Speed;
			obj->Direction[2] = 0.0f;
			
			if(	0 == Speed || CompTimeControl(100000, m_dwRootingTime) )
			{
				m_dwSendDelayTime = GetTickCount();
				m_dwRootingTime = GetTickCount();
#ifdef _VS2008PORTING
				m_state = eAction_Get;
#else // _VS2008PORTING
				m_state = ActionState::eAction_Get;
#endif // _VS2008PORTING
			}
		}
		break;

#ifdef _VS2008PORTING
	case eAction_Get: //줍는 행동
#else // _VS2008PORTING
	case ActionState::eAction_Get: //줍는 행동
#endif // _VS2008PORTING

		{
			if(	!m_isRooting || SEARCH_LENGTH < Distance || CompTimeControl(3000, m_dwRootingTime))
			{
				m_isRooting = false;
				m_dwRootingTime = GetTickCount();
#ifdef _VS2008PORTING
				m_state = eAction_Return;
#else // _VS2008PORTING
				m_state = ActionState::eAction_Return;
#endif // _VS2008PORTING
				break;
			}

			//------------------------------//
			VectorCopy ( m_RootItem.position, targetPos );

			float Angle = CreateAngle(obj->Position[0], obj->Position[1], targetPos[0], targetPos[1] );
			obj->Angle[2] = TurnAngle2(obj->Angle[2], Angle, 10.0f);
			//------------------------------//

			if(CompTimeControl(1000, m_dwSendDelayTime))
			{
#ifdef LJH_FIX_GETTING_ZEN_WITH_PET_OF_OTHER_PLAYER
				if(&Hero->Object == obj->Owner)
#endif //LJH_FIX_GETTING_ZEN_WITH_PET_OF_OTHER_PLAYER
					//1000ms 마다 패킷 보냄
					SendRequestGetItem(m_RootItem.itemIndex);
			}	
		}
		break;

#ifdef _VS2008PORTING
	case eAction_Return:
#else // _VS2008PORTING
	case ActionState::eAction_Return:
#endif // _VS2008PORTING
		{
			targetPos[0] = obj->Owner->Position[0] + (sinf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN);
			targetPos[1] = obj->Owner->Position[1] + (cosf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN);
			targetPos[2] = obj->Owner->Position[2];// + 70 + (sinf(fRadHeight) * 70.0f);

			VectorSubtract( targetPos, obj->Position, Range );
			//------------------------------//

			Distance = sqrtf( Range[0]*Range[0] + Range[1]*Range[1] );
			if(Distance >= FlyRange)
			{
				float Angle = CreateAngle( obj->Position[0],obj->Position[1], targetPos[0],targetPos[1] ); //test
				obj->Angle[2] = TurnAngle2( obj->Angle[2], Angle, 20.0f );
			}
			
			AngleMatrix( obj->Angle, obj->Matrix );
			VectorRotate( obj->Direction, obj->Matrix, Direction );
			VectorAdd( obj->Position, Direction, obj->Position );
			
			float Speed = ( FlyRange >= Distance ) ?  0 : (float)log(Distance) * 2.5f;
			
			obj->Direction[0] = 0.0f;
			obj->Direction[1] = -Speed;
			obj->Direction[2] = 0.0f;

			if(	0 == Speed || CompTimeControl(3000, m_dwRootingTime) )
			{
#ifdef _VS2008PORTING
				m_state = eAction_Stand;
#else // _VS2008PORTING
				m_state = ActionState::eAction_Stand;
#endif // _VS2008PORTING
			}
		}
		break;
	}
	
	return TRUE;
}	

bool PetActionCollecterAdd::Effect( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender )
{
	if( NULL == obj || NULL == Owner ) return FALSE;

#ifdef PJH_ADD_PANDA_PET

	BMD* b = &Models[obj->Type];
	vec3_t Position, vRelativePos, Light;

	VectorCopy(obj->Position,b->BodyOrigin);
	Vector(0.f, 0.f, 0.f, vRelativePos);
	
	b->Animation(BoneTransform,obj->AnimationFrame,obj->PriorAnimationFrame,obj->PriorAction, obj->Angle, obj->HeadAngle);

	float fRad1 = ((3.14f/3000.0f) * (float)(tick%3000));
	float fSize = sinf(fRad1) * 0.2f;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	float fSize2 = 1.0f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

	Vector( 1.f, 1.f, 1.f, Light);
	VectorCopy(obj->Position, Position);


	Vector(0.f, 0.f, 0.f, vRelativePos);
	b->TransformPosition(BoneTransform[7], vRelativePos, Position, false);

	//CreateParticle(BITMAP_LIGHT+3, Position, obj->Angle, Light, 7 );
	if(rand()%3 == 0)
	{
		Vector( 0.6f, 1.0f, 0.4f, Light);
		CreateParticle(BITMAP_LIGHT+3, Position, obj->Angle, Light, 1, 1.f);
	}

	//코
	Vector(0.f, 0.f, 0.f, vRelativePos);
	b->TransformPosition(BoneTransform[4], vRelativePos, Position, false);
	Vector( 0.9f, 0.9f, 0.0f, Light);
	CreateSprite(BITMAP_LIGHT, Position, (1.5f + fSize), Light, obj);
	Vector( 0.6f, 1.0f, 0.2f, Light);
	CreateSprite(BITMAP_LIGHT, Position, (2.5f + fSize), Light, obj);
#endif //PJH_ADD_PANDA_PET
	return TRUE;
}

bool PetActionCollecterAdd::Sound( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender )
{
 	if( NULL == obj || NULL == Owner ) return FALSE;
	
	switch(m_state)
 	{
#ifdef _VS2008PORTING
	case eAction_Return:
#else // _VS2008PORTING
	case ActionState::eAction_Return:
#endif // _VS2008PORTING
		PlayBuffer(SOUND_DROP_GOLD01);
 		break;
 	}

	return TRUE;
}

void PetActionCollecterAdd::FindZen(OBJECT* obj)
{
	if( NULL == obj || true == m_isRooting ) return;

	float dx, dy, dl;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	bool sameItem = false;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

	for(int i=0;i<MAX_ITEMS;i++)	
	{
		OBJECT *_item = &Items[i].Object;	
		if(_item->Live == false || _item->Visible == false)
		{
			continue;
		}

		dx = obj->Owner->Position[0] - _item->Position[0]; // 자기와의 거리를 계산한다.
		dy = obj->Owner->Position[1] - _item->Position[1];

		dl = sqrtf(dx*dx+dy*dy);
		
		//범위 지정
		if( SEARCH_LENGTH > dl )
		{
			//젠만 먹도록 할려면 여기서 걸러버리자...
			//다른 아이템까지 먹도록할려면 여기서 list에 담아버리자...
			//if( -1 == g_pMyInventory->FindEmptySlot(&Items[i].Item) && Items[i].Item.Type != ITEM_POTION+15 )
			if( Items[i].Item.Type != ITEM_POTION+15 ) //젠만 먹자
			{
				continue;
			}

			if(!m_isRooting) //그냥 한번더 검사....
			{
				m_isRooting = true;
				m_RootItem.itemIndex = i;
 				VectorCopy ( _item->Position, m_RootItem.position );
			}

		}
	}
}

bool PetActionCollecterAdd::CompTimeControl(const DWORD& dwCompTime, DWORD& dwTime)
{
	if( (timeGetTime()-dwTime) > dwCompTime )
	{
		dwTime = timeGetTime();
		return true;
	}
	return false;
}

#endif //PJH_ADD_PANDA_PET

#ifdef YDG_ADD_SKELETON_PET
PetActionCollecterSkeleton::PetActionCollecterSkeleton()
{
	m_isRooting = false;

	m_dwSendDelayTime = 0;
	m_dwRootingTime = 0;
	m_dwRoundCountDelay = 0;
#ifdef _VS2008PORTING
	m_state = eAction_Stand;
#else // _VS2008PORTING
	m_state = ActionState::eAction_Stand;
#endif // _VS2008PORTING

	m_fRadWidthStand = 0.0f;
	m_fRadWidthGet = 0.0f;

	m_bIsMoving = FALSE;
}

PetActionCollecterSkeleton::~PetActionCollecterSkeleton()
{
}

PetActionCollecterSkeletonPtr PetActionCollecterSkeleton::Make()
{
	PetActionCollecterSkeletonPtr temp( new PetActionCollecterSkeleton );
	return temp;
}

bool PetActionCollecterSkeleton::Release( OBJECT* obj, CHARACTER *Owner )
{
	return TRUE;
}

bool PetActionCollecterSkeleton::Move( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender )
{
	if( NULL == obj || NULL == Owner ) return FALSE;

	FindZen(obj);

	if( eAction_Stand == m_state && m_isRooting )
	{
		m_state = eAction_Move;
	}

	//------------------------------------------//
	float FlyRange = 12.0f;
	vec3_t targetPos, Range, Direction;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	bool _isMove = false;
	float fRadHeight = ((2*3.14f)/15000.0f) * (float)(tick%15000);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	m_fRadWidthStand = ((2*3.14f)/4000.0f) * (float)(tick%4000);
	m_fRadWidthGet = ((2*3.14f)/2000.0f) * (float)(tick%2000);
	
	obj->Position[2] = obj->Owner->Position[2] + (50.0f * obj->Owner->Scale);

	//------------------------------------------//
	VectorSubtract( obj->Position, obj->Owner->Position, Range );

	float Distance = sqrtf( Range[0]*Range[0] + Range[1]*Range[1] );
	if( Distance > SEARCH_LENGTH*3)
	{
		//맵 이동시 ...
		obj->Position[0] = obj->Owner->Position[0] + (sinf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN);
		obj->Position[1] = obj->Owner->Position[1] + (cosf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN);
		
		VectorCopy ( obj->Owner->Angle, obj->Angle );

		m_state = eAction_Stand;
		m_isRooting = false;
	}
				
	switch(m_state)
	{
	case eAction_Stand:
		{
			targetPos[0] = obj->Owner->Position[0];
			targetPos[1] = obj->Owner->Position[1];
			targetPos[2] = obj->Owner->Position[2];
			
			VectorSubtract( targetPos, obj->Position, Range );
			//------------------------------//
			Distance = sqrtf( Range[0]*Range[0] + Range[1]*Range[1] );
			
			if(80.0f >= FlyRange)
			{
				float Angle = CreateAngle( obj->Position[0],obj->Position[1], targetPos[0],targetPos[1] ); //test
				obj->Angle[2] = TurnAngle2( obj->Angle[2],Angle, 8.0f );
			}
			
			AngleMatrix(obj->Angle,obj->Matrix);
			VectorRotate(obj->Direction,obj->Matrix,Direction);
			VectorAdd(obj->Position,Direction,obj->Position);
			
		//	float Speed = ( FlyRange >= Distance ) ?  0 : (float)log(Distance) * 2.3f;
			float Speed = ( FlyRange*FlyRange >= Distance ) ?  0 : (float)log(Distance) * 2.3f;
						
			obj->Direction[0] = 0.0f;
			obj->Direction[1] = -Speed;
			obj->Direction[2] = 0.0f;
		}
		break;

	case eAction_Move:
		{
			if( !m_isRooting )
			{
				m_isRooting = false;
				m_state = eAction_Return;
				break;
			}

			targetPos[0] = m_RootItem.position[0] + (sinf(m_fRadWidthGet) * CIRCLE_STAND_RADIAN);
			targetPos[1] = m_RootItem.position[1] + (cosf(m_fRadWidthGet) * CIRCLE_STAND_RADIAN);
			targetPos[2] = m_RootItem.position[2];// + 70 + (sinf(fRadHeight) * 70.0f);
			
			VectorSubtract( targetPos, obj->Position, Range );
			//------------------------------//

			Distance = sqrtf( Range[0]*Range[0] + Range[1]*Range[1] );
			if(Distance >= FlyRange)
			{
				float Angle = CreateAngle( obj->Position[0],obj->Position[1], targetPos[0],targetPos[1] ); //test
				obj->Angle[2] = TurnAngle2( obj->Angle[2],Angle, 20.0f );
			}
			
			AngleMatrix(obj->Angle,obj->Matrix);
			VectorRotate(obj->Direction,obj->Matrix,Direction);
			VectorAdd(obj->Position,Direction,obj->Position);
			
			float Speed = ( 20.0f >= Distance ) ?  0 : (float)log(Distance) * 2.5f;
			
			obj->Direction[0] = 0.0f;
			obj->Direction[1] = -Speed;
			obj->Direction[2] = 0.0f;
			
			if(	0 == Speed || CompTimeControl(100000, m_dwRootingTime) )
			{
				m_dwSendDelayTime = GetTickCount();
				m_dwRootingTime = GetTickCount();
				m_state = eAction_Get;
			}
		}
		break;

	case eAction_Get: //줍는 행동
		{
			if(	!m_isRooting || SEARCH_LENGTH < Distance || CompTimeControl(3000, m_dwRootingTime))
			{
				m_isRooting = false;
				m_dwRootingTime = GetTickCount();
				m_state = eAction_Return;
				break;
			}

			//------------------------------//
			VectorCopy ( m_RootItem.position, targetPos );

			float Angle = CreateAngle(obj->Position[0], obj->Position[1], targetPos[0], targetPos[1] );
			obj->Angle[2] = TurnAngle2(obj->Angle[2], Angle, 10.0f);
			//------------------------------//

			if(CompTimeControl(1000, m_dwSendDelayTime))
			{
#ifdef LJH_FIX_GETTING_ZEN_WITH_PET_OF_OTHER_PLAYER
				if(&Hero->Object == obj->Owner)
#endif //LJH_FIX_GETTING_ZEN_WITH_PET_OF_OTHER_PLAYER
					//1000ms 마다 패킷 보냄
					SendRequestGetItem(m_RootItem.itemIndex);
			}
		}
		break;

	case eAction_Return:
		{
			targetPos[0] = obj->Owner->Position[0] + (sinf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN);
			targetPos[1] = obj->Owner->Position[1] + (cosf(m_fRadWidthStand) * CIRCLE_STAND_RADIAN);
			targetPos[2] = obj->Owner->Position[2];// + 70 + (sinf(fRadHeight) * 70.0f);

			VectorSubtract( targetPos, obj->Position, Range );
			//------------------------------//

			Distance = sqrtf( Range[0]*Range[0] + Range[1]*Range[1] );
			if(Distance >= FlyRange)
			{
				float Angle = CreateAngle( obj->Position[0],obj->Position[1], targetPos[0],targetPos[1] ); //test
				obj->Angle[2] = TurnAngle2( obj->Angle[2], Angle, 20.0f );
			}
			
			AngleMatrix( obj->Angle, obj->Matrix );
			VectorRotate( obj->Direction, obj->Matrix, Direction );
			VectorAdd( obj->Position, Direction, obj->Position );
			
			float Speed = ( FlyRange >= Distance ) ?  0 : (float)log(Distance) * 2.5f;
			
			obj->Direction[0] = 0.0f;
			obj->Direction[1] = -Speed;
			obj->Direction[2] = 0.0f;

			if(	0 == Speed || CompTimeControl(3000, m_dwRootingTime) )
			{
				m_state = eAction_Stand;
			}
		}
		break;
	}

	return TRUE;
}

bool PetActionCollecterSkeleton::Effect( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender )
{
	if( NULL == obj || NULL == Owner ) return FALSE;

	BMD* b = &Models[obj->Type];
	vec3_t vPosition, vLight;

	b->BodyScale = obj->Scale;
	b->Animation(BoneTransform,obj->AnimationFrame,obj->PriorAnimationFrame,obj->PriorAction, obj->Angle, obj->HeadAngle, false, false);

	float fLumi = (sinf(WorldTime*0.003f)+1.0f)*0.5f+0.5f;
	Vector( 0.0f*fLumi, 1.0f*fLumi, 0.5f*fLumi, vLight);

	b->TransformByBoneMatrix(vPosition, BoneTransform[12], obj->Position);
	CreateSprite(BITMAP_LIGHTNING+1, vPosition, 0.05f, vLight, obj);
	b->TransformByBoneMatrix(vPosition, BoneTransform[11], obj->Position);
	CreateSprite(BITMAP_LIGHTNING+1, vPosition, 0.05f, vLight, obj);

	b->TransformByBoneMatrix(vPosition, BoneTransform[44], obj->Position);
	CreateSprite(BITMAP_LIGHT, vPosition, 0.3f, vLight, obj);
	b->TransformByBoneMatrix(vPosition, BoneTransform[46], obj->Position);
	CreateSprite(BITMAP_LIGHT, vPosition, 0.6f, vLight, obj);
	b->TransformByBoneMatrix(vPosition, BoneTransform[45], obj->Position);
	CreateSprite(BITMAP_LIGHT, vPosition, 0.4f, vLight, obj);
	b->TransformByBoneMatrix(vPosition, BoneTransform[62], obj->Position);
	CreateSprite(BITMAP_LIGHT, vPosition, 0.3f, vLight, obj);

	// 불뿜기
	m_bIsMoving = !(
		(Owner->Object.CurrentAction >= PLAYER_STOP_MALE && Owner->Object.CurrentAction <= PLAYER_STOP_RIDE_WEAPON)
		|| Owner->Object.CurrentAction == PLAYER_STOP_RIDE_HORSE
		|| Owner->Object.CurrentAction == PLAYER_STOP_TWO_HAND_SWORD_TWO
		|| Owner->Object.CurrentAction == PLAYER_DARKLORD_STAND
		|| (Owner->Object.CurrentAction >= PLAYER_FENRIR_DAMAGE && Owner->Object.CurrentAction <= PLAYER_FENRIR_DAMAGE_ONE_LEFT)
		|| (Owner->Object.CurrentAction >= PLAYER_FENRIR_STAND && Owner->Object.CurrentAction <= PLAYER_FENRIR_STAND_ONE_LEFT)
		|| (Owner->Object.CurrentAction >= PLAYER_DEFENSE1 && Owner->Object.CurrentAction <= PLAYER_CHANGE_UP)
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
		|| (Owner->Object.CurrentAction >= PLAYER_RAGE_FENRIR_STAND && Owner->Object.CurrentAction <= PLAYER_RAGE_FENRIR_STAND_ONE_LEFT)
		|| (Owner->Object.CurrentAction >= PLAYER_RAGE_FENRIR_DAMAGE && Owner->Object.CurrentAction <= PLAYER_RAGE_FENRIR_DAMAGE_ONE_LEFT)
		|| Owner->Object.CurrentAction == PLAYER_RAGE_UNI_STOP_ONE_RIGHT
		|| Owner->Object.CurrentAction == PLAYER_STOP_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK_ANI
		);
	
	if (m_bIsMoving == TRUE)
	{
		Vector( 0.0f, 1.0f, 0.5f, vLight);
		b->TransformByBoneMatrix(vPosition, BoneTransform[13], obj->Position);

		vec3_t vAngle;
		VectorCopy(obj->Angle, vAngle);
		vAngle[0] += 35.0f;		// 바닥을 향해 발사

		for (int i = 0; i < 2; ++i)
		{
			if (i == 1 && rand()%2 == 0) continue;

			switch(rand()%3)
			{
			case 0:
				CreateParticle(BITMAP_FIRE_HIK1_MONO,vPosition,vAngle,vLight,4,obj->Scale,obj);	// 불
				break;
			case 1:
				CreateParticle(BITMAP_FIRE_HIK2_MONO,vPosition,vAngle,vLight,8,obj->Scale,obj);	// 불
				break;
			case 2:
				CreateParticle(BITMAP_FIRE_HIK3_MONO,vPosition,vAngle,vLight,5,obj->Scale,obj);	// 불
				break;
			}
		}

		Vector( 1.0f, 1.0f, 1.0f, vLight);
		CreateSprite(BITMAP_HOLE, vPosition, (sinf(WorldTime*0.005f)+1.0f)*0.1f+0.1f, vLight, obj);
	}

	return TRUE;
}
#endif	// YDG_ADD_SKELETON_PET
