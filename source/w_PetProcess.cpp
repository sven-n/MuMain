// w_PetProcess.cpp: implementation of the PetProcess class.
// LDK_2008/07/08
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef LDK_ADD_NEW_PETPROCESS

#include "w_PetActionStand.h"
#include "w_PetActionRound.h"
#include "w_PetActionDemon.h"

#include "w_PetActionCollecter.h"
#include "w_PetActionCollecter_Add.h"
#include "w_PetActionUnicorn.h"

#include "w_PetProcess.h"
#include "ReadScript.h"
#include "./Utilities/Log/ErrorReport.h"

/////////////////////////////////////////////////////////////////////
static BYTE bBuxCode[3] = {0xfc,0xcf,0xab};
static void BuxConvert(BYTE *Buffer,int Size)
{
	for(int i=0;i<Size;i++)
		Buffer[i] ^= bBuxCode[i%3];
}
//////////////////////////////////////////////////////////////////////
// Infomation class
//////////////////////////////////////////////////////////////////////
PetInfoPtr PetInfo::Make()
{
	PetInfoPtr petInfo( new PetInfo );
	return petInfo;
}

PetInfo::PetInfo():
m_scale(0.0f),
m_actions(NULL),
m_speeds(NULL),
m_count(0)
{
}

PetInfo::~PetInfo()
{
	Destroy();
}

void PetInfo::Destroy()
{
	if(NULL != m_actions)
	{
		delete [] m_actions;
	}
	m_actions = NULL;

	if(NULL != m_speeds)
	{
		delete [] m_speeds;
	}
	m_speeds = NULL;
}

void PetInfo::SetActions(int count, int *actions, float *speeds) 
{
	if(NULL == actions || 0 >= count || NULL == speeds) return;

	m_count = count;
	m_actions = new int[count]();
	memcpy(m_actions, actions, sizeof(int)*m_count);

	m_speeds = new float[count];
	memcpy(m_speeds, speeds, sizeof(int)*m_count);
}

//////////////////////////////////////////////////////////////////////
// extern
//////////////////////////////////////////////////////////////////////
PetProcessPtr g_petProcess;

PetProcess& ThePetProcess()
{
	assert( g_petProcess );
	return *g_petProcess;
}
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
PetProcessPtr PetProcess::Make()
{
	PetProcessPtr petprocess( new PetProcess );
	petprocess->Init();
	return petprocess;
}

PetProcess::PetProcess()
{
}

PetProcess::~PetProcess()
{
	Destroy();
}

//////////////////////////////////////////////////////////////////////
// 사용할 ActionExpress를 ActionMap에 등록
//  - ActionExpress class를 생성시에만 등록하자
//	- 기존 ActionExpress를 사용시에는 등록 안함
//	- 등록시 인자는 스크립트의actNum값, 클래스포인터 입니다.
//////////////////////////////////////////////////////////////////////
void PetProcess::Init()
{
	PetActionStandPtr actionStand = PetActionStand::Make();
	m_petsAction.insert( make_pair( PC4_ELF, actionStand ) ); //정령
	//m_petsAction.insert( make_pair( 4, actionStand ) ); //정령

	PetActionRoundPtr actionRound = PetActionRound::Make();
	m_petsAction.insert( make_pair( PC4_TEST, actionRound ) );

	PetActionDemonPtr actionTest = PetActionDemon::Make();
	m_petsAction.insert( make_pair( PC4_SATAN, actionTest ) ); //사탄

	PetActionCollecterPtr actionCollecter = PetActionCollecter::Make();
	m_petsAction.insert( make_pair( XMAS_RUDOLPH, actionCollecter ) ); //아이템 수집자?????(가칭 : 루돌프 펫 ㅋㅋ)

#ifdef PJH_ADD_PANDA_PET
	PetActionCollecterAddPtr actionCollecter_Add = PetActionCollecterAdd::Make();
	m_petsAction.insert( make_pair( PANDA, actionCollecter_Add ) ); //아이템 수집자?????(가칭 : 루돌프 펫 ㅋㅋ)
#endif //#ifdef PJH_ADD_PANDA_PET

#ifdef LDK_ADD_CS7_UNICORN_PET
 	PetActionUnicornPtr actionUnicorn = PetActionUnicorn::Make();
 	m_petsAction.insert( make_pair( UNICORN, actionUnicorn ) ); //유니콘 펫
#endif //LDK_ADD_CS7_UNICORN_PET
	
#ifdef YDG_ADD_SKELETON_PET
	PetActionCollecterSkeletonPtr actionCollecter_Skeleton = PetActionCollecterSkeleton::Make();
	m_petsAction.insert( make_pair( SKELETON, actionCollecter_Skeleton ) ); // 스켈레톤 펫
#endif	// YDG_ADD_SKELETON_PET

	LoadData();
}

