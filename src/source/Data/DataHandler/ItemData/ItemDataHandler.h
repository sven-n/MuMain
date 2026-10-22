#pragma once

#include "Data/GameData/ItemData/ItemStructs.h"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifdef _EDITOR
#include "Data/DataHandler/ItemData/ItemBmdImport.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#endif

// Owns loading the item data (Data/Items/*.json) into the item database and
// the item models (Data/Items/Models/*.json) into the item model database, and
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

    // Loads and validates the item models (Data/Items/Models) into the item
    // model database. On errors nothing is changed, false is returned and
    // errorMessage describes the errors for the player.
    bool LoadModels(std::string& errorMessage);

#ifdef _EDITOR
    // The item editor changed ItemAttribute[itemType]; copies the change
    // into the item database. A changed name is stored for the current UI
    // locale.
    void OnItemEdited(int itemType);
    // Sets the item's name in `locale` (an empty text removes the translation;
    // the English name cannot be emptied) in the item database, whole also when
    // it is longer than ITEM_ATTRIBUTE's name; ItemAttribute shows the name in
    // the UI locale. False when the item does not exist, when the English name
    // would be empty (also when the item has none yet) or when the text has
    // LocalizedString::Separator, which the item checks refuse.
    bool SetItemName(int itemType, std::string_view locale, std::string text);
    // The item editor swapped two entries of ItemAttribute[]. A moved item
    // takes its model along (the item model database and the opened model
    // slot), and its weapon kind and stat profile follow its new group like
    // an edit (FollowDerivedFieldsAfterMove).
    void OnItemsSwapped(int firstItemType, int secondItemType);

    // Validates the item database and the item models and writes Data/Items,
    // with the model files of Data/Items/Models whose models were moved since
    // they were loaded. Nothing is written when the data has errors, and no
    // file is replaced when one of them cannot be written.
    Data::Items::ItemDataSaveResult Save(std::vector<Data::Items::ItemDataIssue>& issues);

    // Replaces all items with the legacy Item_<language>.bmd files. Items the
    // English file lacks keep their current English name. The imported items
    // are at their bmd numbers, so the models the editor moved go back to the
    // numbers they were loaded at. Nothing changes when the files cannot be
    // read; problems the imported data still has (Save refuses it while they
    // are errors) are in validationIssues.
    Data::Items::ItemBmdImportResult ImportFromBmd();

    // Writes Item_<language>.bmd for every legacy language, with the names
    // of that language. Files that already have the data are left as they
    // are and do not count as a failure. Items with a value the files cannot
    // hold (damage or magic power above 255) are named in the change log and
    // nothing is written.
    bool ExportAsBmd(std::string& changeLog);

    // Writes the S6E3 legacy file; an item with a value the file cannot hold is named in `problems` instead.
    bool ExportAsS6E3(wchar_t* fileName, std::string& problems);
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
    // An item.bmd has a name for every item, so a missing translation is in it
    // as the English name (Export as bmd writes it so). An imported name equal
    // to the English one stays only where the item has a name of its own in
    // that language now (it is the same there, like Kris); otherwise the name
    // stays missing.
    static void KeepMissingNamesMissing(std::vector<Data::Items::ItemDefinition>& items,
                                        const std::vector<std::string>& importedLocales);
    // Gives imported items the current values of the fields item.bmd does
    // not have (tags, wing tier, weapon kind, stat profile, rule flags) and their current names in the
    // languages whose item.bmd was not read: the ones without one (e.g.
    // German) and the ones whose file is missing. A weapon kind that its group
    // gives follows the imported slot (FollowGroupWeaponKind), and so does the
    // stat profile its data gives (FollowStatProfile) if the item stat file has
    // it; otherwise the item keeps the default and `issues` gets a warning.
    static void KeepFieldsNotInBmd(std::vector<Data::Items::ItemDefinition>& items,
                                   const std::vector<std::string>& importedLocales,
                                   std::vector<Data::Items::ItemDataIssue>& issues);
    // Gives the item a swap moved to newItemType the weapon kind of its new
    // group if it had the one of its old group (FollowGroupWeaponKind), and the
    // stat profile its data then gives if it had the derived one or none
    // (FollowStatProfile), as long as the item stat file has that profile;
    // otherwise the default, with a warning in the log.
    static void FollowDerivedFieldsAfterMove(const Data::Items::ItemDefinition& before, int newItemType);
    // Swaps the models of two item types: their entries in the item model
    // database and their opened model slots.
    void SwapItemModels(int firstItemType, int secondItemType);
    // Swaps the models the editor moved since LoadModels back, last move
    // first.
    void TakeBackModelMoves();

    // The model swaps of the item editor since LoadModels, in order.
    std::vector<std::pair<int, int>> m_modelMoves;
    // The groups of the swapped models: Save writes their model files.
    Data::Items::ItemGroupSet m_movedModelGroups;
#endif

    bool m_localeObserverRegistered = false;

    // Prevent copying
    CItemDataHandler(const CItemDataHandler&) = delete;
    CItemDataHandler& operator=(const CItemDataHandler&) = delete;
};

#define g_ItemDataHandler CItemDataHandler::GetInstance()
