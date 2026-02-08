// GMUnitedMarketPlace.h: interface for the GMUnitedMarketPlace class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "w_BaseMap.h"

SmartPointer(GMUnitedMarketPlace);

class GMUnitedMarketPlace : public BaseMap
{
public:
    static GMUnitedMarketPlacePtr Make();
    virtual ~GMUnitedMarketPlace();

public:
    virtual bool CreateObject(OBJECT* o);
    virtual bool MoveObject(OBJECT* o);
    virtual bool RenderObjectVisual(OBJECT* o, BMD* b);
    virtual bool RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
    virtual void RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
    virtual void RenderFrontSideVisual();
    bool RenderMonster(OBJECT* o, BMD* b, bool ExtraMon = 0);

public:
    virtual CHARACTER* CreateMonster(int iType, int PosX, int PosY, int Key);
    virtual bool MoveMonsterVisual(OBJECT* o, BMD* b);
    virtual void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o);
    bool MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);

public:
    virtual bool PlayMonsterSound(OBJECT* o);
    virtual void PlayObjectSound(OBJECT* o);
    void PlayBGM();

public:
    void Init();
    void Destroy();

public:
    bool CreateRain(PARTICLE* o);
    bool MoveRain(PARTICLE* o);

protected:
    GMUnitedMarketPlace();
};
extern bool IsUnitedMarketPlace();
