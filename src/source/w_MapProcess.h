// w_MapProcess.h: interface for the MapProcess class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "w_BaseMap.h"
#include "MapManager.h"

SmartPointer(MapProcess);
class MapProcess
{
public:
    static MapProcessPtr Make();
    virtual ~MapProcess();

public:
    bool LoadMapData();

public:
    bool CreateObject(OBJECT* o);
    bool MoveObject(OBJECT* o);
    bool RenderObjectVisual(OBJECT* o, BMD* b);
    bool RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
    void RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
    void RenderFrontSideVisual();

public:
    CHARACTER* CreateMonster(int iType, int PosX, int PosY, int Key);
    bool MoveMonsterVisual(OBJECT* o, BMD* b);
    void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
    bool RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);
    bool AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b);
    bool SetCurrentActionMonster(CHARACTER* c, OBJECT* o);

public:
    bool PlayMonsterSound(OBJECT* o);

public:
    bool ReceiveMapMessage(BYTE code, BYTE subcode, BYTE* ReceiveBuffer);

public:
    void Register(Smart_Ptr(BaseMap) pMap);
    void UnRegister(ENUM_WORLD type);

public:
    BaseMap& GetMap(int type);

private:
    bool FindMap(ENUM_WORLD type);
    BaseMap& FindBaseMap(ENUM_WORLD type);
    void Init();
    void Destroy();
    MapProcess();

private:
    typedef std::list< Smart_Ptr(BaseMap) >		MapList;

private:
    MapList				m_MapList;
};

extern MapProcessPtr g_MapProcess;

extern MapProcess& TheMapProcess();
