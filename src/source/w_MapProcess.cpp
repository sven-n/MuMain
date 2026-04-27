// w_MapProcess.cpp: implementation of the MapProcess class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_MapProcess.h"
#include "w_MapHeaders.h"

MapProcessPtr g_MapProcess;

MapProcess& TheMapProcess()
{
    assert(g_MapProcess);
    return *g_MapProcess;
}

MapProcessPtr MapProcess::Make()
{
    MapProcessPtr mapprocess(new MapProcess);
    mapprocess->Init();
    return mapprocess;
}

MapProcess::MapProcess()
{
}

MapProcess::~MapProcess()
{
    Destroy();
}

void MapProcess::Init()
{
    SEASON4A::CGM_RaklionPtr raklion = SEASON4A::CGM_Raklion::Make();
    raklion->AddMapIndex(WD_57ICECITY);
    raklion->AddMapIndex(WD_58ICECITY_BOSS);
    Register(raklion);

    CGMSantaTownPtr santatown = CGMSantaTown::Make();
    santatown->AddMapIndex(WD_62SANTA_TOWN);
    Register(santatown);

    CGM_PK_FieldPtr pkfield = CGM_PK_Field::Make();
    pkfield->AddMapIndex(WD_63PK_FIELD);
    Register(pkfield);

    CGMDuelArenaPtr duelarena = CGMDuelArena::Make();
    duelarena->AddMapIndex(WD_64DUELARENA);
    Register(duelarena);

    CGMDoppelGanger1Ptr doppelganger1 = CGMDoppelGanger1::Make();
    doppelganger1->AddMapIndex(WD_65DOPPLEGANGER1);
    Register(doppelganger1);

    CGMDoppelGanger2Ptr doppelganger2 = CGMDoppelGanger2::Make();
    doppelganger2->AddMapIndex(WD_66DOPPLEGANGER2);
    Register(doppelganger2);

    CGMDoppelGanger3Ptr doppelganger3 = CGMDoppelGanger3::Make();
    doppelganger3->AddMapIndex(WD_67DOPPLEGANGER3);
    Register(doppelganger3);

    CGMDoppelGanger4Ptr doppelganger4 = CGMDoppelGanger4::Make();
    doppelganger4->AddMapIndex(WD_68DOPPLEGANGER4);
    Register(doppelganger4);

    GMEmpireGuardian1Ptr empireguardian1 = GMEmpireGuardian1::Make();
    empireguardian1->AddMapIndex(WD_69EMPIREGUARDIAN1);
    Register(empireguardian1);

    GMEmpireGuardian2Ptr empireguardian2 = GMEmpireGuardian2::Make();
    empireguardian2->AddMapIndex(WD_70EMPIREGUARDIAN2);
    Register(empireguardian2);

    GMEmpireGuardian3Ptr empireguardian3 = GMEmpireGuardian3::Make();
    empireguardian3->AddMapIndex(WD_71EMPIREGUARDIAN3);
    Register(empireguardian3);

    GMEmpireGuardian4Ptr empireguardian4 = GMEmpireGuardian4::Make();
    empireguardian4->AddMapIndex(WD_72EMPIREGUARDIAN4);
    Register(empireguardian4);

    GMUnitedMarketPlacePtr unitedMarketPlace = GMUnitedMarketPlace::Make();
    unitedMarketPlace->AddMapIndex(WD_79UNITEDMARKETPLACE);
    Register(unitedMarketPlace);

#ifdef ASG_ADD_MAP_KARUTAN
    CGMKarutan1Ptr karutan1 = CGMKarutan1::Make();
    karutan1->AddMapIndex(WD_80KARUTAN1);
    karutan1->AddMapIndex(WD_81KARUTAN2);
    Register(karutan1);
#endif	// ASG_ADD_MAP_KARUTAN
}

void MapProcess::Destroy()
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            m_MapList.erase(tempiter);
        }
    }
    m_MapList.clear();
}

bool MapProcess::FindMap(ENUM_WORLD type)
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            bool result = basemap.lock()->IsCurrentMap(type);
            if (result) {
                return true;
            }
        }
    }
    return false;
}

BaseMap& MapProcess::FindBaseMap(ENUM_WORLD type)
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            if (basemap.lock()->IsCurrentMap(type) == true) {
                return **tempiter;
            }
        }
    }
    assert(0);
    throw;
}

void MapProcess::Register(Smart_Ptr(BaseMap) pMap)
{
    ENUM_WORLD type = pMap->FindMapIndex();
    if (type == NUM_WD) {
        assert(0);
        throw;
    }

    if (FindMap(type) == false) {
        m_MapList.push_back(pMap);
    }
}

void MapProcess::UnRegister(ENUM_WORLD type)
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            bool result = basemap.lock()->IsCurrentMap(type);
            if (result) {
                m_MapList.erase(tempiter);
                return;
            }
        }
    }
}

BaseMap& MapProcess::GetMap(int type)
{
    return FindBaseMap(static_cast<ENUM_WORLD>(type));
}

