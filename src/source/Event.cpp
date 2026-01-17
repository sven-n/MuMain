// Event.cpp: implementation of the CEvent class.
//
//////////////////////////////////////////////////////////////////////

// common
#include "stdafx.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzTexture.h"
#include "ZzzOpenData.h"
#include "ZzzEffect.h"
#include "ZzzLodTerrain.h"
#include "BoneManager.h"
#include "UIManager.h"
#include "CKANTURUDirection.h"
#include "CDirection.h"
#include "UIBaseDef.h"
#include "DSPlaySound.h"
#include "LoadData.h"
#include "Event.h"
#include "Random.h"

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cwchar>
#include <limits>

namespace
{
using Clock = std::chrono::steady_clock;

constexpr std::int32_t kChristmasEffectIdBase =
    static_cast<std::int32_t>(std::numeric_limits<std::int16_t>::max()) - 60;
constexpr std::int32_t kChristmasEffectIdLimit =
    static_cast<std::int32_t>(std::numeric_limits<std::int16_t>::max()) - 10;

constexpr std::array<std::array<float, 3>, 3> kSnowBurstOffsets{{
    {50.f, 50.f, 50.f},
    {-50.f, -50.f, 50.f},
    {50.f, -50.f, 50.f},
}};

template <std::size_t N>
void CopyWideString(wchar_t (&destination)[N], const wchar_t* source)
{
    if constexpr (N == 0)
    {
        return;
    }

    if (source == nullptr)
    {
        destination[0] = L'\0';
        return;
    }

    std::wcsncpy(destination, source, N - 1);
    destination[N - 1] = L'\0';
}

void CreateSnowBursts(OBJECT& object, vec3_t light)
{
    for (const auto& offset : kSnowBurstOffsets)
    {
        vec3_t position;
        VectorCopy(object.Position, position);
        position[0] += offset[0];
        position[1] += offset[1];
        position[2] += offset[2];

        CreateParticle(BITMAP_EXPLOTION_MONO, position, object.Angle, light, 0, 0.6f);
        for (int i = 0; i < 2; ++i)
        {
            CreateEffect(MODEL_ICE_SMALL, position, object.Angle, light, 0);
            CreateParticle(BITMAP_CLUD64, position, object.Angle, light, 3, 1.0f);
            CreateParticle(BITMAP_CLUD64, position, object.Angle, light, 3, 1.0f);
            CreateEffect(MODEL_HALLOWEEN_CANDY_STAR, position, object.Angle, light, 1);
            CreateParticle(BITMAP_SNOW_EFFECT_1, position, object.Angle, light, 0, 0.5f);
            CreateParticle(BITMAP_SNOW_EFFECT_1, position, object.Angle, light, 0, 0.5f);
        }
    }
}
} // namespace

///////////////////////////////////////////////////////////////////////////////////////////////////////
// 크리스마스 이벤트
///////////////////////////////////////////////////////////////////////////////////////////////////////

CXmasEvent::CXmasEvent()
{
    m_iEffectID = kChristmasEffectIdBase;
}

CXmasEvent::~CXmasEvent()
{
}

void CXmasEvent::LoadXmasEvent()
{
    gLoadData.AccessModel(MODEL_XMAS_EVENT_CHA_SSANTA, L"Data\\Skill\\", L"xmassanta");
    gLoadData.OpenTexture(MODEL_XMAS_EVENT_CHA_SSANTA, L"Skill\\");

    gLoadData.AccessModel(MODEL_XMAS_EVENT_CHA_SNOWMAN, L"Data\\Skill\\", L"xmassnowman");
    gLoadData.OpenTexture(MODEL_XMAS_EVENT_CHA_SNOWMAN, L"Skill\\");

    gLoadData.AccessModel(MODEL_XMAS_EVENT_CHA_DEER, L"Data\\Skill\\", L"xmassaum");
    gLoadData.OpenTexture(MODEL_XMAS_EVENT_CHA_DEER, L"Skill\\");

    gLoadData.AccessModel(MODEL_XMAS_EVENT_CHANGE_GIRL, L"Data\\Skill\\", L"santa");
    gLoadData.OpenTexture(MODEL_XMAS_EVENT_CHANGE_GIRL, L"Skill\\");

    gLoadData.AccessModel(MODEL_XMAS_EVENT_EARRING, L"Data\\Skill\\", L"ring");
    gLoadData.OpenTexture(MODEL_XMAS_EVENT_EARRING, L"Skill\\");

    gLoadData.AccessModel(MODEL_XMAS_EVENT_ICEHEART, L"Data\\Skill\\", L"xmaseicehart");
    gLoadData.OpenTexture(MODEL_XMAS_EVENT_ICEHEART, L"Skill\\");

    gLoadData.AccessModel(MODEL_XMAS_EVENT_BOX, L"Data\\Skill\\", L"xmasebox");
    gLoadData.OpenTexture(MODEL_XMAS_EVENT_BOX, L"Skill\\");

    gLoadData.AccessModel(MODEL_XMAS_EVENT_CANDY, L"Data\\Skill\\", L"xmasecandy");
    gLoadData.OpenTexture(MODEL_XMAS_EVENT_CANDY, L"Skill\\");

    gLoadData.AccessModel(MODEL_XMAS_EVENT_TREE, L"Data\\Skill\\", L"xmasetree");
    gLoadData.OpenTexture(MODEL_XMAS_EVENT_TREE, L"Skill\\");

    gLoadData.AccessModel(MODEL_XMAS_EVENT_SOCKS, L"Data\\Skill\\", L"xmaseyangbal");
    gLoadData.OpenTexture(MODEL_XMAS_EVENT_SOCKS, L"Skill\\");
}

