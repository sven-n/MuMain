// GMEmpireGuardian3.h: interface for the GMEmpireGuardian3 class.
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Render/Models/DrawLook.h"

#include "World/MapInfra/w_BaseMap.h"

#include <string_view>

SmartPointer(GMEmpireGuardian3);

class GMEmpireGuardian3 : public BaseMap
{
public:
    static GMEmpireGuardian3Ptr Make();
    virtual ~GMEmpireGuardian3();

public:
    virtual bool CreateObject(OBJECT* o);
    virtual bool MoveObject(OBJECT* o);
    virtual bool UpdateObjectEffects(OBJECT* o, BMD* b);
    virtual bool RenderObjectMesh(const OBJECT* o, DrawLook& look, BMD* b, bool ExtraMon = 0);
    virtual void RenderAfterObjectMesh(const OBJECT* o, DrawLook& look, BMD* b, bool ExtraMon = 0);
    virtual void RenderFrontSideVisual();
    bool RenderMonster(const OBJECT* o, DrawLook& look, BMD* b, bool ExtraMon = 0);

public:
    virtual bool MoveMonsterVisual(OBJECT* o, BMD* b);
    virtual void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool UpdateMonsterEffects(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool RenderMonsterVisual(const CHARACTER* c, const OBJECT* o, BMD* b);
    virtual bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o);
    bool MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);

public: // Sound
    virtual void PlayObjectSound(OBJECT* o);
    void PlayBGM();

public:
    void Init();
    void Destroy();

public: // Weather
    bool CreateRain(PARTICLE* o);
    void SetWeather(int weather);

public:
    // The boss's rage ends: a new boss of the kind appears (the spawn hook
    // EmpireGuardianRageReset). False for a boss of another map.
    bool ResetRage(std::string_view boss);

private:
    bool m_bCurrentIsRage_Kato;

protected:
    GMEmpireGuardian3();
};
