#pragma once

#include "Data/GameData/ItemData/ItemStructs.h"

#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#ifdef _EDITOR
#include "Data/DataHandler/ItemData/ItemBmdImport.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/DataHandler/ItemData/ItemModelLoader.h"
#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemDataValidation.h"

#include <optional>
#endif

// Owns loading the item data (Data/Items/*.json) into the item database and
// the item models (Data/Items/Models/*.json) into the item model database, and
// keeps ItemAttribute[], the table most game code still reads, filled from
// the database. Item names follow the UI locale, or stay English while
// translated item names are off (SetTranslateItemNames).
class CItemDataHandler
{
public:
    static CItemDataHandler& GetInstance();

    // Whether item names follow the UI locale; off shows them in English. The
    // game sets it from [UI] TranslateItemNames at start and when the option
    // changes. Once items are loaded, the names change at once.
    void SetTranslateItemNames(bool translate);
    bool GetTranslateItemNames() const
    {
        return m_translateItemNames;
    }

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
    // Puts the item into the item database at its (group, number) and refills
    // its row of ItemAttribute[]: the item in the item name language, or an empty row
    // when it no longer exists (no names). The weapon kind and the stat
    // profile follow the data (FollowGroupWeaponKind, FollowStatProfile)
    // unless the definition changes them itself: then they are the user's pick.
    // An empty stat profile is no pick: it follows the data.
    void SetItem(const Data::Items::ItemDefinition& definition);
    // Like SetItem for several items, with the database lists built once; AsGiven keeps the stat
    // profile each item names as a pick.
    void SetItems(std::span<const Data::Items::ItemDefinition> definitions,
                  Data::Items::StatProfileStore profiles = Data::Items::StatProfileStore::Follow);
    // Stores the definitions exactly as they are, nothing derived (the undo of
    // the item tools puts back what an edit stored), and refills their rows of
    // ItemAttribute[].
    void RestoreItems(std::span<const Data::Items::ItemDefinition> definitions);

    // Adds the item at its (group, number), and its model entry when `model`
    // has one (its group and number are the item's). False when the item type
    // is not free (Data::Items::IsFreeItemType) or the item has no names.
    // `copiedFrom` is the item a copy was made of (Duplicate, or Add with the
    // selected item's values): a weapon kind or stat profile picked there stays
    // a pick; otherwise they follow the new item's data.
    bool AddItem(const Data::Items::ItemDefinition& definition, const Data::Items::ItemModelDefinition* model,
                 const Data::Items::ItemDefinition* copiedFrom = nullptr);
    // Removes the item and its model entry; the item type is free again. False
    // when there is no item.
    bool RemoveItem(int itemType);
    // Moves the item, with its ItemAttribute row and its model, to a free item
    // type (OnItemsSwapped). False when there is no item or the target is not
    // free.
    bool MoveItem(int fromItemType, int toItemType);

    // Sets the model entry at its (group, number); an entry without a file and
    // a shared model removes it. An entry that names a shared model takes its
    // file, texture folders and none-blend meshes. Save writes the group's
    // model file; a change to what is opened (file, folders, meshes, material,
    // shared model) is opened again by ReopenChangedModels.
    void SetItemModel(const Data::Items::ItemModelDefinition& model);
    // Adds or replaces a shared model; `previousName` renames the one of that
    // name, and its items name the new one. The items that use it take its
    // file, texture folders and none-blend meshes, and are opened again.
    void SetSharedModel(const Data::Items::SharedItemModel& sharedModel, std::string_view previousName = {});
    // Removes a shared model that no item uses; false while one does.
    bool RemoveSharedModel(std::string_view name);
    // Adds a glow color or changes the value of the one of its name; the
    // items that use it glow with it. Save writes Data/Effects/GlowColors.json.
    void SetGlowColor(const Data::Effects::GlowColor& color);
    // Removes a glow color no item uses; false while one does and for the
    // default colors (ItemGlow), which items without colors of their own use.
    bool RemoveGlowColor(std::string_view name);
    // Makes the glow color list this one (an undo).
    void RestoreGlowColors(std::span<const Data::Effects::GlowColor> colors);
    // Opens the models changed since the last call again (ModelLoader::ReopenModels).
    // For between frames: the editor calls it before the game draws.
    std::vector<Data::Items::ItemModelProblem>
    ReopenChangedModels(const Data::Items::ModelLoader::LookNames& lookNames);
    bool HasModelsToReopen() const
    {
        return !m_modelsToReopen.empty();
    }
    // What keeps or rebuilds data of opened model slots (the cloth of worn
    // pants, the cloth grid of the Grand Soul pants) is game code, so the
    // editor gives it; without hooks nothing runs.
    struct OpenedModelHooks
    {
        // Before the opened data of model slots changes: a move swaps it
        // (OnItemsSwapped, TakeBackModelMoves), and ReopenChangedModels opens
        // it again.
        void (*changing)() = nullptr;
        // After it changed, with the item types whose slots took other data.
        void (*changed)(std::span<const int> itemTypes) = nullptr;
    };
    void SetOpenedModelHooks(const OpenedModelHooks& hooks)
    {
        m_openedModelHooks = hooks;
    }

