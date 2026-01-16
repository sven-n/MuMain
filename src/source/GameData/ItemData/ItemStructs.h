#pragma once

#include <Windows.h>

// Forward declarations for constants
#ifndef MAX_CLASS
#define MAX_CLASS 7
#endif

#ifndef MAX_RESISTANCE
#define MAX_RESISTANCE 7
#endif

#ifndef MAX_ITEM_NAME
#define MAX_ITEM_NAME 50
#endif

// Macro to define item attribute fields (excludes Name field which varies)
#define ITEM_ATTRIBUTE_FIELDS \
    bool TwoHand; \
    WORD Level; \
    BYTE m_byItemSlot; \
    WORD m_wSkillIndex; \
    BYTE Width; \
    BYTE Height; \
    BYTE DamageMin; \
    BYTE DamageMax; \
    BYTE SuccessfulBlocking; \
    BYTE Defense; \
    BYTE MagicDefense; \
    BYTE WeaponSpeed; \
    BYTE WalkSpeed; \
    BYTE Durability; \
    BYTE MagicDur; \
    BYTE MagicPower; \
    WORD RequireStrength; \
    WORD RequireDexterity; \
    WORD RequireEnergy; \
    WORD RequireVitality; \
    WORD RequireCharisma; \
    WORD RequireLevel; \
    BYTE Value; \
    int  iZen; \
    BYTE AttType; \
    BYTE RequireClass[MAX_CLASS]; \
    BYTE Resistance[MAX_RESISTANCE + 1];

// Legacy file format structure (S6E3) with 31-byte(maybe 30byte is the saver bet) name
// Used for backward compatibility with old BMD files
typedef struct
{
    char Name[31];
    ITEM_ATTRIBUTE_FIELDS
} ITEM_ATTRIBUTE_FILE_LEGACY;

// Current file format structure with MAX_ITEM_NAME byte name
// Used for reading/writing BMD files
typedef struct
{
    char Name[MAX_ITEM_NAME];
    ITEM_ATTRIBUTE_FIELDS
} ITEM_ATTRIBUTE_FILE;

// Runtime structure with wide-character name
// Used in-memory during gameplay
typedef struct
{
    wchar_t Name[MAX_ITEM_NAME];
    ITEM_ATTRIBUTE_FIELDS
} ITEM_ATTRIBUTE;
