// w_PetActionStand.h: interface for the PetActionStand class.
//
//////////////////////////////////////////////////////////////////////

#ifdef LDK_ADD_NEW_PETPROCESS

#include "w_PetAction.h"
#include "ZzzBMD.h"

#if !defined(AFX_W_PETACTIONSTAND_H__5F7FD6F9_0980_402C_9E5A_13928A221608__INCLUDED_)
#define AFX_W_PETACTIONSTAND_H__5F7FD6F9_0980_402C_9E5A_13928A221608__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

BoostSmartPointer(PetActionStand);
class PetActionStand : public PetAction
{
public:
	static PetActionStandPtr Make();
	virtual ~PetActionStand();
	virtual bool Release( OBJECT* obj, CHARACTER *Owner );

public:
	virtual bool Model( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender );
	virtual bool Move( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender );
	virtual bool Effect( OBJECT* obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender );
	
private:
	PetActionStand();
};

#endif // !defined(AFX_W_PETACTIONSTAND_H__5F7FD6F9_0980_402C_9E5A_13928A221608__INCLUDED_)
#endif //LDK_ADD_NEW_PETPROCESS