void PetProcess::Destroy()
{
	for( PetList::iterator iter = m_petsList.begin(); iter != m_petsList.end(); )
	{
		PetList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(PetObject) basepet = *tempiter;
		
		if( basepet.expired() == FALSE )
		{
			basepet.lock()->Release();
			m_petsList.erase( tempiter );
		}
	}
	m_petsList.clear();

 	m_petsAction.clear();
}

//////////////////////////////////////////////////////////////////////
// ActionMap에서 key값으로 검색 weakPoint로 넘겨줌
//////////////////////////////////////////////////////////////////////
BoostWeak_Ptr(PetAction) PetProcess::Find( int key )
{
	ActionMap::iterator iter = m_petsAction.find( key );

	if( iter != m_petsAction.end() )
	{
		return (*iter).second;
	}

	BoostWeak_Ptr(PetAction) temp;
	
	return temp;
}

#ifdef LDK_MOD_NUMBERING_PETCREATE
BoostWeak_Ptr(PetObject) PetProcess::FindList( int key, PetList::iterator out_iter )
{
	for( PetList::iterator iter = m_petsList.begin(); iter != m_petsList.end(); )
	{
		PetList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(PetObject) basepet = *tempiter;
		
		if( basepet.expired() == FALSE && basepet.lock()->IsSameRegKey(key) ) 
		{
			if(out_iter != NULL)
			{
				out_iter = tempiter;
			}
			
			return basepet;
		}
	}
	
	BoostWeak_Ptr(PetObject) temp;
	
	return temp;
}

//////////////////////////////////////////////////////////////////////
// 생성된 PetObject를 map에 등록 : 정해진 넘버로 등록( 무기등의 정해진 번호 사용시)
//////////////////////////////////////////////////////////////////////
int PetProcess::Register( BoostSmart_Ptr( PetObject ) pPet, int petNum )
{
	if( 0 >= petNum || FindList(petNum).expired() == FALSE ) return FALSE;

	int temp = petNum + GetTickCount();
	pPet->SetRegKey(temp);

	m_petsList.push_back( pPet );

	return temp;
}
//////////////////////////////////////////////////////////////////////
// map에 등록된 PetObject를 지움 : 같은 Owner가 가진 펫중 등록번호 확인후 삭제
//////////////////////////////////////////////////////////////////////
bool PetProcess::UnRegister( CHARACTER *Owner, int petRegNum )
{
	if( NULL == Owner || -1 > petRegNum || 0 == m_petsList.size() ) return FALSE;

	for( PetList::iterator iter = m_petsList.begin(); iter != m_petsList.end(); )
	{
		PetList::iterator tempiter = iter;
		++iter;
		BoostWeak_Ptr(PetObject) basepet = *tempiter;
		
		if( basepet.expired() == FALSE )
		{
			 if( -1 == petRegNum )
			{
				//전체삭제
				basepet.lock()->Release();
				m_petsList.erase( tempiter );
			}
			else if( basepet.lock()->IsSameOwner(&Owner->Object) && basepet.lock()->IsSameRegKey(petRegNum) )
			{
				//개별삭제
				basepet.lock()->Release();
				m_petsList.erase( tempiter );

				return TRUE;
			}
		}
	}

	if( -1 == petRegNum ) return TRUE;

	return FALSE;
}
#else //LDK_MOD_NUMBERING_PETCREATE
//////////////////////////////////////////////////////////////////////
// 생성된 PetObject를 list에 등록 : 등록 조건 강화 필요할듯..
//////////////////////////////////////////////////////////////////////
void PetProcess::Register( BoostSmart_Ptr( PetObject ) pPet )
{
	m_petsList.push_back( pPet );
}
//////////////////////////////////////////////////////////////////////
// list에 등록된 PetObject를 지움 : 같은 Owner가 가진 여러마리의 펫에대한 처리 필요..
//////////////////////////////////////////////////////////////////////
void PetProcess::UnRegister(CHARACTER *Owner, int itemType, bool isUnregistAll)
{
	if( NULL == Owner ) return;

 	for( PetList::iterator iter = m_petsList.begin(); iter != m_petsList.end(); )
 	{
 		PetList::iterator tempiter = iter;
 		++iter;
 		BoostWeak_Ptr(PetObject) basepet = *tempiter;
 		
 		if( basepet.expired() == FALSE ) 
		{
			if( (-1 == itemType && basepet.lock()->IsSameOwner(&Owner->Object)) 
				|| basepet.lock()->IsSameObject(&Owner->Object, itemType) )
			{
				basepet.lock()->Release();
 				m_petsList.erase( tempiter );

 				if( -1 == itemType || !isUnregistAll ) return;
 			}
 		}
 	}
}
#endif //LDK_MOD_NUMBERING_PETCREATE

