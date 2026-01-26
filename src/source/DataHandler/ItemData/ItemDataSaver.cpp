#include "stdafx.h"

#ifdef _EDITOR

#include "ItemDataSaver.h"
#include "GameData/ItemData/ItemStructs.h"
#include "Translation/i18n.h"
#include "_struct.h"
#include "_define.h"
#include "ZzzInfomation.h"
#include "MultiLanguage.h"
#include "CSChaosCastle.h"
#include <sstream>
#include <memory>

#include "DataHandler/CommonDataSaver.h"
#include "DataHandler/FieldMetadata.h"
#include "DataHandler/ItemData/ItemComparisonMetadata.h"
#include "UI/Console/MuEditorConsoleUI.h"
#include "Utilities/StringUtils.h"

// External references
extern ITEM_ATTRIBUTE* ItemAttribute;

// Comparison function - uses field metadata (NO MACROS!)
static void CompareItems(const ITEM_ATTRIBUTE& oldItem, const ITEM_ATTRIBUTE& newItem,
                        std::stringstream& changes, bool& changed)
{
    // Compare Name field (wide string)
    if (wcscmp(oldItem.Name, newItem.Name) != 0)
    {
        char oldUtf8[256];
        char newUtf8[256];
        WideCharToMultiByte(CP_UTF8, 0, oldItem.Name, -1, oldUtf8, sizeof(oldUtf8), NULL, NULL);
        WideCharToMultiByte(CP_UTF8, 0, newItem.Name, -1, newUtf8, sizeof(newUtf8), NULL, NULL);

        changes << "  Name: \"" << oldUtf8 << "\" -> \"" << newUtf8 << "\"\n";
        changed = true;
    }

    // Compare all simple fields - just a loop, no macros!
    CompareAllFieldsByMetadata(oldItem, newItem,
                               ItemComparisonMetadata::SIMPLE_FIELDS,
                               changes, changed);

    // Compare all array fields - another simple loop!
    CompareAllFieldsByMetadata(oldItem, newItem,
                               ItemComparisonMetadata::ARRAY_FIELDS,
                               changes, changed);
}

bool ItemDataSaver::Save(wchar_t* fileName, std::string* outChangeLog)
{
    // Create standard save config with item-specific parameters
    auto config = CreateStandardSaveConfig<ITEM_ATTRIBUTE, ITEM_ATTRIBUTE_FILE>(
        fileName,
        MAX_ITEM,
        ItemAttribute,
        [](ITEM_ATTRIBUTE_FILE& dest, ITEM_ATTRIBUTE& src) {
            CopyItemAttributeToDestination(dest, src);
        },
        [](ITEM_ATTRIBUTE& dest, ITEM_ATTRIBUTE_FILE& src) {
            CopyItemAttributeFromSource(dest, src);
        },
        CompareItems,
        [](int index, const ITEM_ATTRIBUTE& item) {
            return ChangeTracker::GetNameUtf8(index, item, MAX_ITEM_NAME);
        },
        0xE2F1,  // Item-specific checksum key
        outChangeLog
    );

    // Add legacy format support for backwards compatibility with old Item.bmd files
    config.legacyFileStructSize = sizeof(ITEM_ATTRIBUTE_FILE_LEGACY);
    config.convertFromFileLegacy = [](ITEM_ATTRIBUTE& dest, BYTE* buffer, size_t size) {
        ITEM_ATTRIBUTE_FILE_LEGACY legacyStruct;
        memcpy(&legacyStruct, buffer, sizeof(legacyStruct));
        CopyItemAttributeFromSource(dest, legacyStruct);
    };

    // Use generic save method
    bool result = CommonDataSaver::SaveData(config);

    // Adjust change log header
    if (result && outChangeLog && !outChangeLog->empty())
    {
        size_t pos = outChangeLog->find("=== Changes");
        if (pos != std::string::npos)
        {
            outChangeLog->replace(pos, 11, "=== Item Changes");
        }
    }

    return result;
}

#endif // _EDITOR
