//////////////////////////////////////////////////////////////////////////
//  
//  이  름 : CSPetSystem.
//
//  내  용 : 캐릭터가 소환, 한정적 사용이 아닌 팻들의 시스템.
//           다크로드 ( 다크 스피릿 ),
//
//  날  짜 : 2004.03.29.
//
//  작성자 : 조 규 하.
//  
//////////////////////////////////////////////////////////////////////////
#ifndef __CSPET_SYSTEM_H__
#define __CSPET_SYSTEM_H__

//////////////////////////////////////////////////////////////////////////
//  INCLUDE.
//////////////////////////////////////////////////////////////////////////
#ifdef PET_SYSTEM
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"

//////////////////////////////////////////////////////////////////////////
//  CLASS.
//////////////////////////////////////////////////////////////////////////
//  기본적인 팻시스템.
// Hero가 장착하고있는 펫만 해당.
// 인벤토리안에 있는 Pet(Item)은 해당하지 않는다.
class CSPetSystem
{
protected:
    CHARACTER*  m_PetOwner;     //  팻 주인.
    CHARACTER*  m_PetTarget;    //  팻 공격 목표.
    CHARACTER   m_PetCharacter; //  팻 모습.
    PET_TYPE    m_PetType;      //  팻 종류.
#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
    PET_INFO*   m_pPetInfo;      //  팻 정보
#else // KJH_FIX_DARKLOAD_PET_SYSTEM
    PET_INFO    m_PetInfo;      //  팻 정보.
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM
    BYTE        m_byCommand;    //  명령상태.


public :
    CSPetSystem () {};
#ifdef YDG_FIX_MEMORY_LEAK_0905
    virtual ~CSPetSystem ();
#else	// YDG_FIX_MEMORY_LEAK_0905
    virtual ~CSPetSystem () {};
#endif	// YDG_FIX_MEMORY_LEAK_0905

    PET_TYPE    GetPetType ( void ) { return m_PetType; }
#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
	void		SetPetInfo(PET_INFO* pPetInfo) {m_pPetInfo = pPetInfo;};
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM

    virtual void    MovePet ( void ) = 0;
#ifdef KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER
	virtual void	CalcPetInformation ( const PET_INFO& Petinfo ) = 0;
#else // KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER
    virtual void	CalcPetInformation ( BYTE Level, int exp ) = 0;
#endif // KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER
    virtual void    RenderPetInventory ( void ) = 0;
    virtual void    RenderPet ( int PetState = 0 ) = 0;

    virtual void    Eff_LevelUp ( void ) = 0;
    virtual void    Eff_LevelDown ( void ) = 0;

    void    CreatePetPointer ( int Type, unsigned char PositionX, unsigned char PositionY, float Rotation );
    bool    PlayAnimation ( OBJECT* o );

    void    MoveInventory ( void );
    void    RenderInventory ( void );

    void    SetAI ( int AI );
    void    SetCommand ( int Key, BYTE cmd );
    void    SetAttack ( int Key, int attackType );

#ifdef LDS_FIX_AFTER_PETDESTRUCTOR_ATTHESAMETIME_TERMINATE_EFFECTOWNER
	int		GetObjectType()
	{
		return m_PetCharacter.Object.Type;
	}
#endif // LDS_FIX_AFTER_PETDESTRUCTOR_ATTHESAMETIME_TERMINATE_EFFECTOWNER
};


//  다크 스피릿.
class CSPetDarkSpirit : public CSPetSystem
{
private :

public :
    CSPetDarkSpirit ( CHARACTER* c );
#ifdef YDG_FIX_MEMORY_LEAK_0905_2ND
    virtual ~CSPetDarkSpirit ( void );
#else	// YDG_FIX_MEMORY_LEAK_0905_2ND
    virtual ~CSPetDarkSpirit ( void ) {};
#endif	// YDG_FIX_MEMORY_LEAK_0905_2ND

    virtual void MovePet ( void );
#ifdef KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER
	virtual void CalcPetInformation ( const PET_INFO& Petinfo );
#else // KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER
    virtual void CalcPetInformation ( BYTE Level, int exp );
#endif // KJH_FIX_WOPS_K19787_PET_LIFE_ABNORMAL_RENDER
    virtual void RenderPetInventory ( void );
    virtual void RenderPet ( int PetState = 0 );

    virtual void Eff_LevelUp ( void );
    virtual void Eff_LevelDown ( void );

    void    AttackEffect ( CHARACTER* c, OBJECT* o );
    void    RenderCmdType ( void );
};

#endif	// PET_SYSTEM

#endif// __CSPET_SYSTEM_H__