//--------------------------------------------//
// 외부 지원함수들
//--------------------------------------------//

//////////////////////////////////////////////////////////////////////
// 스크립트 load 함수 : load후 map에 등록
//////////////////////////////////////////////////////////////////////
bool PetProcess::LoadData()
{
	char FileName[100];
#ifdef USE_PET_TEST_BMD
	sprintf(FileName, "Data\\Local\\pettest.bmd");
#else	// USE_PET_TEST_BMD
	sprintf(FileName, "Data\\Local\\pet.bmd");
#endif	// USE_PET_TEST_BMD

	int _ver;
	int _array;

	FILE *fp = fopen(FileName, "rb");
	if(fp == NULL)
	{
		char Text[256];
    	sprintf(Text,"%s - File not exist.",FileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);

		return FALSE;
	}

	fread( &_ver, sizeof(int), 1, fp );
	fread( &_array, sizeof(int), 1, fp );

	int _type;
	int _blendMesh;
	float _scale;
	int _count;
	int *_action = new int[_array];
	float *_speed = new float[_array];

	int _listSize = 0;
	fread( &_listSize, sizeof(DWORD), 1, fp );

	int Size = sizeof(int) + sizeof(int) + sizeof(float) + sizeof(int) + ((sizeof(int) + sizeof(float)) * _array);
	BYTE *Buffer = new BYTE [Size*_listSize];

	fread( Buffer,Size*_listSize,1,fp );

	DWORD dwCheckSum;
	fread(&dwCheckSum,sizeof ( DWORD),1,fp);

	if ( dwCheckSum != GenerateCheckSum2( Buffer, Size*_listSize, 0x7F1D))
	{
		char Text[256];
    	sprintf(Text,"%s - File corrupted.",FileName);
		g_ErrorReport.Write( Text);
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		
		return FALSE;
	}
	else
	{
		BYTE *pSeek = Buffer;
		for(int i=0;i<_listSize;i++)
		{
			_type = 0;
			_scale = 0.0f;
			_blendMesh = -1;
			_count = 0;
			ZeroMemory( _action, sizeof(_action) );
			ZeroMemory( _speed, sizeof(_speed) );

			BuxConvert(pSeek,Size);

			memcpy(&_type, pSeek, sizeof(_type));
			pSeek += sizeof(_type);

			memcpy(&_blendMesh, pSeek, sizeof(_blendMesh));
			pSeek += sizeof(_blendMesh);

			memcpy(&_scale, pSeek, sizeof(_scale));
			pSeek += sizeof(_scale);
			
			memcpy(&_count, pSeek, sizeof(_count));
			pSeek += sizeof(_count);
			
			memcpy(_action, pSeek, sizeof(int)*_array);
			pSeek += sizeof(int)*_array;
			
			memcpy(_speed, pSeek, sizeof(_speed)*_array);
			pSeek += sizeof(_speed)*_array;

			//입력
			PetInfoPtr petInfo = PetInfo::Make();
			petInfo->SetBlendMesh( _blendMesh );
			petInfo->SetScale( _scale );
			petInfo->SetActions( _count, _action, _speed );

			m_petsInfo.insert( make_pair(ITEM_HELPER+_type, petInfo) );
		}
	}
	delete [] _action;
	delete [] _speed;
	delete [] Buffer;

	return TRUE;
}

