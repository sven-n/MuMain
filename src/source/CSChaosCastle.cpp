//////////////////////////////////////////////////////////////////////////
//  CSChaosCastle.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "Random.h"
#include "UIWindows.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "ZzzBMD.h"
#include "Scenes/SceneCore.h"
#include "ZzzEffect.h"
#include "UIManager.h"
#include "CSChaosCastle.h"

#include "NewUICustomMessageBox.h"
#include "MapManager.h"
#include "CharacterManager.h"
#include "DSPlaySound.h"
#include "NewUISystem.h"

#include <algorithm>
#include <array>
#include <cstdint>

extern int g_iChatInputType;
extern int g_iCustomMessageBoxButton[NUM_BUTTON_CMB][NUM_PAR_BUTTON_CMB];
extern  int g_iActionObjectType;
extern  int g_iActionWorld;
extern  int g_iActionTime;
extern  float g_fActionObjectVelocity;

namespace
{
using CastleArea = std::array<int, 4>;
using CastleAreaSet = std::array<CastleArea, 4>;

constexpr CastleAreaSet gChaosCastleLimitArea1{{
    CastleArea{23, 75, 44, 76},
    CastleArea{43, 77, 44, 108},
    CastleArea{23, 107, 42, 108},
    CastleArea{23, 77, 24, 106},
}};

constexpr CastleAreaSet gChaosCastleLimitArea2{{
    CastleArea{25, 77, 42, 78},
    CastleArea{41, 79, 42, 106},
    CastleArea{25, 105, 40, 106},
    CastleArea{25, 79, 26, 104},
}};

constexpr CastleAreaSet gChaosCastleLimitArea3{{
    CastleArea{27, 79, 40, 80},
    CastleArea{39, 81, 40, 104},
    CastleArea{27, 103, 38, 104},
    CastleArea{27, 81, 28, 102},
}};

constexpr std::array<const CastleAreaSet*, 9> kCastleLimitAreas{{
    &gChaosCastleLimitArea1, // 0
    nullptr,                 // 1
    nullptr,                 // 2
    &gChaosCastleLimitArea2, // 3
    nullptr,                 // 4
    nullptr,                 // 5
    &gChaosCastleLimitArea3, // 6
    nullptr,                 // 7
    nullptr,                 // 8
}};

enum class CastleLevel : std::uint8_t
{
    Zero = 0,
    One = 1,
    Two = 2,
    Three = 3,
    Four = 4,
    Five = 5,
    Six = 6,
    Seven = 7,
    Eight = 8,
    Invalid = 255,
};

constexpr std::uint8_t LevelValue(CastleLevel level)
{
    return static_cast<std::uint8_t>(level);
}

constexpr int kActionTriggerTime = 30;

CastleLevel g_currentCastleLevel = CastleLevel::Invalid;
bool g_actionMatch = true;

const CastleAreaSet* SelectLimitArea(CastleLevel level)
{
    const auto index = LevelValue(level);
    if (index < kCastleLimitAreas.size())
    {
        return kCastleLimitAreas[index];
    }
    return nullptr;
}

bool IsWithinLimitArea(const CastleAreaSet& areas, int xi, int yi)
{
    return std::any_of(areas.begin(), areas.end(), [xi, yi](const CastleArea& area) {
        return xi >= area[0] && xi <= area[2] && yi >= area[1] && yi <= area[3];
    });
}
} // namespace

void    ClearChaosCastleHelper(CHARACTER* c)
{
    c->Wing.Type = -1;
    c->Wing.Level = 0;
    c->Helper.Type = -1;
    c->Helper.Level = 0;
    c->Weapon[0].Type = -1;
    c->Weapon[0].Level = 0;
    c->Weapon[0].ExcellentFlags = 0;
    c->Weapon[1].Type = -1;
    c->Weapon[1].Level = 0;
    c->Weapon[1].ExcellentFlags = 0;
#ifdef LJW_FIX_MANY_FLAG_DISAPPEARED_PROBREM
    c->EtcPart = PARTS_NONE;
#endif
}

