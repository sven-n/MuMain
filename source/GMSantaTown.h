// GMSantaTown.h: interface for the CGMSantaTown class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GMSANTATOWN_H__BFA5CEE0_59CC_43E9_842B_C5DD402D263D__INCLUDED_)
#define AFX_GMSANTATOWN_H__BFA5CEE0_59CC_43E9_842B_C5DD402D263D__INCLUDED_

#pragma once

#ifdef YDG_ADD_MAP_SANTA_TOWN

#include "w_BaseMap.h"

BoostSmartPointer( CGMSantaTown );
class CGMSantaTown : public BaseMap  
{
public:
	static CGMSantaTownPtr Make();
	virtual ~CGMSantaTown();

public:	// Object
	virtual bool CreateObject(OBJECT* o);
	virtual bool MoveObject(OBJECT* o);
	virtual bool RenderObjectVisual(OBJECT* o, BMD* b);
	virtual bool RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
	virtual void RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
	
public:	// Character
	virtual CHARACTER* CreateMonster(int iType, int PosX, int PosY, int Key);
	virtual bool MoveMonsterVisual(OBJECT* o, BMD* b);
	virtual void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
	virtual bool RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
	virtual bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b);
	virtual bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o);
	
public: // Sound
	virtual bool PlayMonsterSound(OBJECT* o);
	virtual void PlayObjectSound(OBJECT* o);
	void PlayBGM();

public:
	void Init();
	void Destroy();
	bool CreateSnow( PARTICLE* o );

protected:
	CGMSantaTown();
};

extern bool IsSantaTown();

#endif	// YDG_ADD_MAP_SANTA_TOWN

#endif // !defined(AFX_GMSANTATOWN_H__BFA5CEE0_59CC_43E9_842B_C5DD402D263D__INCLUDED_)
