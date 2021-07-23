// w_BasePet.h: interface for the BasePet class.
// LDK_2008/07/08
//////////////////////////////////////////////////////////////////////

#ifdef LDK_ADD_NEW_PETPROCESS

#if !defined(AFX_W_BASEPET_H__F071AE33_ED95_4AB3_B875_052B02C90553__INCLUDED_)
#define AFX_W_BASEPET_H__F071AE33_ED95_4AB3_B875_052B02C90553__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_PetAction.h"
#include "ZzzBMD.h"

BoostSmartPointer(PetObject);
class PetObject
{
public:
	enum ActionType{
		eAction_Stand		= 0,
#ifdef LDK_MOD_PETPROCESS_SYSTEM
		eAction_Move,
		eAction_Attack,
		eAction_Skill,

		eAction_Birth,
		eAction_Dead,
		eAction_End
#else //LDK_MOD_PETPROCESS_SYSTEM
		eAction_Attack		= 1,
		eAction_Skill		= 2,
		
		eAction_Birth		= 3,
		eAction_Dead		= 4,
#endif //LDK_MOD_PETPROCESS_SYSTEM
	};

public:
	static PetObjectPtr Make();
	virtual ~PetObject();

public:
	DWORD GetNowTick() { return GetTickCount() - m_startTick; }
	OBJECT* GetObject() { return m_obj; }
	bool IsSameOwner( OBJECT *Owner );
	bool IsSameObject( OBJECT *Owner, int itemType );
#ifdef LDK_MOD_NUMBERING_PETCREATE
	//bool IsSameType( int itemType ) { return (m_itemType == itemType ? TRUE : FALSE); }
	bool IsSameRegKey( int regstKey ) { return (m_regstKey == regstKey ? TRUE : FALSE); }

	void SetRegKey( int regstKey ) { m_regstKey = regstKey; }
#endif //LDK_MOD_NUMBERING_PETCREATE
	void SetActions( ActionType type , BoostWeak_Ptr(PetAction) action, float speed );
	void SetCommand( int targetKey, ActionType cmdType );

	void SetScale( float scale = 0.0f );
	void SetBlendMesh( int blendMesh = -1 );

public:
	bool Create( int itemType, int modelType, vec3_t Position, CHARACTER *Owner, int SubType, int LinkBone );
	void Release();
	void Update( bool bForceRender = false );
	void Render( bool bForceRender = false );

private:
	bool UpdateMove( DWORD tick, bool bForceRender = false );
	bool UpdateModel( DWORD tick, bool bForceRender = false );
#ifdef LDK_ADD_NEW_PETPROCESS_ADD_SOUND
	bool UpdateSound( DWORD tick, bool bForceRender = false );
#endif //LDK_ADD_NEW_PETPROCESS_ADD_SOUND
	
	bool CreateEffect( DWORD tick, bool bForceRender = false );

private:
	void Init();
	PetObject();
	
public:
	typedef map< ActionType, BoostWeak_Ptr(PetAction) > ActionMap;
	typedef map< ActionType, float > SpeedMap;

private:
	ActionMap m_actionMap;
	SpeedMap m_speedMap;

	CHARACTER *m_pOwner;
	OBJECT* m_obj;
	int m_targetKey;
	int m_itemType;
	
	DWORD m_startTick;

	ActionType m_moveType; //실행중인 행동
#ifdef LDK_MOD_PETPROCESS_SYSTEM
	ActionType m_oldMoveType;
#endif //LDK_MOD_PETPROCESS_SYSTEM

#ifdef LDK_MOD_NUMBERING_PETCREATE
	int m_regstKey;
#endif //LDK_MOD_NUMBERING_PETCREATE

};

#endif // !defined(AFX_W_BASEPET_H__F071AE33_ED95_4AB3_B875_052B02C90553__INCLUDED_)
#endif //LDK_ADD_NEW_PETPROCESS