void    ChangeChaosCastleUnit(CHARACTER* c)
{
    if (gMapManager.InChaosCastle() == false) return;

    ClearChaosCastleHelper(c);

    DWORD t_dwUIID = g_pWindowMgr->GetAddFriendWindow();
    if (t_dwUIID != 0)
    {
        g_pWindowMgr->SendUIMessage(UI_MESSAGE_CLOSE, t_dwUIID, 0);
    }
    if (g_pUIManager->IsOpen(INTERFACE_FRIEND))
    {
        CUIFriendWindow* t_pFW = g_pWindowMgr->GetFriendMainWindow();
        t_pFW->Close();
    }

    int Class = gCharacterManager.GetBaseClass(c->Class);

    if (Class == CLASS_KNIGHT || Class == CLASS_DARK || Class == CLASS_DARK_LORD || Class == CLASS_RAGEFIGHTER)
    {
        c->Weapon[0].Type = MODEL_SWORD_OF_DESTRUCTION;
        c->Weapon[0].Level = 0;
        c->Weapon[1].Type = MODEL_SWORD_OF_DESTRUCTION;
        c->Weapon[1].Level = 0;
    }
    else if (Class == CLASS_ELF)
    {
        c->Weapon[0].Type = MODEL_GREAT_REIGN_CROSSBOW;
        c->Weapon[0].Level = 0;
    }
    else if (Class == CLASS_WIZARD || Class == CLASS_SUMMONER)
    {
        c->Weapon[0].Type = MODEL_LEGENDARY_STAFF;
        c->Weapon[0].Level = 0;
    }
}

bool MoveChaosCastleObjectSetting(int& objCount, int object)
{
    if (gMapManager.InChaosCastle() == false) return false;

    if (rand_fps_check(10) && object)
    {
        objCount = Random::RangeInt(0, object - 1);
        return true;
    }

    if (!Hero->SafeZone && rand_fps_check(10))
    {
        vec3_t Position;

        Position[0] = Hero->Object.Position[0] + Random::RangeFloat(-400, 399);
        Position[1] = Hero->Object.Position[1] + Random::RangeFloat(-400, 399);
        Position[2] = Hero->Object.Position[2] - 150.f;

        CreateJoint(BITMAP_JOINT_SPIRIT2, Position, Position, Hero->Object.Angle, 9, NULL, Random::RangeFloat(50, 59));
    }

    return true;
}

bool MoveChaosCastleObject(OBJECT* o, int& object, int& visibleObject)
{
    if (gMapManager.InChaosCastle() == true)
    {
        int objectCount = object;
        if (o->Type == 3)
        {
            visibleObject++;
            if (g_actionMatch)
            {
                o->LifeTime = 10;
                o->PKKey = 1;
                g_actionMatch = false;
            }
            else if (objectCount)
            {
                o->LifeTime = 10;
                o->PKKey = 0;
                objectCount--;
                if (objectCount == 0)
                {
                    o->PKKey = 1;
                }
            }
            object = objectCount;
        }

        return true;
    }

    return false;
}