//////////////////////////////////////////////////////////////////////
// map에 등록 된것 인지 확인한다.
//////////////////////////////////////////////////////////////////////
bool PetProcess::IsPet(int itemType)
{
	//등록되어있는  item_number면 true
	InfoMap::iterator iter = m_petsInfo.find(itemType);
	if( iter == m_petsInfo.end() ) return FALSE;
  	
	BoostWeak_Ptr(PetInfo) petInfo = (*iter).second;
	if( petInfo.expired() ) return FALSE;

	return TRUE;
}

#ifdef LDK_MOD_NUMBERING_PETCREATE
//////////////////////////////////////////////////////////////////////
// 펫 생성후 등록
//////////////////////////////////////////////////////////////////////
int PetProcess::CreatePet( int itemType, int modelType, vec3_t Position, CHARACTER *Owner, int SubType, int LinkBone )
{
	if ( NULL == Owner ) return 0;
	
	PetObjectPtr _tempPet = PetObject::Make();
	if( _tempPet->Create( itemType, modelType, Position, Owner, SubType, LinkBone ) )
	{
		//actions및 초기값 설정.-----------------------------//
		InfoMap::iterator iter = m_petsInfo.find(itemType);
		if( iter == m_petsInfo.end() ) return 0;
		
		BoostWeak_Ptr(PetInfo) petInfo = (*iter).second;
		
		int _count = 0;
		int *action = NULL;
		float *speed = NULL;
		if( petInfo.expired() == FALSE ) 
		{
			_count = petInfo.lock()->GetActionsCount();
			action = petInfo.lock()->GetActions();
			speed =  petInfo.lock()->GetSpeeds();
			
			_tempPet->SetScale( petInfo.lock()->GetScale() );
			_tempPet->SetBlendMesh( petInfo.lock()->GetBlendMesh() );
		}
		
		for(int i=0; i<_count; i++)
		{
			_tempPet->SetActions( (PetObject::ActionType)i, Find(action[i]), speed[i] );
		}
		//---------------------------------------------------//
		
		return Register(_tempPet, itemType);
	}
	
	return 0;
}//////////////////////////////////////////////////////////////////////
// 등록되어 있는 펫 삭제 : 외부 지원 함수
//////////////////////////////////////////////////////////////////////
bool PetProcess::DeletePet( CHARACTER *Owner, int petRegNum, bool ex )
{
	if( NULL == Owner ) return FALSE;
	
	if( TRUE == ex )
	{
		petRegNum = -1;
	}
	
	return UnRegister( Owner, petRegNum );
}
#else //LDK_MOD_NUMBERING_PETCREATE
//////////////////////////////////////////////////////////////////////
// 펫 생성후 등록
//////////////////////////////////////////////////////////////////////
bool PetProcess::CreatePet( int itemType, int modelType, vec3_t Position, CHARACTER *Owner, int SubType, int LinkBone )
{
	if ( NULL == Owner ) return FALSE;
#ifndef LDK_FIX_HIDE_PET_TO_NOT_MODEL_PLAYER //not defined 
	if ( Owner->Object.Type != MODEL_PLAYER ) return FALSE;
#endif LDK_FIX_HIDE_PET_TO_NOT_MODEL_PLAYER

	PetObjectPtr _tempPet = PetObject::Make();
	if( _tempPet->Create( itemType, modelType, Position, Owner, SubType, LinkBone ) )
	{
		//actions및 초기값 설정.-----------------------------//
		InfoMap::iterator iter = m_petsInfo.find(itemType);
		if( iter == m_petsInfo.end() ) return FALSE;

  		BoostWeak_Ptr(PetInfo) petInfo = (*iter).second;
		
		int _count = 0;
		int *action = NULL;
		float *speed = NULL;
		if( petInfo.expired() == FALSE ) 
		{
			_count = petInfo.lock()->GetActionsCount();
 			action = petInfo.lock()->GetActions();
			speed =  petInfo.lock()->GetSpeeds();

			_tempPet->SetScale( petInfo.lock()->GetScale() );
			_tempPet->SetBlendMesh( petInfo.lock()->GetBlendMesh() );
 		}

		for(int i=0; i<_count; i++)
		{
			_tempPet->SetActions( (PetObject::ActionType)i, Find(action[i]), speed[i] );
		}
		//---------------------------------------------------//

		//list에 등록
		Register( _tempPet );
     	return TRUE;
	}

	return FALSE;
}
//////////////////////////////////////////////////////////////////////
// 등록되어 있는 펫 삭제 : 외부 지원 함수
//////////////////////////////////////////////////////////////////////
void PetProcess::DeletePet(CHARACTER *Owner, int itemType, bool allDelete)
{
	if( NULL == Owner ) return;

	UnRegister( Owner, itemType, allDelete );
}
#endif //LDK_MOD_NUMBERING_PETCREATE

