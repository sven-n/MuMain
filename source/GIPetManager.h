//////////////////////////////////////////////////////////////////////////
//  
//  이  름 : GIPetManager.
//
//  내  용 : 팻을 관리.
//
//  날  짜 : 2004.07.01.
//
//  작성자 : 조 규 하.
//  
//////////////////////////////////////////////////////////////////////////
#ifndef __CIPET_MANAGER_H__
#define __CIPET_MANAGER_H__

//////////////////////////////////////////////////////////////////////////
//  INCLUDE.
//////////////////////////////////////////////////////////////////////////
#ifdef PET_SYSTEM
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "CSPetSystem.h"


namespace giPetManager
{
    //////////////////////////////////////////////////////////////////////////
    //  EXTERN.
    //////////////////////////////////////////////////////////////////////////
    
    //////////////////////////////////////////////////////////////////////////
    //  STRUCTURE.
    //////////////////////////////////////////////////////////////////////////
    
    //////////////////////////////////////////////////////////////////////////
    //  VARIABLE
    //////////////////////////////////////////////////////////////////////////

#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
	// 아이템에 마우스On했을때 펫정보
	static PET_INFO	gs_PetInfo;
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM


    //////////////////////////////////////////////////////////////////////////
    //  FUNCTION.
    //////////////////////////////////////////////////////////////////////////	
    void    InitPetManager ( void );
    void    CreatePetDarkSpirit (  CHARACTER*c );
    void    CreatePetDarkSpirit_Now ( CHARACTER* c );

    void    MovePet ( CHARACTER*c );
    void    RenderPet ( CHARACTER* c );
    void    DeletePet ( CHARACTER* c );

	// PetItem 관련
    void    InitItemBackup ( void );
#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
	void SetPetInfo( BYTE InvType, BYTE InvPos, PET_INFO* pPetinfo );				// 펫정보 셋팅
	PET_INFO* GetPetInfo(ITEM* pItem);												// 펫정보 가져오기
	void CalcPetInfo( PET_INFO* pPetInfo );											// 펫정보 세부계산 (경험치, 데미지등)
    void SetPetItemConvert( ITEM* ip, PET_INFO* pPetInfo );							// 펫 아이템 정보(옵션) 설정
	DWORD GetPetItemValue( PET_INFO* pPetInfo );														// 펫 아이템 가격 가져오기
	bool RequestPetInfo( int sx, int sy, ITEM* pItem );								// 펫정보 서버에 요청
	bool RenderPetItemInfo( int sx, int sy, ITEM* pItem, int iInvenType );
#else // KJH_FIX_DARKLOAD_PET_SYSTEM											//## 소스정리 대상임.
#ifdef KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER
	void	SetPetInfo ( BYTE InvType, BYTE Index, const PET_INFO& Petinfo );
#else // KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER
    void    SetPetInfo ( BYTE PetType, BYTE InvType, BYTE Index, BYTE Level, int exp );
#endif // KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER
	void    CalcPetInfo ( BYTE petType );
	void    ItemConvert ( ITEM* ip );
	DWORD   ItemValue ( PET_TYPE pType );
	bool    RenderPetItemInfo ( int sx, int sy, ITEM* ip, bool Sell );
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM											//## 소스정리 대상임.

	// Pet 행동
    bool    SelectPetCommand ( void );
    void    MovePetCommand ( CHARACTER* c );
    bool    SendPetCommand ( CHARACTER* c, int Index );
    void    SetPetCommand ( CHARACTER* c, int Key, BYTE Cmd );
    void    SetAttack ( CHARACTER* c, int Key, int attackType );  
    bool    RenderPetCmdInfo ( int sx, int sy, int Type );
#ifndef KJH_FIX_DARKLOAD_PET_SYSTEM												//## 소스정리 대상임.			
	void	GetPetInfo(PET_INFO &PetInfo, short type);
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM											//## 소스정리 대상임.
}

using namespace giPetManager;

#endif// PET_SYSTEM.

#endif//