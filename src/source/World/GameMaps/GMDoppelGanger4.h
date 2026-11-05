// GMDoppelGanger4.h: interface for the GMDoppelGanger4 class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "Render/Models/DrawLook.h"

#include "World/MapInfra/w_BaseMap.h"

SmartPointer(CGMDoppelGanger4);
class CGMDoppelGanger4 : public BaseMap
{
public:
    static CGMDoppelGanger4Ptr Make();
    virtual ~CGMDoppelGanger4();

public:
    virtual bool CreateObject(OBJECT* o);
    virtual bool MoveObject(OBJECT* o);
    virtual bool UpdateObjectEffects(OBJECT* o, BMD* b);
    virtual bool RenderObjectVisual(const OBJECT* o, DrawLook& look, BMD* b);
    virtual bool RenderObjectMesh(const OBJECT* o, DrawLook& look, BMD* b, bool ExtraMon = 0);
    virtual void RenderAfterObjectMesh(const OBJECT* o, DrawLook& look, BMD* b, bool ExtraMon = 0);

public:
    virtual CHARACTER* CreateMonster(int iType, int PosX, int PosY, int Key);
    virtual bool MoveMonsterVisual(OBJECT* o, BMD* b);
    virtual void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool UpdateMonsterEffects(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool RenderMonsterVisual(const CHARACTER* c, const OBJECT* o, BMD* b);
    virtual bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o);

public:
    virtual void PlayObjectSound(OBJECT* o);

public:
    void Init();
    void Destroy();

protected:
    CGMDoppelGanger4();
};

extern bool IsDoppelGanger4();