bool MoveChaosCastleAllObject(OBJECT* o)
{
    if (gMapManager.InChaosCastle() == false) return false;

    vec3_t Position;

    switch (o->Type)
    {
    case    24:
    case    25:
    case    26:
    case    27:
    case    28:
    case    29:
        if (g_currentCastleLevel == CastleLevel::Seven)
        {
            if (g_iActionTime >= kActionTriggerTime)
            {
                vec3_t Light = { 1.f, 1.f, 1.f };

                Position[0] = o->Position[0] + Random::RangeFloat(-150, 149);
                Position[1] = o->Position[1];
                Position[2] = Hero->Object.Position[2];
                CreateParticle(BITMAP_SMOKE + 4, Position, o->Angle, Light, 0, 1.5f);

                EarthQuake = Random::RangeFloat(-3, -1) * 0.1f;
            }
            else if (g_iActionTime <= 0)
            {
                o->HiddenMesh = -2;
                g_currentCastleLevel = CastleLevel::Eight;

                ClearActionObject();
            }
            else
            {
                o->Position[2] = o->StartPosition[2] - ((kActionTriggerTime - g_iActionTime) * g_fActionObjectVelocity);
                g_fActionObjectVelocity += 0.4f;
            }
        }
        break;

    case    30:
    case    31:
    case    32:
    case    33:
    case    34:
    case    35:
        if (g_currentCastleLevel == CastleLevel::Four)
        {
            if (g_iActionTime >= kActionTriggerTime)
            {
                vec3_t Light = { 1.f, 1.f, 1.f };

                Position[0] = o->Position[0] + Random::RangeFloat(-150, 149);
                Position[1] = o->Position[1];
                Position[2] = Hero->Object.Position[2];
                CreateParticle(BITMAP_SMOKE + 4, Position, o->Angle, Light, 0, 1.5f);

                EarthQuake = Random::RangeFloat(-3, -1) * 0.1f;
            }
            else if (g_iActionTime <= 0)
            {
                o->HiddenMesh = -2;
                g_currentCastleLevel = CastleLevel::Five;

                ClearActionObject();
            }
            else
            {
                o->Position[2] = o->StartPosition[2] - ((30 - g_iActionTime) * g_fActionObjectVelocity);
                g_fActionObjectVelocity += 0.4f;
            }
        }
        break;

    case    0:
    case    1:
    case    2:
    case    3:
    case    4:
    case    5:
    case    13:
    case    14:
    case    15:
    case    16:
    case    17:
        if (g_currentCastleLevel == CastleLevel::One)
        {
            if (g_iActionTime >= kActionTriggerTime)
            {
                vec3_t Light = { 1.f, 1.f, 1.f };

                Position[0] = o->Position[0] + Random::RangeFloat(-150, 149);
                Position[1] = o->Position[1];
                Position[2] = Hero->Object.Position[2];
                CreateParticle(BITMAP_SMOKE + 4, Position, o->Angle, Light, 0, 1.5f);

                EarthQuake = Random::RangeFloat(-3, -1) * 0.1f;
            }
            else if (g_iActionTime <= 0)
            {
                o->HiddenMesh = -2;
                g_currentCastleLevel = CastleLevel::Two;

                ClearActionObject();
            }
            else
            {
                o->Position[2] = o->StartPosition[2] - ((30 - g_iActionTime) * g_fActionObjectVelocity);
                g_fActionObjectVelocity += 0.4f;
            }
        }
        break;
    }

    return true;
}

bool CreateChaosCastleObject(OBJECT* o)
{
    if (gMapManager.InChaosCastle() == false) return false;

    switch (o->Type)
    {
    case    18:
    case    19:
    case    20:
    case    21:
    case    24:
    case    25:
    case    26:
    case    27:
    case    28:
    case    29:
    case    30:
    case    31:
    case    32:
    case    33:
    case    34:
    case    35:
        o->HiddenMesh = -2;
        VectorCopy(o->Position, o->StartPosition);
        break;
    }

    return true;
}