void CXmasEvent::LoadXmasEventEffect()
{
    LoadBitmap(L"Effect\\snowseff01.jpg", BITMAP_SNOW_EFFECT_1, GL_LINEAR, GL_CLAMP);
    LoadBitmap(L"Effect\\snowseff02.jpg", BITMAP_SNOW_EFFECT_2, GL_LINEAR, GL_CLAMP);
}

void CXmasEvent::LoadXmasEventItem()
{
    gLoadData.AccessModel(MODEL_CHRISTMAS_STAR, L"Data\\Item\\", L"MagicBox", 2);
    gLoadData.OpenTexture(MODEL_CHRISTMAS_STAR, L"Item\\");
}

void CXmasEvent::LoadXmasEventSound()
{
    LoadWaveFile(SOUND_XMAS_JUMP_SNOWMAN, L"Data\\Sound\\xmasjumpsnowman.wav", 1);
    LoadWaveFile(SOUND_XMAS_JUMP_DEER, L"Data\\Sound\\xmasjumpsasum.wav", 1);
    LoadWaveFile(SOUND_XMAS_JUMP_SANTA, L"Data\\Sound\\xmasjumpsanta.wav", 1);
    LoadWaveFile(SOUND_XMAS_TURN, L"Data\\Sound\\xmasturn.wav", 1);
}

void CXmasEvent::CreateXmasEventEffect(CHARACTER* pCha, OBJECT* pObj, int iType)
{
    if (pCha->m_iTempKey >= 0)
    {
        DeleteCharacter(pCha->m_iTempKey);
    }

    GenID();

    CHARACTER* c = CreateCharacter(m_iEffectID, MODEL_PLAYER, pCha->PositionX, pCha->PositionY, pCha->Rot);

    pCha->m_iTempKey = m_iEffectID;
    c->Object.Scale = 0.30f;
    c->Object.SubType = iType + MODEL_XMAS_EVENT_CHA_SSANTA;

    switch (c->Object.SubType)
    {
    case MODEL_XMAS_EVENT_CHA_SSANTA:
        CopyWideString(c->ID, GlobalText[2245]);
        break;
    case MODEL_XMAS_EVENT_CHA_DEER:
        CopyWideString(c->ID, GlobalText[2246]);
        break;
    case MODEL_XMAS_EVENT_CHA_SNOWMAN:
        CopyWideString(c->ID, GlobalText[2247]);
        break;
    default:
        CopyWideString(c->ID, L"");
        break;
    }

    c->Object.m_bRenderShadow = false;
    c->Object.Owner = pObj;

    c->Object.m_dwTime = GetMillisecondsTimestamp();

    OBJECT* o = &c->Object;

    VectorCopy(pObj->Position, o->Position);
    VectorCopy(pObj->Angle, o->Angle);
    o->PriorAction = pObj->PriorAction;
    o->PriorAnimationFrame = pObj->PriorAnimationFrame;
    o->CurrentAction = pObj->CurrentAction;
    o->AnimationFrame = pObj->AnimationFrame;

    vec3_t vLight;
    Vector(0.6f, 0.6f, 0.6f, vLight);

    CreateSnowBursts(*o, vLight);

    if (o->CurrentAction == PLAYER_SANTA_2)
    {
        vec3_t vPos, vAngle, vLight;
        VectorCopy(o->Position, vPos);
        vPos[2] += 230.f;
        Vector(1.f, 1.f, 1.f, vLight);
        Vector(0.f, 0.f, 40.f, vAngle);
        CreateEffect(MODEL_XMAS_EVENT_ICEHEART, vPos, vAngle, vLight, 0, o);
        CreateParticle(BITMAP_DS_EFFECT, vPos, o->Angle, vLight, 0, 3.f, o);
        Vector(1.f, 0.f, 0.f, vLight);
        CreateParticle(BITMAP_LIGHT, vPos, o->Angle, vLight, 10, 3.f, o);
    }
}

