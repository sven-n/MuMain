#include "stdafx.h"

#ifdef _EDITOR

#include "ItemDataSaver.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"
#include "MultiLanguage.h"
#include "CSChaosCastle.h"
#include <sstream>

// External references
extern ITEM_ATTRIBUTE* ItemAttribute;

// Helper template to copy from runtime structure to file structure
template<typename TDest>
static void CopyItemAttributeToDestination(TDest& dest, ITEM_ATTRIBUTE& source)
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

bool ItemDataSaver::Save(wchar_t* fileName, std::string* outChangeLog)
{
    const int Size = sizeof(ITEM_ATTRIBUTE_FILE);

    // Load original file for comparison if change log is requested
    ITEM_ATTRIBUTE* originalItems = nullptr;
    if (outChangeLog)
    {
        originalItems = new ITEM_ATTRIBUTE[MAX_ITEM];
        memset(originalItems, 0, sizeof(ITEM_ATTRIBUTE) * MAX_ITEM);

        // Read original file
        FILE* fpOrig = _wfopen(fileName, L"rb");
        if (fpOrig)
        {
            BYTE* OrigBuffer = ItemDataFileIO::ReadAndDecryptBuffer(fpOrig, Size, MAX_ITEM, nullptr);
            fclose(fpOrig);

            if (OrigBuffer)
            {
                ItemDataFileIO::DecryptBuffer(OrigBuffer, Size, MAX_ITEM);

                BYTE* pSeek = OrigBuffer;
                for (int i = 0; i < MAX_ITEM; i++)
                {
                    // Use field-by-field copy to avoid alignment issues
                    ITEM_ATTRIBUTE_FILE source;
                    memcpy(&source, pSeek, Size);

                    // Copy all fields individually (same approach as loader)
                    CMultiLanguage::ConvertFromUtf8(originalItems[i].Name, source.Name, MAX_ITEM_NAME);
                    originalItems[i].TwoHand = source.TwoHand;
                    originalItems[i].Level = source.Level;
                    originalItems[i].m_byItemSlot = source.m_byItemSlot;
                    originalItems[i].m_wSkillIndex = source.m_wSkillIndex;
                    originalItems[i].Width = source.Width;
                    originalItems[i].Height = source.Height;
                    originalItems[i].DamageMin = source.DamageMin;
                    originalItems[i].DamageMax = source.DamageMax;
                    originalItems[i].SuccessfulBlocking = source.SuccessfulBlocking;
                    originalItems[i].Defense = source.Defense;
                    originalItems[i].MagicDefense = source.MagicDefense;
                    originalItems[i].WeaponSpeed = source.WeaponSpeed;
                    originalItems[i].WalkSpeed = source.WalkSpeed;
                    originalItems[i].Durability = source.Durability;
                    originalItems[i].MagicDur = source.MagicDur;
                    originalItems[i].MagicPower = source.MagicPower;
                    originalItems[i].RequireStrength = source.RequireStrength;
                    originalItems[i].RequireDexterity = source.RequireDexterity;
                    originalItems[i].RequireEnergy = source.RequireEnergy;
                    originalItems[i].RequireVitality = source.RequireVitality;
                    originalItems[i].RequireCharisma = source.RequireCharisma;
                    originalItems[i].RequireLevel = source.RequireLevel;
                    originalItems[i].Value = source.Value;
                    originalItems[i].iZen = source.iZen;
                    originalItems[i].AttType = source.AttType;
                    memcpy(originalItems[i].RequireClass, source.RequireClass, sizeof(source.RequireClass));
                    memcpy(originalItems[i].Resistance, source.Resistance, sizeof(source.Resistance));

                    pSeek += Size;
                }
                delete[] OrigBuffer;
            }
        }
    }

    // Prepare new buffer
    FILE* fp = _wfopen(fileName, L"wb");
    if (fp == NULL)
    {
        if (originalItems) delete[] originalItems;
        return false;
    }

    BYTE* Buffer = new BYTE[Size * MAX_ITEM];
    BYTE* pSeek = Buffer;

    // Track changes
    std::stringstream changeLog;
    int changeCount = 0;

    // Convert ItemAttribute back to ITEM_ATTRIBUTE_FILE format
    for (int i = 0; i < MAX_ITEM; i++)
    {
        ITEM_ATTRIBUTE_FILE dest;
        memset(&dest, 0, Size);

        CopyItemAttributeToDestination(dest, ItemAttribute[i]);

        // Track changes - now including ALL fields
        if (originalItems && ItemAttribute[i].Name[0] != 0)
        {
            bool changed = false;
            std::stringstream itemChanges;

            // Basic properties
            if (originalItems[i].TwoHand != ItemAttribute[i].TwoHand)
            {
                itemChanges << "  TwoHand: " << (originalItems[i].TwoHand ? "true" : "false") << " -> " << (ItemAttribute[i].TwoHand ? "true" : "false") << "\n";
                changed = true;
            }
            if (originalItems[i].Level != ItemAttribute[i].Level)
            {
                itemChanges << "  Level: " << originalItems[i].Level << " -> " << ItemAttribute[i].Level << "\n";
                changed = true;
            }
            if (originalItems[i].m_byItemSlot != ItemAttribute[i].m_byItemSlot)
            {
                itemChanges << "  Slot: " << (int)originalItems[i].m_byItemSlot << " -> " << (int)ItemAttribute[i].m_byItemSlot << "\n";
                changed = true;
            }
            if (originalItems[i].m_wSkillIndex != ItemAttribute[i].m_wSkillIndex)
            {
                itemChanges << "  Skill: " << originalItems[i].m_wSkillIndex << " -> " << ItemAttribute[i].m_wSkillIndex << "\n";
                changed = true;
            }

            // Dimensions
            if (originalItems[i].Width != ItemAttribute[i].Width)
            {
                itemChanges << "  Width: " << (int)originalItems[i].Width << " -> " << (int)ItemAttribute[i].Width << "\n";
                changed = true;
            }
            if (originalItems[i].Height != ItemAttribute[i].Height)
            {
                itemChanges << "  Height: " << (int)originalItems[i].Height << " -> " << (int)ItemAttribute[i].Height << "\n";
                changed = true;
            }

            // Combat stats
            if (originalItems[i].DamageMin != ItemAttribute[i].DamageMin)
            {
                itemChanges << "  DmgMin: " << (int)originalItems[i].DamageMin << " -> " << (int)ItemAttribute[i].DamageMin << "\n";
                changed = true;
            }
            if (originalItems[i].DamageMax != ItemAttribute[i].DamageMax)
            {
                itemChanges << "  DmgMax: " << (int)originalItems[i].DamageMax << " -> " << (int)ItemAttribute[i].DamageMax << "\n";
                changed = true;
            }
            if (originalItems[i].SuccessfulBlocking != ItemAttribute[i].SuccessfulBlocking)
            {
                itemChanges << "  Block: " << (int)originalItems[i].SuccessfulBlocking << " -> " << (int)ItemAttribute[i].SuccessfulBlocking << "\n";
                changed = true;
            }
            if (originalItems[i].Defense != ItemAttribute[i].Defense)
            {
                itemChanges << "  Def: " << (int)originalItems[i].Defense << " -> " << (int)ItemAttribute[i].Defense << "\n";
                changed = true;
            }
            if (originalItems[i].MagicDefense != ItemAttribute[i].MagicDefense)
            {
                itemChanges << "  MagDef: " << (int)originalItems[i].MagicDefense << " -> " << (int)ItemAttribute[i].MagicDefense << "\n";
                changed = true;
            }
            if (originalItems[i].WeaponSpeed != ItemAttribute[i].WeaponSpeed)
            {
                itemChanges << "  AtkSpd: " << (int)originalItems[i].WeaponSpeed << " -> " << (int)ItemAttribute[i].WeaponSpeed << "\n";
                changed = true;
            }
            if (originalItems[i].WalkSpeed != ItemAttribute[i].WalkSpeed)
            {
                itemChanges << "  WalkSpd: " << (int)originalItems[i].WalkSpeed << " -> " << (int)ItemAttribute[i].WalkSpeed << "\n";
                changed = true;
            }

            // Durability
            if (originalItems[i].Durability != ItemAttribute[i].Durability)
            {
                itemChanges << "  Dur: " << (int)originalItems[i].Durability << " -> " << (int)ItemAttribute[i].Durability << "\n";
                changed = true;
            }
            if (originalItems[i].MagicDur != ItemAttribute[i].MagicDur)
            {
                itemChanges << "  MagDur: " << (int)originalItems[i].MagicDur << " -> " << (int)ItemAttribute[i].MagicDur << "\n";
                changed = true;
            }
            if (originalItems[i].MagicPower != ItemAttribute[i].MagicPower)
            {
                itemChanges << "  MagPow: " << (int)originalItems[i].MagicPower << " -> " << (int)ItemAttribute[i].MagicPower << "\n";
                changed = true;
            }

            // Requirements
            if (originalItems[i].RequireStrength != ItemAttribute[i].RequireStrength)
            {
                itemChanges << "  Str: " << originalItems[i].RequireStrength << " -> " << ItemAttribute[i].RequireStrength << "\n";
                changed = true;
            }
            if (originalItems[i].RequireDexterity != ItemAttribute[i].RequireDexterity)
            {
                itemChanges << "  Dex: " << originalItems[i].RequireDexterity << " -> " << ItemAttribute[i].RequireDexterity << "\n";
                changed = true;
            }
            if (originalItems[i].RequireEnergy != ItemAttribute[i].RequireEnergy)
            {
                itemChanges << "  Ene: " << originalItems[i].RequireEnergy << " -> " << ItemAttribute[i].RequireEnergy << "\n";
                changed = true;
            }
            if (originalItems[i].RequireVitality != ItemAttribute[i].RequireVitality)
            {
                itemChanges << "  Vit: " << originalItems[i].RequireVitality << " -> " << ItemAttribute[i].RequireVitality << "\n";
                changed = true;
            }
            if (originalItems[i].RequireCharisma != ItemAttribute[i].RequireCharisma)
            {
                itemChanges << "  Cha: " << originalItems[i].RequireCharisma << " -> " << ItemAttribute[i].RequireCharisma << "\n";
                changed = true;
            }
            if (originalItems[i].RequireLevel != ItemAttribute[i].RequireLevel)
            {
                itemChanges << "  Lvl: " << originalItems[i].RequireLevel << " -> " << ItemAttribute[i].RequireLevel << "\n";
                changed = true;
            }

            // Other attributes
            if (originalItems[i].Value != ItemAttribute[i].Value)
            {
                itemChanges << "  Value: " << (int)originalItems[i].Value << " -> " << (int)ItemAttribute[i].Value << "\n";
                changed = true;
            }
            if (originalItems[i].iZen != ItemAttribute[i].iZen)
            {
                itemChanges << "  Zen: " << originalItems[i].iZen << " -> " << ItemAttribute[i].iZen << "\n";
                changed = true;
            }
            if (originalItems[i].AttType != ItemAttribute[i].AttType)
            {
                itemChanges << "  AttType: " << (int)originalItems[i].AttType << " -> " << (int)ItemAttribute[i].AttType << "\n";
                changed = true;
            }

            // Class requirements
            for (int c = 0; c < MAX_CLASS; c++)
            {
                if (originalItems[i].RequireClass[c] != ItemAttribute[i].RequireClass[c])
                {
                    const char* classNames[] = {"DW/SM", "DK/BK", "ELF/ME", "MG", "DL", "SUM", "RF"};
                    itemChanges << "  " << classNames[c] << ": " << (int)originalItems[i].RequireClass[c] << " -> " << (int)ItemAttribute[i].RequireClass[c] << "\n";
                    changed = true;
                }
            }

            // Resistance
            for (int r = 0; r < MAX_RESISTANCE + 1; r++)
            {
                if (originalItems[i].Resistance[r] != ItemAttribute[i].Resistance[r])
                {
                    itemChanges << "  Res[" << r << "]: " << (int)originalItems[i].Resistance[r] << " -> " << (int)ItemAttribute[i].Resistance[r] << "\n";
                    changed = true;
                }
            }

            if (changed)
            {
                char itemName[MAX_ITEM_NAME];
                CMultiLanguage::ConvertToUtf8(itemName, ItemAttribute[i].Name, MAX_ITEM_NAME);
                changeLog << "[" << i << "] " << itemName << "\n" << itemChanges.str();
                changeCount++;
            }
        }

        memcpy(pSeek, &dest, Size);
        pSeek += Size;
    }

    // Encrypt buffer
    ItemDataFileIO::EncryptBuffer(Buffer, Size, MAX_ITEM);

    // Write buffer and checksum
    ItemDataFileIO::WriteAndEncryptBuffer(fp, Buffer, Size * MAX_ITEM);

    fclose(fp);
    delete[] Buffer;

    if (originalItems)
    {
        delete[] originalItems;
        if (outChangeLog)
        {
            if (changeCount > 0)
            {
                *outChangeLog = "=== Item Changes (" + std::to_string(changeCount) + " items modified) ===\n" + changeLog.str();
            }
            else
            {
                *outChangeLog = "No changes detected.\n";
            }
        }
    }

    return true;
}

#endif // _EDITOR
