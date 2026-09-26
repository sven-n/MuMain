#pragma once

#include "Data/GameData/ItemData/ItemStructs.h"

#include <string>
#include <vector>

#ifdef _EDITOR
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#endif

// Owns loading the item data (Data/Items/*.json) into the item database and
// keeps ItemAttribute[], the table most game code still reads, filled from
// the database. Item names follow the UI locale.
class CItemDataHandler
{
public:
    static CItemDataHandler& GetInstance();

    // Data/Local/<language>/Item_<language>.bmd, written by Export as bmd.
    static std::wstring GetItemFilePath(const std::wstring& language);

    // Loads and validates Data/Items, builds the item database and fills
    // ItemAttribute[]. On errors nothing is changed, false is returned and
    // errorMessage describes the errors for the player.
    bool Load(std::string& errorMessage);

#ifdef _EDITOR
    // The item editor changed ItemAttribute[itemType]; copies the change
    // into the item database. A changed name is stored for the current UI
    // locale.
    void OnItemEdited(int itemType);
    // The item editor swapped two entries of ItemAttribute[].
    void OnItemsSwapped(int firstItemType, int secondItemType);

    // Validates the item database and writes Data/Items. Nothing is written
    // when the data has errors.
    Data::Items::ItemDataSaveResult Save(std::vector<Data::Items::ItemDataIssue>& issues);

    // Writes Item_<language>.bmd for every legacy language, with the names
    // of that language. Files that already have the data are left as they
    // are and do not count as a failure.
    bool ExportAsBmd(std::string& changeLog);

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

    void FillItemAttributes();
    void RegisterLocaleObserver();
    static void OnLocaleChanged(void* context) noexcept;

    bool m_localeObserverRegistered = false;

    // Prevent copying
    CItemDataHandler(const CItemDataHandler&) = delete;
    CItemDataHandler& operator=(const CItemDataHandler&) = delete;
};

#define g_ItemDataHandler CItemDataHandler::GetInstance()
