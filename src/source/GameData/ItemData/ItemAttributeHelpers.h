#pragma once

#include "ItemStructs.h"
#include "_define.h"
#include "MultiLanguage.h"

// Macro to copy all non-name fields from source to dest
#define COPY_ITEM_ATTRIBUTE_FIELDS(dest, source) \
    do { \
        (dest).TwoHand = (source).TwoHand; \
        (dest).Level = (source).Level; \
        (dest).m_byItemSlot = (source).m_byItemSlot; \
        (dest).m_wSkillIndex = (source).m_wSkillIndex; \
        (dest).Width = (source).Width; \
        (dest).Height = (source).Height; \
        (dest).DamageMin = (source).DamageMin; \
        (dest).DamageMax = (source).DamageMax; \
        (dest).SuccessfulBlocking = (source).SuccessfulBlocking; \
        (dest).Defense = (source).Defense; \
        (dest).MagicDefense = (source).MagicDefense; \
        (dest).WeaponSpeed = (source).WeaponSpeed; \
        (dest).WalkSpeed = (source).WalkSpeed; \
        (dest).Durability = (source).Durability; \
        (dest).MagicDur = (source).MagicDur; \
        (dest).MagicPower = (source).MagicPower; \
        (dest).RequireStrength = (source).RequireStrength; \
        (dest).RequireDexterity = (source).RequireDexterity; \
        (dest).RequireEnergy = (source).RequireEnergy; \
        (dest).RequireVitality = (source).RequireVitality; \
        (dest).RequireCharisma = (source).RequireCharisma; \
        (dest).RequireLevel = (source).RequireLevel; \
        (dest).Value = (source).Value; \
        (dest).iZen = (source).iZen; \
        (dest).AttType = (source).AttType; \
        memcpy((dest).RequireClass, (source).RequireClass, sizeof((source).RequireClass)); \
        memcpy((dest).Resistance, (source).Resistance, sizeof((source).Resistance)); \
    } while(0)

// Helper template to copy from file structure to runtime structure
template<typename TSource>
inline void CopyItemAttributeFromSource(ITEM_ATTRIBUTE& dest, TSource& source)
{
    CMultiLanguage::ConvertFromUtf8(dest.Name, source.Name, MAX_ITEM_NAME);
    COPY_ITEM_ATTRIBUTE_FIELDS(dest, source);
}

// Helper template to copy from runtime structure to file structure
template<typename TDest>
inline void CopyItemAttributeToDestination(TDest& dest, ITEM_ATTRIBUTE& source)
{
    CMultiLanguage::ConvertToUtf8(dest.Name, source.Name, sizeof(dest.Name));
    COPY_ITEM_ATTRIBUTE_FIELDS(dest, source);
}
