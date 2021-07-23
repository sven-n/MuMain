// w_PetActionUnicorn.h: interface for the w_PetActionUnicorn class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_W_PETACTIONUNICORN_H__3F921335_4F54_4FFA_8808_0869016403DA__INCLUDED_)
#define AFX_W_PETACTIONUNICORN_H__3F921335_4F54_4FFA_8808_0869016403DA__INCLUDED_

#ifdef LDK_ADD_CS7_UNICORN_PET

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_PetAction.h"
#include "ZzzBMD.h"
#include "w_PetActionCollecter.h"

BoostSmartPointer(PetActionUnicorn);
class PetActionUnicorn : public PetAction
{
public:
	static PetActionUnicornPtr Make();
	virtual ~PetActionUnicorn();
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
	PetActionUnicorn();
	
	//ItemList m_ItemList;
	RootingItem m_RootItem;
	bool m_isRooting;
	
	DWORD m_dwSendDelayTime;
	DWORD m_dwRootingTime;
	DWORD m_dwRoundCountDelay;
	ActionState m_state;
	
	float m_fRadWidthStand;
	float m_fRadWidthGet;

	float m_speed;
	
	//test
};

#endif //LDK_ADD_CS7_UNICORN_PET

#endif // !defined(AFX_W_PETACTIONUNICORN_H__3F921335_4F54_4FFA_8808_0869016403DA__INCLUDED_)