void CXmasEvent::GenID()
{
    if (m_iEffectID >= kChristmasEffectIdLimit)
    {
        m_iEffectID = kChristmasEffectIdBase;
    }

    ++m_iEffectID;
}

CNewYearsDayEvent::CNewYearsDayEvent()
{
}

CNewYearsDayEvent::~CNewYearsDayEvent()
{
}

void CNewYearsDayEvent::LoadModel()
{
    gLoadData.AccessModel(MODEL_NEWYEARSDAY_EVENT_BEKSULKI, L"Data\\Monster\\", L"sulbeksulki");
    gLoadData.OpenTexture(MODEL_NEWYEARSDAY_EVENT_BEKSULKI, L"Monster\\");

    gLoadData.AccessModel(MODEL_NEWYEARSDAY_EVENT_CANDY, L"Data\\Monster\\", L"sulcandy");
    gLoadData.OpenTexture(MODEL_NEWYEARSDAY_EVENT_CANDY, L"Monster\\");

    gLoadData.AccessModel(MODEL_NEWYEARSDAY_EVENT_MONEY, L"Data\\Monster\\", L"sulgold");
    gLoadData.OpenTexture(MODEL_NEWYEARSDAY_EVENT_MONEY, L"Monster\\");

    gLoadData.AccessModel(MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_GREEN, L"Data\\Monster\\", L"sulgreengochu");
    gLoadData.OpenTexture(MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_GREEN, L"Monster\\");

    gLoadData.AccessModel(MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_RED, L"Data\\Monster\\", L"sulredgochu");
    gLoadData.OpenTexture(MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_RED, L"Monster\\");

    gLoadData.AccessModel(MODEL_NEWYEARSDAY_EVENT_PIG, L"Data\\Monster\\", L"sulpeg");
    gLoadData.OpenTexture(MODEL_NEWYEARSDAY_EVENT_PIG, L"Monster\\");

    gLoadData.AccessModel(MODEL_NEWYEARSDAY_EVENT_YUT, L"Data\\Monster\\", L"sulyutnulre");
    gLoadData.OpenTexture(MODEL_NEWYEARSDAY_EVENT_YUT, L"Monster\\");
}

void CNewYearsDayEvent::LoadSound()
{
    LoadWaveFile(SOUND_NEWYEARSDAY_DIE, L"Data\\Sound\\newyeardie.wav", 1);
}

CHARACTER* CNewYearsDayEvent::CreateMonster(int iType, int iPosX, int iPosY, int iKey)
{
    CHARACTER* pCharacter = nullptr;

    switch (iType)
    {
    case MONSTER_POUCH_OF_BLESSING:
    {
        OpenMonsterModel(MONSTER_MODEL_POUCH_OF_BLESSING);
        pCharacter = CreateCharacter(iKey, MODEL_POUCH_OF_BLESSING, iPosX, iPosY);
        if (pCharacter != nullptr)
        {
            CopyWideString(pCharacter->ID, L"Fortune Pouch");
            pCharacter->Object.Scale = 1.5f;
        }
    }
    break;
    }

    return pCharacter;
}

