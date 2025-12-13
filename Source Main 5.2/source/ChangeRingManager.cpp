// ChangeRingManager.cpp: implementation of the CChangeRingManager class.
//////////////////////////////////////////////////////////////////////

// common
#include "stdafx.h"
#include "ZzzTexture.h"
#include "ZzzOpenData.h"
#include "ZzzBMD.h"
#include "ZzzCharacter.h"
#include "ChangeRingManager.h"
#include "LoadData.h"
#include "CharacterManager.h"

#include <algorithm>
#include <array>

namespace
{
    template <typename Container, typename T>
    constexpr bool Contains(const Container& container, const T& value)
    {
        for (const auto& item : container)
        {
            if (item == value)
            {
                return true;
            }
        }
        return false;
    }

    constexpr std::array<int, 12> kDarkLordHairModels{
        MODEL_SKELETON1,
        MODEL_SKELETON2,
        MODEL_SKELETON3,
        MODEL_SKELETON_PCBANG,
        MODEL_HALLOWEEN,
        MODEL_XMAS_EVENT_CHANGE_GIRL,
        MODEL_GM_CHARACTER,
        MODEL_CURSEDTEMPLE_ALLIED_PLAYER,
        MODEL_CURSEDTEMPLE_ILLUSION_PLAYER,
        MODEL_XMAS2008_SNOWMAN,
        MODEL_PANDA,
        MODEL_SKELETON_CHANGED,
    };

    constexpr std::array<int, 6> kDarkCloakModels{
        MODEL_HALLOWEEN,
        MODEL_XMAS_EVENT_CHANGE_GIRL,
        MODEL_GM_CHARACTER,
        MODEL_XMAS2008_SNOWMAN,
        MODEL_PANDA,
        MODEL_SKELETON_CHANGED,
    };

    constexpr std::array<short, 8> kChangeRingTypes{
        ITEM_TRANSFORMATION_RING,
        ITEM_ELITE_TRANSFER_SKELETON_RING,
        ITEM_JACK_OLANTERN_TRANSFORMATION_RING,
        ITEM_CHRISTMAS_TRANSFORMATION_RING,
        ITEM_GAME_MASTER_TRANSFORMATION_RING,
        ITEM_SNOWMAN_TRANSFORMATION_RING,
        ITEM_PANDA_TRANSFORMATION_RING,
        ITEM_SKELETON_TRANSFORMATION_RING,
    };

    constexpr std::array<short, 6> kIcarusBanRingTypes{
        ITEM_TRANSFORMATION_RING,
        ITEM_ELITE_TRANSFER_SKELETON_RING,
        ITEM_JACK_OLANTERN_TRANSFORMATION_RING,
        ITEM_CHRISTMAS_TRANSFORMATION_RING,
        ITEM_GAME_MASTER_TRANSFORMATION_RING,
        ITEM_SNOWMAN_TRANSFORMATION_RING,
    };
}

CChangeRingManager::CChangeRingManager()
{
}

CChangeRingManager::~CChangeRingManager()
{
}

void CChangeRingManager::LoadItemModel()
{
    gLoadData.AccessModel(MODEL_TRANSFORMATION_RING, L"Data\\Item\\", L"Ring", 1);
    gLoadData.AccessModel(MODEL_ELITE_TRANSFER_SKELETON_RING, L"Data\\Item\\", L"Ring", 1);
    gLoadData.AccessModel(MODEL_JACK_OLANTERN_TRANSFORMATION_RING, L"Data\\Item\\", L"Ring", 1);
    gLoadData.AccessModel(MODEL_CHRISTMAS_TRANSFORMATION_RING, L"Data\\Item\\", L"Ring", 1);
    gLoadData.AccessModel(MODEL_GAME_MASTER_TRANSFORMATION_RING, L"Data\\Item\\", L"Ring", 1);
    gLoadData.AccessModel(MODEL_SNOWMAN_TRANSFORMATION_RING, L"Data\\Item\\xmas\\", L"xmasring");
    gLoadData.AccessModel(MODEL_PANDA_TRANSFORMATION_RING, L"Data\\Item\\", L"PandaPetRing");
    gLoadData.AccessModel(MODEL_SKELETON_TRANSFORMATION_RING, L"Data\\Item\\", L"SkeletonRing");
}

void CChangeRingManager::LoadItemTexture()
{
    gLoadData.OpenTexture(MODEL_ELITE_TRANSFER_SKELETON_RING, L"Item\\");
    gLoadData.OpenTexture(MODEL_CHRISTMAS_TRANSFORMATION_RING, L"Item\\");
    gLoadData.OpenTexture(MODEL_JACK_OLANTERN_TRANSFORMATION_RING, L"Item\\");
    gLoadData.OpenTexture(MODEL_GAME_MASTER_TRANSFORMATION_RING, L"Item\\");
    gLoadData.OpenTexture(MODEL_SNOWMAN_TRANSFORMATION_RING, L"Item\\xmas\\");
    gLoadData.OpenTexture(MODEL_PANDA_TRANSFORMATION_RING, L"Item\\");
    gLoadData.OpenTexture(MODEL_SKELETON_TRANSFORMATION_RING, L"Item\\");
}

bool CChangeRingManager::CheckDarkLordHair(int iType)
{
    return Contains(kDarkLordHairModels, iType);
}

bool CChangeRingManager::CheckDarkCloak(CLASS_TYPE iClass, int iType)
{
    if (iClass != CLASS_DARK)
        return false;

    return Contains(kDarkCloakModels, iType);
}

bool CChangeRingManager::CheckChangeRing(short RingType)
{
    return Contains(kChangeRingTypes, RingType);
}

bool CChangeRingManager::CheckRepair(int iType)
{
    return Contains(kChangeRingTypes, iType);
}

bool CChangeRingManager::CheckMoveMap(short sLeftRingType, short sRightRingType)
{
    return Contains(kChangeRingTypes, sLeftRingType) || Contains(kChangeRingTypes, sRightRingType);
}

bool CChangeRingManager::CheckBanMoveIcarusMap(short sLeftRingType, short sRightRingType)
{
    return Contains(kIcarusBanRingTypes, sLeftRingType) || Contains(kIcarusBanRingTypes, sRightRingType);
}