// GMSantaTown.h: interface for the CGMSantaTown class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "w_BaseMap.h"

SmartPointer(CGMSantaTown);

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
    bool CreateSnow(PARTICLE* o);

protected:
    CGMSantaTown();
};

extern bool IsSantaTown();
