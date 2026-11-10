// GMSantaTown.h: interface for the CGMSantaTown class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Render/Models/DrawLook.h"

#include "World/MapInfra/w_BaseMap.h"

SmartPointer(CGMSantaTown);

class CGMSantaTown : public BaseMap
{
public:
    static CGMSantaTownPtr Make();
    virtual ~CGMSantaTown();

public: // Object
    virtual bool CreateObject(OBJECT* o);
    virtual bool MoveObject(OBJECT* o);
    virtual bool UpdateObjectEffects(OBJECT* o, BMD* b);
    virtual bool RenderObjectMesh(const OBJECT* o, DrawLook& look, BMD* b, bool ExtraMon = 0);
    virtual void RenderAfterObjectMesh(const OBJECT* o, DrawLook& look, BMD* b, bool ExtraMon = 0);

public: // Character
    virtual bool MoveMonsterVisual(OBJECT* o, BMD* b);
    virtual void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool RenderMonsterVisual(const CHARACTER* c, const OBJECT* o, BMD* b);
    virtual bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o);

public: // Sound
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