bool MapProcess::LoadMapData()
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            if (basemap.lock()->IsCurrentMap(gMapManager.WorldActive)) {
                bool result = basemap.lock()->LoadMapData();
                if (result) {
                    return true;
                }
            }
        }
        else {
            m_MapList.erase(tempiter);
        }
    }
    return false;
}

bool MapProcess::CreateObject(OBJECT* o)
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            bool result = basemap.lock()->CreateObject(o);
            if (result) {
                return true;
            }
        }
        else {
            m_MapList.erase(tempiter);
        }
    }
    return false;
}

bool MapProcess::MoveObject(OBJECT* o)
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            if (basemap.lock()->IsCurrentMap(gMapManager.WorldActive)) {
                basemap.lock()->PlayObjectSound(o);
                bool result = basemap.lock()->MoveObject(o);
                if (result) {
                    return true;
                }
            }
        }
        else {
            m_MapList.erase(tempiter);
        }
    }
    return false;
}

bool MapProcess::RenderObjectVisual(OBJECT* o, BMD* b)
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            if (basemap.lock()->IsCurrentMap(gMapManager.WorldActive)) {
                bool result = basemap.lock()->RenderObjectVisual(o, b);
                if (result) {
                    return true;
                }
            }
        }
        else {
            m_MapList.erase(tempiter);
        }
    }
    return false;
}

bool MapProcess::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            if (basemap.lock()->IsCurrentMap(gMapManager.WorldActive)) {
                bool result = basemap.lock()->RenderObjectMesh(o, b, ExtraMon);
                if (result) {
                    return true;
                }
            }
        }
        else {
            m_MapList.erase(tempiter);
        }
    }
    return false;
}

void MapProcess::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            if (basemap.lock()->IsCurrentMap(gMapManager.WorldActive)) {
                basemap.lock()->RenderAfterObjectMesh(o, b, ExtraMon);
            }
        }
        else {
            m_MapList.erase(tempiter);
        }
    }
}

void MapProcess::RenderFrontSideVisual()
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false)
        {
            if (basemap.lock()->IsCurrentMap(gMapManager.WorldActive))
            {
                basemap.lock()->RenderFrontSideVisual();
            }
        }
        else
        {
            m_MapList.erase(tempiter);
        }
    }
}

CHARACTER* MapProcess::CreateMonster(int iType, int PosX, int PosY, int Key)
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            CHARACTER* p = basemap.lock()->CreateMonster(iType, PosX, PosY, Key);
            if (p) {
                return p;
            }
        }
        else {
            m_MapList.erase(tempiter);
        }
    }

    return NULL;
}

bool MapProcess::MoveMonsterVisual(OBJECT* o, BMD* b)
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            if (basemap.lock()->IsCurrentMap(gMapManager.WorldActive)) {
                bool result = basemap.lock()->MoveMonsterVisual(o, b);
                if (result) {
                    return true;
                }
            }
        }
        else {
            m_MapList.erase(tempiter);
        }
    }
    return false;
}

void MapProcess::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            if (basemap.lock()->IsCurrentMap(gMapManager.WorldActive)) {
                basemap.lock()->MoveBlurEffect(c, o, b);
            }
        }
        else {
            m_MapList.erase(tempiter);
        }
    }
}

bool MapProcess::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            if (basemap.lock()->IsCurrentMap(gMapManager.WorldActive)) {
                bool result = basemap.lock()->RenderMonsterVisual(c, o, b);
                if (result) {
                    return true;
                }
            }
        }
        else {
            m_MapList.erase(tempiter);
        }
    }
    return false;
}

bool MapProcess::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            if (basemap.lock()->IsCurrentMap(gMapManager.WorldActive)) {
                bool result = basemap.lock()->AttackEffectMonster(c, o, b);
                if (result) {
                    return true;
                }
            }
        }
        else {
            m_MapList.erase(tempiter);
        }
    }
    return false;
}

bool MapProcess::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            if (basemap.lock()->IsCurrentMap(gMapManager.WorldActive)) {
                bool result = basemap.lock()->SetCurrentActionMonster(c, o);
                if (result) {
                    return true;
                }
            }
        }
        else {
            m_MapList.erase(tempiter);
        }
    }
    return false;
}

bool MapProcess::PlayMonsterSound(OBJECT* o)
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            if (basemap.lock()->IsCurrentMap(gMapManager.WorldActive)) {
                bool result = basemap.lock()->PlayMonsterSound(o);
                if (result) {
                    return true;
                }
            }
        }
        else {
            m_MapList.erase(tempiter);
        }
    }
    return false;
}

bool MapProcess::ReceiveMapMessage(BYTE code, BYTE subcode, BYTE* ReceiveBuffer)
{
    for (auto iter = m_MapList.begin(); iter != m_MapList.end(); )
    {
        auto tempiter = iter;
        ++iter;
        Weak_Ptr(BaseMap) basemap = *tempiter;

        if (basemap.expired() == false) {
            if (basemap.lock()->IsCurrentMap(gMapManager.WorldActive)) {
                bool result = basemap.lock()->ReceiveMapMessage(code, subcode, ReceiveBuffer);
                if (result) {
                    return true;
                }
            }
        }
        else {
            m_MapList.erase(tempiter);
        }
    }
    return false;
}