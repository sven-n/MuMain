#include "stdafx.h"

#ifdef _EDITOR

#include "ItemDataSaver.h"
#include "ItemDataFileIO.h"
#include "GameData/ItemData/ItemAttributeHelpers.h"
#include "Translation/i18n.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"
#include "MultiLanguage.h"
#include "CSChaosCastle.h"
#include <sstream>

// External references
extern ITEM_ATTRIBUTE* ItemAttribute;

bool ItemDataSaver::Save(wchar_t* fileName, std::string* outChangeLog)
{
    const int Size = sizeof(ITEM_ATTRIBUTE_FILE);

    // Load original file for comparison if change log is requested
    ITEM_ATTRIBUTE* originalItems = nullptr;
    if (outChangeLog)
    {
        originalItems = new ITEM_ATTRIBUTE[MAX_ITEM];
        memset(originalItems, 0, sizeof(ITEM_ATTRIBUTE) * MAX_ITEM);

        // Read original file and detect format
        FILE* fpOrig = _wfopen(fileName, L"rb");
        if (fpOrig)
        {
            // Get file size to determine structure version
            fseek(fpOrig, 0, SEEK_END);
            long fileSize = ftell(fpOrig);
            fseek(fpOrig, 0, SEEK_SET);

            const int LegacySize = sizeof(ITEM_ATTRIBUTE_FILE_LEGACY);
            const int NewSize = sizeof(ITEM_ATTRIBUTE_FILE);
            const long expectedLegacySize = LegacySize * MAX_ITEM + sizeof(DWORD);
            const long expectedNewSize = NewSize * MAX_ITEM + sizeof(DWORD);

            bool isLegacyFormat = (fileSize == expectedLegacySize);
            int readSize = isLegacyFormat ? LegacySize : NewSize;

            BYTE* OrigBuffer = ItemDataFileIO::ReadAndDecryptBuffer(fpOrig, readSize, MAX_ITEM, nullptr);
            fclose(fpOrig);

            if (OrigBuffer)
            {
                ItemDataFileIO::DecryptBuffer(OrigBuffer, readSize, MAX_ITEM);

                BYTE* pSeek = OrigBuffer;
                for (int i = 0; i < MAX_ITEM; i++)
                {
                    if (isLegacyFormat)
                    {
                        ITEM_ATTRIBUTE_FILE_LEGACY source;
                        memcpy(&source, pSeek, LegacySize);
                        CopyItemAttributeFromSource(originalItems[i], source);
                    }
                    else
                    {
                        ITEM_ATTRIBUTE_FILE source;
                        memcpy(&source, pSeek, NewSize);
                        CopyItemAttributeFromSource(originalItems[i], source);
                    }

                    pSeek += readSize;
                }
                delete[] OrigBuffer;
            }
        }
    }

    // Prepare new buffer
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

            // Name
            if (wcscmp(originalItems[i].Name, ItemAttribute[i].Name) != 0)
            {
                char oldName[MAX_ITEM_NAME];
                char newName[MAX_ITEM_NAME];
                CMultiLanguage::ConvertToUtf8(oldName, originalItems[i].Name, MAX_ITEM_NAME);
                CMultiLanguage::ConvertToUtf8(newName, ItemAttribute[i].Name, MAX_ITEM_NAME);
                itemChanges << "  Name: \"" << oldName << "\" -> \"" << newName << "\"\n";
                changed = true;
            }

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
                std::string entryHeader = i18n::FormatEditor("log_item_change_entry", {std::to_string(i), std::string(itemName)});
                changeLog << entryHeader << "\n" << itemChanges.str();
                changeCount++;
            }
        }

        memcpy(pSeek, &dest, Size);
        pSeek += Size;
    }

    // Check if we should skip saving when no changes detected
    if (originalItems && changeCount == 0)
    {
        // No changes - skip save
        delete[] Buffer;
        delete[] originalItems;
        
        if (outChangeLog)
        {
            *outChangeLog = std::string(EDITOR_TEXT("log_no_changes_detected")) + "\n";
        }
        
        return false; // Return false to indicate no save was needed
    }

    // Open file for writing only after we know we need to save
    FILE* fp = _wfopen(fileName, L"wb");
    if (fp == NULL)
    {
        delete[] Buffer;
        if (originalItems) delete[] originalItems;
        return false;
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
        if (outChangeLog && changeCount > 0)
        {
            std::string header = i18n::FormatEditor("log_item_changes_header", {std::to_string(changeCount)});
            *outChangeLog = header + "\n" + changeLog.str();
        }
    }

    return true;
}

#endif // _EDITOR
