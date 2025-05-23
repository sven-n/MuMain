// GMDoppelGanger1.h: interface for the CGMDoppelGanger1 class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "w_BaseMap.h"

SmartPointer(CGMDoppelGanger1);

class CGMDoppelGanger1 : public BaseMap
{
public:
    static CGMDoppelGanger1Ptr Make();
    virtual ~CGMDoppelGanger1();

public:
    virtual bool CreateObject(OBJECT* o);
    virtual bool MoveObject(OBJECT* o);
    virtual bool RenderObjectVisual(OBJECT* o, BMD* b);
    virtual bool RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
    virtual void RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);

public:
    virtual CHARACTER* CreateMonster(int iType, int PosX, int PosY, int Key);
    virtual bool MoveMonsterVisual(OBJECT* o, BMD* b);
    virtual void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o);

public:
    virtual bool PlayMonsterSound(OBJECT* o);
    virtual void PlayObjectSound(OBJECT* o);
    void PlayBGM();

public:
    void Init();
    void Destroy();

protected:
    CGMDoppelGanger1();

    BOOL m_bIsMP3Playing;
};

extern bool IsDoppelGanger1();
