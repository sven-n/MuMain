// w_PetAction.h: interface for the PetAction class.
// LDK_2008/07/08
//////////////////////////////////////////////////////////////////////

#ifdef LDK_ADD_NEW_PETPROCESS

#include "ZzzBMD.h"

#if !defined(AFX_W_PETACTION_H__6A4B0CEF_7F8A_4B5D_AF6D_B62BEB89009A__INCLUDED_)
#define AFX_W_PETACTION_H__6A4B0CEF_7F8A_4B5D_AF6D_B62BEB89009A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class PetAction  
{
public:
	PetAction() {}
	virtual ~PetAction() {}

	//자체적으로 생성한 내용을 삭제하도록 하자.
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual bool Release( OBJECT* pObj, CHARACTER* pOwner )
			{
				if(pObj == NULL)	return false;
				if(pOwner == NULL)	return false;

				return true;
			}
	virtual bool Model( OBJECT* pObj, CHARACTER* pOwner, int iTargetKey, DWORD dwTick, bool bForceRender )
			{
				if(pObj == NULL)			return false;
				if(pOwner == NULL)			return false;
				if(iTargetKey < 0)			return false;
				if(dwTick == 0)				return false;
				if(bForceRender == false)	return false;

				return true;
			}
	virtual bool Move( OBJECT* pObj, CHARACTER* pOwner, int iTargetKey, DWORD dwTick, bool bForceRender )
			{
				if(pObj == NULL)			return false;
				if(pOwner == NULL)			return false;
				if(iTargetKey < 0)			return false;
				if(dwTick == 0)				return false;
				if(bForceRender == false)	return false;

				return true;
			}
	virtual bool Effect( OBJECT* pObj, CHARACTER* pOwner, int iTargetKey, DWORD dwTick, bool bForceRender )
			{
				if(pObj == NULL)			return false;
				if(pOwner == NULL)			return false;
				if(iTargetKey < 0)			return false;
				if(dwTick == 0)				return false;
				if(bForceRender == false)	return false;

				return true;
			}
#ifdef LDK_ADD_NEW_PETPROCESS_ADD_SOUND
	virtual bool Sound( OBJECT* pObj, CHARACTER* pOwner, int iTargetKey, DWORD dwTick, bool bForceRender )
			{
				if(pObj == NULL)			return false;
				if(pOwner == NULL)			return false;
				if(iTargetKey < 0)			return false;
				if(dwTick == 0)				return false;
				if(bForceRender == false)	return false;

				return true;
			}
#endif //LDK_ADD_NEW_PETPROCESS_ADD_SOUND
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	virtual bool Release( OBJECT* obj, CHARACTER *Owner ) { return false; }
	
public:
	virtual bool Model( OBJECT *obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender )	{ return false; }
	virtual bool Move( OBJECT *obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender )	{ return false; }
	virtual bool Effect( OBJECT *obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender )	{ return false; }
#ifdef LDK_ADD_NEW_PETPROCESS_ADD_SOUND
	virtual bool Sound( OBJECT *obj, CHARACTER *Owner, int targetKey, DWORD tick, bool bForceRender )	{ return false; }
#endif //LDK_ADD_NEW_PETPROCESS_ADD_SOUND
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
};

#endif // !defined(AFX_W_PETACTION_H__6A4B0CEF_7F8A_4B5D_AF6D_B62BEB89009A__INCLUDED_)
#endif //LDK_ADD_NEW_PETPROCESS