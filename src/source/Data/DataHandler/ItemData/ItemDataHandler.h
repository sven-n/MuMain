#pragma once

#include <string>
#include <vector>

class CItemDataHandler
{
public:
    static CItemDataHandler& GetInstance();

    // Data/Local/<language>/Item_<language>.bmd
    static std::wstring GetItemFilePath(const std::wstring& language);

    // Data Operations - delegates to specialized classes
    bool Load(const wchar_t* fileName);

#ifdef _EDITOR
    bool Save(wchar_t* fileName, std::string* outChangeLog = nullptr);
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

    void LoadEnglishNames();
    void RebuildItemDatabase();

    // English item names (UTF-8) for logs, one per item type. Empty while
    // English is the selected language; the loaded names are English then.
    std::vector<std::string> m_englishNames;

    // Prevent copying
    CItemDataHandler(const CItemDataHandler&) = delete;
    CItemDataHandler& operator=(const CItemDataHandler&) = delete;
};

#define g_ItemDataHandler CItemDataHandler::GetInstance()