bool RenderChaosCastleVisual(OBJECT* o, BMD* b)
{
    if (gMapManager.InChaosCastle() == false) return false;

    vec3_t p, Position;

    switch (o->Type)
    {
    case    6:
        if (o->HiddenMesh != -2)
        {
            vec3_t  Light;
            Vector(0.05f, 0.05f, 0.1f, Light);
            for (int i = 0; i < 10; ++i)
            {
                CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 0, o->Scale, o);
            }
        }
        o->HiddenMesh = -2;
        break;

    case    7:
        if (o->HiddenMesh != -2)
        {
            vec3_t  Light;
            Vector(0.05f, 0.05f, 0.1f, Light);
            for (int i = 0; i < 10; ++i)
            {
                CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 1, o->Scale, o);
            }
        }
        o->HiddenMesh = -2;
        break;

    case    8:
        if (o->HiddenMesh != -2)
        {
            vec3_t  Light;
            Vector(0.05f, 0.05f, 0.1f, Light);
            for (int i = 0; i < 10; ++i)
            {
                CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 2, o->Scale, o);
            }
        }
        o->HiddenMesh = -2;
        break;

    case    9:
        if (o->HiddenMesh != -2)
        {
            vec3_t  Light;
            Vector(0.05f, 0.05f, 0.1f, Light);
            for (int i = 0; i < 5; ++i)
            {
                CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 3, o->Scale, o);
            }
        }
        o->HiddenMesh = -2;
        break;

    case    10:
        if (o->HiddenMesh != -2)
        {
            vec3_t  Light;
            Vector(0.05f, 0.05f, 0.1f, Light);
            for (int i = 0; i < 5; ++i)
            {
                CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 4, o->Scale, o);
            }
        }
        o->HiddenMesh = -2;
        break;

    case    11:
        if (o->HiddenMesh != -2)
        {
            vec3_t  Light;
            Vector(0.05f, 0.05f, 0.1f, Light);
            for (int i = 0; i < 5; ++i)
            {
                CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 5, o->Scale, o);
            }
        }
        o->HiddenMesh = -2;
        break;

    case    12:
        if (o->HiddenMesh != -2)
        {
            vec3_t  Light;
            Vector(0.3f, 0.3f, 0.3f, Light);
            for (int i = 0; i < 7; ++i)
            {
                CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 7, o->Scale, o);
            }
        }
        o->HiddenMesh = -2;
        break;

    case    18:
    case    19:
    case    20:
    case    21:
        if (g_currentCastleLevel == CastleLevel::Seven || g_currentCastleLevel == CastleLevel::Eight)
        {
            o->HiddenMesh = -1;
        }
        else
        {
            o->HiddenMesh = -2;
        }
        break;

    case    24:
    case    25:
    case    26:
    case    27:
    case    28:
    case    29:
        if (g_currentCastleLevel == CastleLevel::Four || g_currentCastleLevel == CastleLevel::Five)
        {
            o->HiddenMesh = -1;
        }
        else if (LevelValue(g_currentCastleLevel) >= LevelValue(CastleLevel::Eight))
        {
            o->HiddenMesh = -2;
        }
        break;

    case    30:
    case    31:
    case    32:
    case    33:
    case    34:
    case    35:
        if (g_currentCastleLevel == CastleLevel::One || g_currentCastleLevel == CastleLevel::Two)
        {
            o->HiddenMesh = -1;
        }
        else if (LevelValue(g_currentCastleLevel) >= LevelValue(CastleLevel::Five))
        {
            o->HiddenMesh = -2;
        }
        break;

    case    0:
    case    1:
    case    2:
    case    3:
        if (o->PKKey && o->HiddenMesh != -2)
        {
            Vector(0.f, 0.f, 0.f, p);
            b->TransformPosition(BoneTransform[1], p, Position);
            if ((int)o->LifeTime == 10)
            {
                CreateJoint(BITMAP_JOINT_THUNDER + 1, Position, Position, o->Angle, 2, NULL, 60.f + Random::RangeFloat(0, 9));

                int randValue = Random::RangeInt(0, 1);
                PlayBuffer(static_cast<ESound>(SOUND_CHAOS_THUNDER01 + randValue));
                o->LifeTime = 9.9f;
            }

            if (o->LifeTime < 5)
            {
                o->PKKey = 0;
            }
            else
            {
                o->LifeTime -= FPS_ANIMATION_FACTOR;
            }
        }
    case    4:
    case    5:
    case    13:
    case    14:
    case    15:
    case    16:
    case    17:
        if (LevelValue(g_currentCastleLevel) >= LevelValue(CastleLevel::Two) && g_currentCastleLevel != CastleLevel::Invalid)
        {
            o->HiddenMesh = -2;
        }
        break;
    }

    return true;
}

void RenderTerrainVisual(int xi, int yi)
{
    if (gMapManager.InChaosCastle() == false || Random::RangeInt(0, 7) != 0)
    {
        return;
    }

    const CastleAreaSet* areaSet = SelectLimitArea(g_currentCastleLevel);
    if (areaSet == nullptr)
    {
        return;
    }

    if (IsWithinLimitArea(*areaSet, xi, yi))
    {
        vec3_t Light = { 1.f, 1.f, 1.f };
        vec3_t Angle = { 0.f, 0.f, 0.f };
        vec3_t Position;

        Position[0] = (xi * TERRAIN_SCALE) + Random::RangeFloat(-15, 14);
        Position[1] = (yi * TERRAIN_SCALE) + Random::RangeFloat(-15, 14);
        Position[2] = Hero->Object.Position[2];
        CreateParticleFpsChecked(BITMAP_SMOKE + 4, Position, Angle, Light, 0, 1.5f);

        if (rand_fps_check(5))
        {
            EarthQuake = Random::RangeFloat(-3, -1) * 0.1f;
        }
    }
}