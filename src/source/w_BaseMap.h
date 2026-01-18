// w_BaseMap.h: interface for the BaseMap class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#include "ZzzBMD.h"
#include "MapManager.h"

class BaseMap
{
public:
    BaseMap() {}
    virtual ~BaseMap() { clear(); }

public:
    virtual bool LoadMapData() { return false; }

public:	// Object
    virtual bool CreateObject(OBJECT* o) { return false; }
    virtual bool MoveObject(OBJECT* o) { return false; }
    virtual bool RenderObjectVisual(OBJECT* o, BMD* b) { return false; }
    virtual bool RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0) { return false; }
    virtual void RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0) {}

    virtual void RenderFrontSideVisual() {}

public:	// Character
    virtual CHARACTER* CreateMonster(int iType, int PosX, int PosY, int Key) { return NULL; }
    virtual bool MoveMonsterVisual(OBJECT* o, BMD* b) { return false; }
    virtual void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b) {}
    virtual bool RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b) { return false; }
    virtual bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b) { return false; }
    virtual bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o) { return false; }

public: // Weather
    virtual bool CreateWeather(PARTICLE* o) { return false; }

public: // Sound
    virtual bool PlayMonsterSound(OBJECT* o) { return false; }
    virtual void PlayObjectSound(OBJECT* o) {}

public: // Server Message
    virtual bool ReceiveMapMessage(BYTE code, BYTE subcode, BYTE* ReceiveBuffer) { return false; }

public:
    void clear() { m_MapTypes.clear(); }

public:
    bool isMapIndex(ENUM_WORLD type)
    {
        for (int i = 0; i < (int)m_MapTypes.size(); ++i)
        {
            if (type == m_MapTypes[i])
            {
                return true;
            }
        }
        return false;
    }

    ENUM_WORLD FindMapIndex(int index = 0) {
        if (m_MapTypes.size() == 0 || index >= (int)m_MapTypes.size()) return NUM_WD;
        else return m_MapTypes[index];
    }

public:
    const bool IsCurrentMap(int type) { return isMapIndex(static_cast<ENUM_WORLD>(type)); }

public:
    void AddMapIndex(ENUM_WORLD type) { if (!isMapIndex(type)) m_MapTypes.push_back(type); }

public:
    typedef std::vector<ENUM_WORLD>	WorldVector;

private:
    WorldVector		m_MapTypes;
};
