// GMEmpireGuardian1.h: interface for the GMEmpireGuardian1 class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Render/Models/DrawLook.h"
#include "World/MapInfra/w_BaseMap.h"

#include <string_view>

SmartPointer(GMEmpireGuardian1);

class GMEmpireGuardian1 : public BaseMap
{
public:
    static GMEmpireGuardian1Ptr Make();
    virtual ~GMEmpireGuardian1();

public: // Object
    virtual bool CreateObject(OBJECT* o);
    virtual bool MoveObject(OBJECT* o);
    virtual bool UpdateObjectEffects(OBJECT* o, BMD* b);
    virtual bool RenderObjectMesh(const OBJECT* o, DrawLook& look, BMD* b, bool ExtraMon = 0);
    virtual void RenderAfterObjectMesh(const OBJECT* o, DrawLook& look, BMD* b, bool ExtraMon = 0);
    virtual void RenderFrontSideVisual();
    bool RenderMonster(const OBJECT* o, DrawLook& look, BMD* b, bool ExtraMon = 0);
    // The wall cannons fire on every Empire Guardian map.
    void UpdateProjectileLauncher(OBJECT* o, BMD* b);

public: // Character
    virtual bool MoveMonsterVisual(OBJECT* o, BMD* b);
    virtual void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool UpdateMonsterEffects(CHARACTER* c, OBJECT* o, BMD* b);
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
    enum WEATHER_TYPE
    {
        WEATHER_SUN = 0,
        WEATHER_RAIN = 1,
        WEATHER_FOG = 2,
        WEATHER_STORM = 3,
    };

    bool CreateRain(PARTICLE* o);

    void SetWeather(int weather)
    {
        m_iWeather = weather;
    }
    int GetWeather()
    {
        return m_iWeather;
    }

private:
    int m_iWeather;

public:
    // The boss's rage ends: a new boss of the kind appears (the spawn hook
    // EmpireGuardianRageReset). False for a boss of another map.
    bool ResetRage(std::string_view boss);

private:
    bool m_bCurrentIsRage_Raymond;
    bool m_bCurrentIsRage_Ercanne;
    bool m_bCurrentIsRage_Daesuler;
    bool m_bCurrentIsRage_Gallia;

protected:
    GMEmpireGuardian1();
};