bool CNewYearsDayEvent::MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (c == nullptr || o == nullptr || b == nullptr)
    {
        return false;
    }
    switch (o->Type)
    {
    case MODEL_POUCH_OF_BLESSING:
    {
        vec3_t vRelativePos, vWorldPos;
        Vector(0.f, 0.f, 0.f, vRelativePos);
        b->TransformPosition(o->BoneTransform[81], vRelativePos, vWorldPos, true);

        vec3_t vLight;
        Vector(1.f, 1.f, 1.f, vLight);

        if (o->CurrentAction >= MONSTER01_STOP1 && o->CurrentAction <= MONSTER01_ATTACK2)
        {
            if (rand_fps_check(3))
            {
                Vector(1.0f, 0.4f, 0.0f, vLight);
                CreateParticle(BITMAP_LIGHT + 2, vWorldPos, o->Angle, vLight, 0, 0.28f);

                if (rand_fps_check(2))
                {
                    Vector(0.8f, 0.6f, 0.1f, vLight);
                }
                else
                {
                    vLight[0] = Random::Unit();
                    vLight[1] = Random::Unit();
                    vLight[2] = Random::Unit();
                }
                CreateParticle(BITMAP_SHINY, vWorldPos, o->Angle, vLight, 4, 0.8f);
            }
        }
        else if (o->CurrentAction == MONSTER01_SHOCK || o->CurrentAction == MONSTER01_DIE)
        {
            if (rand_fps_check(3))
            {
                Vector(1.0f, 0.4f, 0.0f, vLight);
                const float fScale = (o->CurrentAction == MONSTER01_SHOCK) ? 0.4f : 0.5f;

                CreateParticle(BITMAP_LIGHT + 2, vWorldPos, o->Angle, vLight, 0, fScale);

                for (int i = 0; i < 5; ++i)
                {
                    if (rand_fps_check(2))
                    {
                        Vector(0.8f, 0.6f, 0.1f, vLight);
                    }
                    else
                    {
                        vLight[0] = Random::Unit();
                        vLight[1] = Random::Unit();
                        vLight[2] = Random::Unit();
                    }
                    CreateParticle(BITMAP_SHINY, vWorldPos, o->Angle, vLight, 4, 0.8f);
                }
            }
            if (rand_fps_check(2))
            {
                Vector(0.3f, 0.3f, 0.8f, vLight);
                CreateParticle(BITMAP_LIGHT + 2, vWorldPos, o->Angle, vLight, 1, 1.f);
            }
        }

        if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->AnimationFrame <= 0.3f)
            {
                o->m_iAnimation = Random::RangeInt(0, 5) + MODEL_NEWYEARSDAY_EVENT_BEKSULKI;
                CreateParticleFpsChecked(BITMAP_EXPLOTION, vWorldPos, o->Angle, vLight, 0, 0.5f);
                if (rand_fps_check(4)) o->m_iAnimation = MODEL_NEWYEARSDAY_EVENT_PIG;

                PlayBuffer(SOUND_NEWYEARSDAY_DIE);
            }

            if (o->AnimationFrame >= 4.5f)
            {
                CreateEffectFpsChecked(MODEL_NEWYEARSDAY_EVENT_MONEY, vWorldPos, o->Angle, vLight);
            }

            if (o->m_iAnimation != 0 && rand_fps_check(3))
            {
                if (o->AnimationFrame >= 4.5f)
                {
                    CreateEffectFpsChecked(o->m_iAnimation, vWorldPos, o->Angle, vLight);
                }
            }
        }
    }
    break;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
// 행운의 파란가방 이벤트
///////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef CSK_FIX_BLUELUCKYBAG_MOVECOMMAND

const DWORD CBlueLuckyBagEvent::m_dwBlueLuckyBagCheckTime = 600000;

CBlueLuckyBagEvent::CBlueLuckyBagEvent()
{
    m_bBlueLuckyBag = false;
    m_dwBlueLuckyBagTime = 0;
}

CBlueLuckyBagEvent::~CBlueLuckyBagEvent()
{
}

void CBlueLuckyBagEvent::StartBlueLuckyBag()
{
    m_bBlueLuckyBag = true;
    m_dwBlueLuckyBagTime = timeGetTime();
}

void CBlueLuckyBagEvent::CheckTime()
{
    if (m_bBlueLuckyBag == true)
    {
        if (timeGetTime() - m_dwBlueLuckyBagTime > m_dwBlueLuckyBagCheckTime)
        {
            m_bBlueLuckyBag = false;
            m_dwBlueLuckyBagTime = 0;
        }
    }
}

bool CBlueLuckyBagEvent::IsEnableBlueLuckyBag()
{
    return m_bBlueLuckyBag;
}
#endif // CSK_FIX_BLUELUCKYBAG_MOVECOMMAND

C09SummerEvent::C09SummerEvent()
{
}

C09SummerEvent::~C09SummerEvent()
{
}

void C09SummerEvent::LoadModel()
{
}

