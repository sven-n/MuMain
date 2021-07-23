// w_PetActionCollecter.h: interface for the PetActionStand class.
//
//////////////////////////////////////////////////////////////////////

#ifdef LDK_ADD_NEW_PETPROCESS

#include "w_PetAction.h"
#include "ZzzBMD.h"

#if !defined(AFX_W_PETACTIONCOLLECTER_H__5F7FD6F9_0980_402C_9E5A_13928A221608__INCLUDED_)
#define AFX_W_PETACTIONCOLLECTER_H__5F7FD6F9_0980_402C_9E5A_13928A221608__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define SEARCH_LENGTH 300.0f
#define CIRCLE_STAND_RADIAN 50.0f
#define CIRCLE_GETITEM_RADIAN 50.0f

typedef struct _RootingItem
{
	int itemIndex;
	vec3_t position;
}RootingItem;

BoostSmartPointer(PetActionCollecter);
class PetActionCollecter : public PetAction
{
public:
	static PetActionCollecterPtr Make();
	virtual ~PetActionCollecter();
	virtual bool Release( OBJECT* obj, CHARACTER *Owner );

public:
	virtual bool Model( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender );
	virtual bool Move( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender );
	virtual bool Effect( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender );
	virtual bool Sound( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender );
	
	//test
	void FindZen(OBJECT* obj);
	bool CompTimeControl(const DWORD& dwCompTime, DWORD& dwTime);

public:
	typedef list< RootingItem > ItemList;
	enum ActionState
	{
		eAction_Stand		= 0,
		eAction_Move		= 1,
		eAction_Get			= 2,
		eAction_Return		= 3,

		eAction_End_NotUse,
	};

private:
	PetActionCollecter();

	//test

	//ItemList m_ItemList;
	RootingItem m_RootItem;
	bool m_isRooting;

	DWORD m_dwSendDelayTime;
	DWORD m_dwRootingTime;
	DWORD m_dwRoundCountDelay;
	ActionState m_state;

	float m_fRadWidthStand;
	float m_fRadWidthGet;

	//test
};

#endif // !defined(AFX_W_PETACTIONCOLLECTER_H__5F7FD6F9_0980_402C_9E5A_13928A221608__INCLUDED_)
#endif //LDK_ADD_NEW_PETPROCESS
