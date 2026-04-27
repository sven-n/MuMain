// GMEmpireGuardian2.h: interface for the GMEmpireGuardian2 class.
//////////////////////////////////////////////////////////////////////
#pragma once

#include "w_BaseMap.h"

SmartPointer(GMEmpireGuardian2);

class GMEmpireGuardian2 : public BaseMap
{
public:
    static GMEmpireGuardian2Ptr Make();
    virtual ~GMEmpireGuardian2();

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
    bool CreateRain(PARTICLE* o);
    void SetWeather(int weather);

private:
    bool m_bCurrentIsRage_Bermont;

public:
    void Init();
    void Destroy();

protected:
    GMEmpireGuardian2();
};