void C09SummerEvent::LoadSound()
{
    LoadWaveFile(SOUND_UMBRELLA_MONSTER_WALK1, L"Data\\Sound\\UmbMon_Walk01.wav", 1);
    LoadWaveFile(SOUND_UMBRELLA_MONSTER_WALK2, L"Data\\Sound\\UmbMon_Walk02.wav", 1);
    LoadWaveFile(SOUND_UMBRELLA_MONSTER_DAMAGE, L"Data\\Sound\\UmbMon_Damage01.wav", 1);
    LoadWaveFile(SOUND_UMBRELLA_MONSTER_DEAD, L"Data\\Sound\\UmbMon_Dead.wav", 1);
}

CHARACTER* C09SummerEvent::CreateMonster(int iType, int iPosX, int iPosY, int iKey)
{
    CHARACTER* pCharacter = nullptr;

    if (iType == MONSTER_FIRE_FLAME_GHOST)
    {
        OpenMonsterModel(MONSTER_MODEL_FIRE_FLAME_GHOST);
        pCharacter = CreateCharacter(iKey, MODEL_FIRE_FLAME_GHOST, iPosX, iPosY);
        if (pCharacter != nullptr)
        {
            CopyWideString(pCharacter->ID, L"Initial Helper");
            pCharacter->Object.Scale = 0.8f;
            pCharacter->Object.HiddenMesh = 2;
            pCharacter->Object.m_iAnimation = 0;
        }
    }

    return pCharacter;
}

bool C09SummerEvent::MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (o == nullptr || o->Type != MODEL_FIRE_FLAME_GHOST)
    {
        return false;
    }

    vec3_t vRelativePos, vWorldPos;
    Vector(0.f, 0.f, 0.f, vRelativePos);
    b->TransformPosition(o->BoneTransform[39], vRelativePos, vWorldPos, true);

    vec3_t vLight;
    const float fLumi = (std::sin(WorldTime * 0.0015f) + 1.0f) * 0.4f + 0.2f;

    Vector(0.8f, 0.4f, 0.2f, vLight);

    CreateSprite(BITMAP_LIGHT, vWorldPos, 2.0f, vLight, o);
    Vector(fLumi * 0.8f, fLumi * 0.4f, fLumi * 0.2f, vLight);
    CreateSprite(BITMAP_LIGHT, vWorldPos, 3.0f, vLight, o);

    switch (o->CurrentAction)
    {
    case MONSTER01_WALK:
    {
        if (o->AnimationFrame >= 1.0f && o->AnimationFrame <= 1.3f)
        {
            if (o->m_iAnimation == 0)
            {
                PlayBuffer(SOUND_UMBRELLA_MONSTER_WALK1);
                o->m_iAnimation = 1;
            }
            else
            {
                PlayBuffer(SOUND_UMBRELLA_MONSTER_WALK2);
                o->m_iAnimation = 0;
            }
        }
    }break;
    case MONSTER01_SHOCK:
    {
        if (o->AnimationFrame >= 0.5f && o->AnimationFrame <= 0.8f)
            PlayBuffer(SOUND_UMBRELLA_MONSTER_DAMAGE);
    }break;
    case MONSTER01_DIE:
    {
        if (o->AnimationFrame > 0.0f && o->AnimationFrame <= 0.3f && rand_fps_check(1))
        {
            CreateEffect(MODEL_EFFECT_SKURA_ITEM, o->Position, o->Angle, o->Light, 1, o);
            vec3_t Position, Angle, Light;
            Position[0] = o->Position[0];
            Position[1] = o->Position[1];
            Position[2] = RequestTerrainHeight(Position[0], Position[1]);
            Vector(0.f, 0.f, 0.f, Angle);
            Vector(1.0f, 1.0f, 1.0f, Light);
            CreateEffect(BITMAP_FIRECRACKER0001, Position, Angle, Light, 0);
            CreateEffect(MODEL_EFFECT_UMBRELLA_DIE, o->Position, o->Angle, o->Light, 0, o);
            for (int i = 0; i < 40; ++i)
            {
                CreateEffect(MODEL_EFFECT_UMBRELLA_GOLD, o->Position, o->Angle, o->Light, 0, o);
            }
        }
        if (o->AnimationFrame >= 0.5f && o->AnimationFrame <= 0.8f)
        {
            PlayBuffer(SOUND_UMBRELLA_MONSTER_DEAD);
        }
    }break;
    }

    return true;
}