// w_PetActionDemon.h: interface for the PetActionDemon class.
// LDK_2008/07/08
//////////////////////////////////////////////////////////////////////

#ifdef LDK_ADD_NEW_PETPROCESS

#include "w_PetAction.h"
#include "ZzzBMD.h"

#if !defined(AFX_W_PetActionDemon_H__C962E1B9_079B_4B48_AC98_62CCFBF3CBDB__INCLUDED_)
#define AFX_W_PetActionDemon_H__C962E1B9_079B_4B48_AC98_62CCFBF3CBDB__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

BoostSmartPointer(PetActionDemon);
class PetActionDemon : public PetAction
{
public:
	static PetActionDemonPtr Make();
	virtual ~PetActionDemon();
	virtual bool Release( OBJECT* obj, CHARACTER *Owner );

public:
	virtual bool Model( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender );
	virtual bool Move( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender );
	virtual bool Effect( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender );
	
private:
	PetActionDemon();
};

#endif // !defined(AFX_W_PetActionDemon_H__C962E1B9_079B_4B48_AC98_62CCFBF3CBDB__INCLUDED_)
#endif //LDK_ADD_NEW_PETPROCESS
 