#ifdef LDK_MOD_NUMBERING_PETCREATE
//////////////////////////////////////////////////////////////////////
// 펫 행동 변경 함수 : 넘버링된 펫 행동 변경
//////////////////////////////////////////////////////////////////////
bool PetProcess::SetCommandPet( CHARACTER *Owner, int petRegNum, int targetKey, PetObject::ActionType cmdType )
{
	if( NULL == Owner || -1 > petRegNum ) return FALSE;

	//m_petsMap 등록된 펫 정보 변경
	if( -1 != petRegNum )
	{
		//개별 행동
		BoostWeak_Ptr(PetObject) petReg = FindList(petRegNum);
		if( petReg.expired() == FALSE && petReg.lock()->IsSameOwner(&Owner->Object) ) 
		{
			petReg.lock()->SetCommand( targetKey, cmdType );
			
			return TRUE;
		}
	}
	else
	{
		//전체 행동
		for( PetList::iterator iter = m_petsList.begin(); iter != m_petsList.end(); )
		{
			PetList::iterator tempiter = iter;
			++iter;
			BoostWeak_Ptr(PetObject) basepet = *tempiter;
			
			if( basepet.expired() == FALSE && basepet.lock()->IsSameOwner(&Owner->Object)  )
			{
				basepet.lock()->SetCommand( targetKey, cmdType );
			}
		}
		return TRUE;
	}

	return FALSE;
}
#else //LDK_MOD_NUMBERING_PETCREATE
//////////////////////////////////////////////////////////////////////
// 펫 행동 변경 함수 : 같은 주인의 모든 펫
//////////////////////////////////////////////////////////////////////
void PetProcess::SetCommandPet(CHARACTER *Owner, int targetKey, PetObject::ActionType cmdType )
{
	if( NULL == Owner ) return;

 	for( PetList::iterator iter = m_petsList.begin(); iter != m_petsList.end(); )
 	{
 		PetList::iterator tempiter = iter;
 		++iter;
 		BoostWeak_Ptr(PetObject) basepet = *tempiter;
 		
 		if( basepet.expired() == FALSE ) 
		{
			if( basepet.lock()->IsSameOwner(&Owner->Object) )
			{
				basepet.lock()->SetCommand( targetKey, cmdType );
			}
 		}
 	}
}
#endif //LDK_MOD_NUMBERING_PETCREATE

//////////////////////////////////////////////////////////////////////
// MoveUpdate 함수
//////////////////////////////////////////////////////////////////////
void PetProcess::UpdatePets()
{
 	for( PetList::iterator iter = m_petsList.begin(); iter != m_petsList.end(); )
 	{
 		PetList::iterator tempiter = iter;
 		++iter;
 		BoostWeak_Ptr(PetObject) basepet = *tempiter;
 		
 		if( basepet.expired() == false ) 
		{
 			basepet.lock()->Update();
 		}
 	}
}

//////////////////////////////////////////////////////////////////////
// RenderUpdate 함수
//////////////////////////////////////////////////////////////////////
void PetProcess::RenderPets()
{
	for( PetList::iterator iter = m_petsList.begin(); iter != m_petsList.end(); )
 	{
 		PetList::iterator tempiter = iter;
 		++iter;
 		BoostWeak_Ptr(PetObject) basepet = *tempiter;
 		
 		if( basepet.expired() == FALSE ) 
		{
			basepet.lock()->Render();
 		}
 	}
}

#endif //LDK_ADD_NEW_PETPROCESS