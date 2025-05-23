// GMEmpireGuardian1.h: interface for the GMEmpireGuardian1 class.
//
//////////////////////////////////////////////////////////////////////

#pragma once
#include "w_BaseMap.h"

SmartPointer(GMEmpireGuardian1);

class GMEmpireGuardian1 : public BaseMap
{
public:
    static GMEmpireGuardian1Ptr Make();
    virtual ~GMEmpireGuardian1();

public:	// Object
    virtual bool CreateObject(OBJECT* o);
    virtual bool MoveObject(OBJECT* o);
    virtual bool RenderObjectVisual(OBJECT* o, BMD* b);
    virtual bool RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
    virtual void RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
    virtual void RenderFrontSideVisual();
    bool RenderMonster(OBJECT* o, BMD* b, bool ExtraMon = 0);

public:	// Character
    virtual CHARACTER* CreateMonster(int iType, int PosX, int PosY, int Key);
    virtual bool MoveMonsterVisual(OBJECT* o, BMD* b);
    virtual void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b);
    virtual bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o);

    bool MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);

public: // Sound
    virtual bool PlayMonsterSound(OBJECT* o);
    virtual void PlayObjectSound(OBJECT* o);
    void PlayBGM();

public:
    void Init();
    void Destroy();

public: //Weather
    enum WEATHER_TYPE
    {
        WEATHER_SUN = 0,
        WEATHER_RAIN = 1,
        WEATHER_FOG = 2,
        WEATHER_STORM = 3,
    };

    bool CreateRain(PARTICLE* o);

    void SetWeather(int weather) { m_iWeather = weather; }
    int GetWeather() { return m_iWeather; }

private:
    int m_iWeather;

private:
    bool m_bCurrentIsRage_Raymond;
    bool m_bCurrentIsRage_Ercanne;
    bool m_bCurrentIsRage_Daesuler;
    bool m_bCurrentIsRage_Gallia;

protected:
    GMEmpireGuardian1();
};