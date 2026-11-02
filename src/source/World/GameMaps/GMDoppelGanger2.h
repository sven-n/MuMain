// GMDoppelGanger2.h: interface for the GMDoppelGanger2 class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "Render/Models/DrawLook.h"

#include "World/MapInfra/w_BaseMap.h"

SmartPointer(CGMDoppelGanger2);
class CGMDoppelGanger2 : public BaseMap
{
public:
    static CGMDoppelGanger2Ptr Make();
    virtual ~CGMDoppelGanger2();

public:
    virtual bool CreateObject(OBJECT* o);
    virtual bool MoveObject(OBJECT* o);
    virtual bool UpdateObjectEffects(OBJECT* o, BMD* b);
    virtual bool RenderObjectMesh(const OBJECT* o, DrawLook& look, BMD* b, bool ExtraMon = 0);
    virtual void RenderAfterObjectMesh(const OBJECT* o, DrawLook& look, BMD* b, bool ExtraMon = 0);

public:
    bool CreateFireSpark(PARTICLE* o);

public:
    virtual CHARACTER* CreateMonster(int iType, int PosX, int PosY, int Key);
    virtual bool MoveMonsterVisual(OBJECT* o, BMD* b);
    virtual void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool UpdateMonsterEffects(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o);

public:
    virtual void PlayObjectSound(OBJECT* o);

public:
    void Init();
    void Destroy();

protected:
    CGMDoppelGanger2();
};

extern bool IsDoppelGanger2();