    // Sets the item's name in `locale` (an empty text removes the translation;
    // the English name cannot be emptied) in the item database, whole also when
    // it is longer than ITEM_ATTRIBUTE's name; ItemAttribute shows the name in
    // the item name language. False when the item does not exist, when the English name
    // would be empty (also when the item has none yet) or when the text has
    // LocalizedString::Separator, which the item checks refuse.
    bool SetItemName(int itemType, std::string_view locale, std::string text);

    // The item stat file as loaded, with the changes of the item editor.
    const Data::Items::ItemStatFile& GetStatFile() const
    {
        return m_statFile;
    }
    // Takes `file` as the item stat file, with its tables and profiles in the
    // writer's order (by name), and gives the item database its profiles; Save
    // writes it. `file` must be valid (ValidateItemStatsAsWritten). Items keep
    // the profile they name.
    void SetStatFile(const Data::Items::ItemStatFile& file);
    // The item editor swapped two entries of ItemAttribute[]. A moved item
    // takes its model along (the item model database and the opened model
    // slot), and its weapon kind and stat profile follow its new group like
    // an edit (FollowDerivedFieldsAfterMove).
    void OnItemsSwapped(int firstItemType, int secondItemType);

    // The checks of the item data and the item models that Save runs, without
    // writing anything: unique ids and names, stat profiles, name languages,
    // shared model names, glow colors, look names (once SetLookNames gave
    // them), the values the model file reader takes, model files several
    // items name themselves and model files that are missing. Appends the
    // problems to `issues`.
    void Validate(std::vector<Data::Items::ItemDataIssue>& issues) const;
    // The looks the checks know; they are drawing code, so the editor gives
    // them. Without them render style and item effect names are not checked.
    void SetLookNames(Data::Items::LookNameExists renderStyleExists, Data::Items::LookNameExists itemEffectExists);

    // Validates the item database and the item models (Validate) and writes Data/Items,
    // with the model files of Data/Items/Models whose models changed (moved,
    // edited) since they were loaded, SharedModels.json when a shared model
    // changed and Stats/ItemStats.json when the item stat file changed. Nothing is written when the data has errors,
    // and no file is replaced when one of them cannot be written.
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

    // Writes the S6E3 legacy file, with the item names of the UI locale, also while translated item names are off;
    // an item with a value the file cannot hold is named in `problems` instead. `notes` gets a line for each
    // requirement the bmd cannot hold exactly (ToBmdRequirementWeights); so does ExportAsBmd's change log.
    bool ExportAsS6E3(wchar_t* fileName, std::string& problems, std::string& notes);
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
    // The language of the item names: the UI locale, or English while
    // translated item names are off.
    std::string_view ItemNameLocale() const;
    // Shows the item names in that language.
    void RefreshItemNames();
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
    // stat profile the profile rules give (FollowStatProfile).
    static void KeepFieldsNotInBmd(std::vector<Data::Items::ItemDefinition>& items,
                                   const std::vector<std::string>& importedLocales);
    // Gives the item a swap moved to newItemType the weapon kind of its new
    // group if it had the one of its old group (FollowGroupWeaponKind), and the
    // stat profile its data then gives if it had the derived one or none
    // (FollowStatProfile).
    static void FollowDerivedFieldsAfterMove(const Data::Items::ItemDefinition& before, int newItemType);
    // Swaps the models of two item types: their entries in the item model
    // database and their opened model slots.
    void SwapItemModels(int firstItemType, int secondItemType);
    // Swaps the models the editor moved since LoadModels back, last move
    // first.
    void TakeBackModelMoves();
    // The definition SetItem stores: the weapon kind and the stat profile
    // follow the data unless `definition` changes them from `previous` (the
    // item as it is, or the one a new item copies; nullptr: none), or the
    // profile is AsGiven.
    static Data::Items::ItemDefinition
    WithDerivedFields(const Data::Items::ItemDefinition& definition, const Data::Items::ItemDefinition* previous,
                      Data::Items::StatProfileStore profiles = Data::Items::StatProfileStore::Follow);
    // ItemAttribute[itemType] from the item database: the item, or all zero.
    static void RefillItemAttribute(int itemType);

    // The model swaps of the item editor since LoadModels, in order.
    std::vector<std::pair<int, int>> m_modelMoves;
    // The groups of the models the editor moved or changed since LoadModels:
    // Save writes their model files.
    Data::Items::ItemGroupSet m_changedModelGroups;
    // Whether the editor changed a shared model since LoadModels.
    bool m_sharedModelsChanged = false;
    // Whether the editor changed the glow color list since LoadModels.
    bool m_glowColorsChanged = false;
    // The item stat file as Load read it, and whether the editor changed it since.
    Data::Items::ItemStatFile m_statFile;
    bool m_statFileChanged = false;
    // The item types whose opened models no longer match their model entries.
    std::vector<int> m_modelsToReopen;
    OpenedModelHooks m_openedModelHooks;

    struct LookNames
    {
        Data::Items::LookNameExists renderStyle;
        Data::Items::LookNameExists itemEffect;
    };
    std::optional<LookNames> m_lookNames;
#endif

    bool m_localeObserverRegistered = false;
    bool m_translateItemNames = true;
    // Load succeeded once: there are item names to show.
    bool m_loaded = false;

    // Prevent copying
    CItemDataHandler(const CItemDataHandler&) = delete;
    CItemDataHandler& operator=(const CItemDataHandler&) = delete;
};

#define g_ItemDataHandler CItemDataHandler::GetInstance()
