#include "stdafx.h"

#ifdef _EDITOR

#include "ItemDataSaver.h"
#include "ItemDataFileIO.h"
#include "GameData/ItemData/ItemStructs.h"
#include "GameData/ItemData/ItemFieldDefs.h"
#include "Translation/i18n.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"
#include "MultiLanguage.h"
#include "CSChaosCastle.h"
#include <sstream>
#include <memory>

#include "DataHandler/CommonDataSaver.h"
#include "UI/Console/MuEditorConsoleUI.h"
#include "Utilities/StringUtils.h"

// External references
extern ITEM_ATTRIBUTE* ItemAttribute;

// X-Macro helpers for change tracking
#define COMPARE_FIELD_Bool(name, type, arraySize, width) \
    if (oldItem.name != newItem.name) { \
        itemChanges << "  " #name ": " << (oldItem.name ? "true" : "false") << " -> " << (newItem.name ? "true" : "false") << "\n"; \
        changed = true; \
    }

#define COMPARE_FIELD_Byte(name, type, arraySize, width) \
    if (oldItem.name != newItem.name) { \
        itemChanges << "  " #name ": " << (int)oldItem.name << " -> " << (int)newItem.name << "\n"; \
        changed = true; \
    }

#define COMPARE_FIELD_Word(name, type, arraySize, width) \
    if (oldItem.name != newItem.name) { \
        itemChanges << "  " #name ": " << oldItem.name << " -> " << newItem.name << "\n"; \
        changed = true; \
    }

#define COMPARE_FIELD_Int(name, type, arraySize, width) \
    if (oldItem.name != newItem.name) { \
        itemChanges << "  " #name ": " << oldItem.name << " -> " << newItem.name << "\n"; \
        changed = true; \
    }

#define COMPARE_ARRAY_FIELD(nameWithIndex, baseName, index, type, width) \
    if (oldItem.baseName[index] != newItem.baseName[index]) { \
        itemChanges << "  " #nameWithIndex ": " << (int)oldItem.baseName[index] << " -> " << (int)newItem.baseName[index] << "\n"; \
        changed = true; \
    }

bool ItemDataSaver::Save(wchar_t* fileName, std::string* outChangeLog)
{
    const int Size = sizeof(ITEM_ATTRIBUTE_FILE);

    // Load original file for comparison if change log is requested
    std::unique_ptr<ITEM_ATTRIBUTE[]> originalItems;
    if (outChangeLog)
    {
        originalItems = std::make_unique<ITEM_ATTRIBUTE[]>(MAX_ITEM);
        memset(originalItems.get(), 0, sizeof(ITEM_ATTRIBUTE) * MAX_ITEM);

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

            auto origBuffer = ItemDataFileIO::ReadAndDecryptBuffer(fpOrig, readSize, MAX_ITEM, nullptr);
            fclose(fpOrig);

            if (origBuffer)
            {
                ItemDataFileIO::DecryptBuffer(origBuffer.get(), readSize, MAX_ITEM);

                BYTE* pSeek = origBuffer.get();
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
            }
        }
    }

    // Prepare new buffer
    auto Buffer = std::make_unique<BYTE[]>(Size * MAX_ITEM);
    BYTE* pSeek = Buffer.get();

    // Track changes
    std::stringstream changeLog;
    int changeCount = 0;

    // Convert ItemAttribute back to ITEM_ATTRIBUTE_FILE format
    for (int i = 0; i < MAX_ITEM; i++)
    {
        ITEM_ATTRIBUTE_FILE dest;
        memset(&dest, 0, Size);

        CopyItemAttributeToDestination(dest, ItemAttribute[i]);

        // Track changes using X-macros
        if (originalItems && ItemAttribute[i].Name[0] != 0)
        {
            bool changed = false;
            std::stringstream itemChanges;

            ITEM_ATTRIBUTE& oldItem = originalItems[i];
            ITEM_ATTRIBUTE& newItem = ItemAttribute[i];

            // Name (special case - wide char comparison)
            if (wcscmp(oldItem.Name, newItem.Name) != 0)
            {
                char oldName[MAX_ITEM_NAME];
                char newName[MAX_ITEM_NAME];
                CMultiLanguage::ConvertToUtf8(oldName, oldItem.Name, MAX_ITEM_NAME);
                CMultiLanguage::ConvertToUtf8(newName, newItem.Name, MAX_ITEM_NAME);
                itemChanges << "  Name: \"" << oldName << "\" -> \"" << newName << "\"\n";
                changed = true;
            }

            // All other fields using X-macros
            #define COMPARE_SIMPLE(name, type, arraySize, width) COMPARE_FIELD_##type(name, type, arraySize, width)
            ITEM_FIELDS_SIMPLE(COMPARE_SIMPLE)
            #undef COMPARE_SIMPLE

            ITEM_FIELDS_ARRAYS(COMPARE_ARRAY_FIELD)

            if (changed)
            {
                char itemName[MAX_ITEM_NAME];
                CMultiLanguage::ConvertToUtf8(itemName, newItem.Name, MAX_ITEM_NAME);
                changeLog << "[" << i << "] " << itemName << "\n" << itemChanges.str();
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
        if (outChangeLog)
        {
            *outChangeLog = "No changes detected.\n";
        }

        return false; // Return false to indicate no save was needed
    }

    // Create backup
    CommonDataSaver::CreateBackup(fileName);

    // Open file for writing only after we know we need to save
    FILE* fp = _wfopen(fileName, L"wb");
    if (fp == NULL)
    {
        return false;
    }

    // Encrypt buffer
    ItemDataFileIO::EncryptBuffer(Buffer.get(), Size, MAX_ITEM);

    // Write buffer and checksum
    ItemDataFileIO::WriteAndEncryptBuffer(fp, Buffer.get(), Size * MAX_ITEM);

    fclose(fp);

    if (originalItems && outChangeLog && changeCount > 0)
    {
        *outChangeLog = "=== Item Changes (" + std::to_string(changeCount) + " items modified) ===\n" + changeLog.str();
    }

    return true;
}

#endif // _EDITOR
