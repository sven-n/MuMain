#pragma once

#include "Data/GameData/ItemData/ItemStructs.h"

#include <string>
#include <vector>

class CItemDataHandler
{
public:
    static CItemDataHandler& GetInstance();

    // Data/Local/<language>/Item_<language>.bmd
    static std::wstring GetItemFilePath(const std::wstring& language);

    // Loads Data/Local/<language>/Item_<language>.bmd into ItemAttribute and
    // builds the item database from it.
    bool Load(const std::wstring& language);

#ifdef _EDITOR
    bool Save(const wchar_t* fileName, std::string* outChangeLog = nullptr);
    bool ExportAsS6E3(wchar_t* fileName);
    bool ExportToCsv(wchar_t* fileName);
#endif

    // Data Access
    ITEM_ATTRIBUTE* GetItemAttributes();
    ITEM_ATTRIBUTE* GetItemAttribute(int index);
    int GetItemCount() const;

private:
    CItemDataHandler();
    ~CItemDataHandler() = default;

#ifdef _EDITOR
    void RebuildItemDatabase();

    // English item names (UTF-8) from the last Load, kept to rebuild the
    // database after an editor save. Empty when English was loaded.
    std::vector<std::string> m_englishNames;
#endif

    // Prevent copying
    CItemDataHandler(const CItemDataHandler&) = delete;
    CItemDataHandler& operator=(const CItemDataHandler&) = delete;
};

#define g_ItemDataHandler CItemDataHandler::GetInstance()
