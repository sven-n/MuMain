// ChangeRingManager.cpp: implementation of the CChangeRingManager class.
//////////////////////////////////////////////////////////////////////

// common
#include "stdafx.h"
#include "Render/Textures/ZzzTexture.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzCharacter.h"
#include "GameLogic/Items/ChangeRingManager.h"
#include "Data/DataHandler/LoadData.h"
#include "Character/CharacterManager.h"
#include "GameLogic/Items/ItemCategories.h"

#include <algorithm>
#include <array>

namespace
{
template <typename Container, typename T> constexpr bool Contains(const Container& container, const T& value)
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
    MODEL_HALLOWEEN, MODEL_XMAS_EVENT_CHANGE_GIRL, MODEL_GM_CHARACTER, MODEL_XMAS2008_SNOWMAN,
    MODEL_PANDA,     MODEL_SKELETON_CHANGED,
};

// The panda and skeleton rings may enter Icarus.
bool IsIcarusBanRing(short ringType)
{
    return GameLogic::Items::IsTransformationRingType(ringType) && !GameLogic::Items::IsPandaOrSkeletonType(ringType);
}
} // namespace

CChangeRingManager::CChangeRingManager() {}

CChangeRingManager::~CChangeRingManager() {}

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
    return GameLogic::Items::IsTransformationRingType(RingType);
}

bool CChangeRingManager::CheckMoveMap(short sLeftRingType, short sRightRingType)
{
    return GameLogic::Items::IsTransformationRingType(sLeftRingType) ||
           GameLogic::Items::IsTransformationRingType(sRightRingType);
}

bool CChangeRingManager::CheckBanMoveIcarusMap(short sLeftRingType, short sRightRingType)
{
    return IsIcarusBanRing(sLeftRingType) || IsIcarusBanRing(sRightRingType);
}