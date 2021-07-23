// w_PetActionRound.h: interface for the PetActionRound class.
//
//////////////////////////////////////////////////////////////////////

#ifdef LDK_ADD_NEW_PETPROCESS

#include "w_PetAction.h"
#include "ZzzBMD.h"

#if !defined(AFX_W_PETACTIONROUND_H__74746333_F761_43C8_BAA7_AC9BB6F2F858__INCLUDED_)
#define AFX_W_PETACTIONROUND_H__74746333_F761_43C8_BAA7_AC9BB6F2F858__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

BoostSmartPointer(PetActionRound);
class PetActionRound : public PetAction
{
public:
	static PetActionRoundPtr Make();
	virtual ~PetActionRound();
	virtual bool Release( OBJECT* obj, CHARACTER *Owner );

public:
	virtual bool Model( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender );
	virtual bool Move( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender );
	virtual bool Effect( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender );
	
private:
	PetActionRound();
};

#endif // !defined(AFX_W_PETACTIONROUND_H__74746333_F761_43C8_BAA7_AC9BB6F2F858__INCLUDED_)
#endif //LDK_ADD_NEW_PETPROCESS
