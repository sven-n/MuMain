#pragma once

#include "Data/GameData/ItemData/ItemStructs.h"

#include <string>
#include <vector>

#ifdef _EDITOR
#include "Data/DataHandler/ItemData/ItemBmdImport.h"
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

    // Data/Local/<language>/Item_<language>.bmd (legacy item files).
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

    // Replaces all items with the legacy Item_<language>.bmd files. Items the
    // English file lacks keep their current English name. Nothing changes
    // when the files cannot be read; problems the imported data still has
    // (Save refuses it while they are errors) are in validationIssues.
    Data::Items::ItemBmdImportResult ImportFromBmd();

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

#ifdef _EDITOR
    // Gives imported items without an English name the one the item has now.
    // Returns how many names were kept.
    static int KeepCurrentEnglishNames(std::vector<Data::Items::ItemDefinition>& items);
    // Gives imported items the current values of the fields item.bmd does
    // not have (tags, wing tier, rule flags).
    static void KeepFieldsNotInBmd(std::vector<Data::Items::ItemDefinition>& items);
#endif

    bool m_localeObserverRegistered = false;

    // Prevent copying
    CItemDataHandler(const CItemDataHandler&) = delete;
    CItemDataHandler& operator=(const CItemDataHandler&) = delete;
};

#define g_ItemDataHandler CItemDataHandler::GetInstance()
