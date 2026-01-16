#include "stdafx.h"

#ifdef _EDITOR

#include "ItemDataSaverLegacy.h"
#include "ItemDataFileIO.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"
#include "MultiLanguage.h"

// External references
extern ITEM_ATTRIBUTE* ItemAttribute;

// Helper template to copy from runtime structure to legacy file structure
template<typename TDest>
static void CopyItemAttributeToLegacyDestination(TDest& dest, ITEM_ATTRIBUTE& source)
{
    CMultiLanguage::ConvertToUtf8(dest.Name, source.Name, sizeof(dest.Name));
    dest.TwoHand = source.TwoHand;
    dest.Level = source.Level;
    dest.m_byItemSlot = source.m_byItemSlot;
    dest.m_wSkillIndex = source.m_wSkillIndex;
    dest.Width = source.Width;
    dest.Height = source.Height;
    dest.DamageMin = source.DamageMin;
    dest.DamageMax = source.DamageMax;
    dest.SuccessfulBlocking = source.SuccessfulBlocking;
    dest.Defense = source.Defense;
    dest.MagicDefense = source.MagicDefense;
    dest.WeaponSpeed = source.WeaponSpeed;
    dest.WalkSpeed = source.WalkSpeed;
    dest.Durability = source.Durability;
    dest.MagicDur = source.MagicDur;
    dest.MagicPower = source.MagicPower;
    dest.RequireStrength = source.RequireStrength;
    dest.RequireDexterity = source.RequireDexterity;
    dest.RequireEnergy = source.RequireEnergy;
    dest.RequireVitality = source.RequireVitality;
    dest.RequireCharisma = source.RequireCharisma;
    dest.RequireLevel = source.RequireLevel;
    dest.Value = source.Value;
    dest.iZen = source.iZen;
    dest.AttType = source.AttType;
    memcpy(dest.RequireClass, source.RequireClass, sizeof(dest.RequireClass));
    memcpy(dest.Resistance, source.Resistance, sizeof(dest.Resistance));
}

bool ItemDataSaverLegacy::SaveLegacy(wchar_t* fileName)
{
    const int Size = sizeof(ITEM_ATTRIBUTE_FILE_LEGACY);

    FILE* fp = _wfopen(fileName, L"wb");
    if (fp == NULL)
    {
        return false;
    }

    BYTE* Buffer = new BYTE[Size * MAX_ITEM];
    BYTE* pSeek = Buffer;

    // Convert ItemAttribute to ITEM_ATTRIBUTE_FILE_LEGACY format
    for (int i = 0; i < MAX_ITEM; i++)
    {
        ITEM_ATTRIBUTE_FILE_LEGACY dest;
        memset(&dest, 0, Size);

        CopyItemAttributeToLegacyDestination(dest, ItemAttribute[i]);

        memcpy(pSeek, &dest, Size);
        pSeek += Size;
    }

    // Encrypt buffer
    ItemDataFileIO::EncryptBuffer(Buffer, Size, MAX_ITEM);

    // Write buffer and checksum
    ItemDataFileIO::WriteAndEncryptBuffer(fp, Buffer, Size * MAX_ITEM);

    fclose(fp);
    delete[] Buffer;

    return true;
}

#endif // _EDITOR
