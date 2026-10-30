#include "stdafx.h"
#include "ItemDataHandler.h"
#include "ItemBmdImport.h"
#include "ItemJsonStorage.h"
#include "ItemModelLoader.h"
#include "Core/Globals/_struct.h"
#include "Core/Globals/_define.h"
#include "Core/Text/Utf8.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/DataIssueReport.h"
#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemEnumNames.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemModelJsonFormat.h"
#include "Data/GameData/ItemData/ItemTypeUse.h"
#include "Data/GameData/ItemData/ItemNames.h"
#include "Engine/Object/ZzzInfomation.h"
#include "I18N/All.h"

#include <algorithm>
#include <iterator>
#include <chrono>
#include <memory>
#include <set>

#ifdef _EDITOR
#include "Data/DataHandler/CommonDataSaver.h"
#include "ItemDataSaver.h"
#include "ItemDataExportS6E3.h"
#include "Data/GameData/ItemData/ItemFileFit.h"
#include "ItemDataExportAsCSV.h"
#endif

// External references
extern ITEM_ATTRIBUTE* ItemAttribute;

using namespace Data::Items;

namespace
{
constexpr std::string_view ItemDataLabel = "Item data";
constexpr std::string_view ItemDataName = "item data";

double MillisecondsSince(std::chrono::steady_clock::time_point start)
{
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
    return elapsed.count();
}

// ITEM_ATTRIBUTE records with the item names of `locale`.
std::unique_ptr<ITEM_ATTRIBUTE[]> BuildItemAttributes(std::string_view locale)
{
    auto attributes = std::make_unique<ITEM_ATTRIBUTE[]>(MAX_ITEM);
    for (const ItemDefinition& slot : g_ItemDatabase.GetAllSlots())
    {
        if (!slot.Exists())
        {
            continue;
        }
        ItemDefinition definition = slot;
        definition.name = Core::Text::FromUtf8(definition.names.Get(locale));
        ToItemAttribute(definition, attributes[MakeItemType(slot.group, slot.number)]);
    }
    return attributes;
}

#ifdef _EDITOR
// The orbs', crystals' and scrolls' requirements as the bmd files hold them, weights
// (ToBmdRequirementWeights), with a line of `notes` for each one no weight gives exactly.
void WriteBmdRequirementWeights(const ItemDefinition& definition, ITEM_ATTRIBUTE& attribute, std::string& notes)
{
    const ItemStatProfiles& profiles = g_ItemDatabase.GetStatProfiles();
    if (!HasRequirementWeightsInBmd(profiles, definition))
    {
        return;
    }
    for (const ItemRequirement requirement : ToBmdRequirementWeights(profiles, definition, attribute))
    {
        notes += g_ItemDatabase.GetLogName(MakeItemType(definition.group, definition.number)) +
                 ": no weight gives its " + FindEnumName(requirement) +
                 " requirement exactly, the nearest one is written\n";
    }
}

// The records as the bmd files hold them: with the item names of `locale`, and the
// requirements WriteBmdRequirementWeights turns into weights.
std::unique_ptr<ITEM_ATTRIBUTE[]> BuildBmdItemAttributes(std::string_view locale, std::string& notes)
{
    auto attributes = BuildItemAttributes(locale);
    for (const ItemDefinition& definition : g_ItemDatabase.GetAllSlots())
    {
        if (definition.Exists())
        {
            WriteBmdRequirementWeights(definition, attributes[MakeItemType(definition.group, definition.number)],
                                       notes);
        }
    }
    return attributes;
}
#endif

#ifdef _EDITOR
// The tables and profiles in the writer's order (by name), so a file read as it
// was written and one the editor stored compare equal (the undo history).
ItemStatFile InWriterOrder(ItemStatFile file)
{
    const auto byName = [](const auto& left, const auto& right) { return left.name < right.name; };
    std::ranges::stable_sort(file.tables, byName);
    std::ranges::stable_sort(file.profiles, byName);
    return file;
}

// The item type before the moves since the start (each one swaps two item types), where the
// bmd files have it.
int TypeBeforeMoves(int itemType, std::span<const std::pair<int, int>> moves)
{
    for (auto move = moves.rbegin(); move != moves.rend(); ++move)
    {
        if (itemType == move->first)
        {
            itemType = move->second;
        }
        else if (itemType == move->second)
        {
            itemType = move->first;
        }
    }
    return itemType;
}

// The bmd files have no entries without a name: the import keeps the ones at numbers it
// brings no item to, so the numbers the client uses stay in the item files. A moved entry
// goes back to its old number, as its model does.
void KeepEntriesWithoutName(std::vector<ItemDefinition>& items, std::span<const std::pair<int, int>> moves)
{
    std::set<int> imported;
    for (const ItemDefinition& item : items)
    {
        imported.insert(MakeItemType(item.group, item.number));
    }
    for (const ItemDefinition& slot : g_ItemDatabase.GetAllSlots())
    {
        const int itemType = TypeBeforeMoves(MakeItemType(slot.group, slot.number), moves);
        if (!slot.entryWithoutName || imported.contains(itemType))
        {
            continue;
        }
        ItemDefinition entry = slot;
        entry.group = GetItemGroup(itemType);
        entry.number = GetItemNumber(itemType);
        items.push_back(std::move(entry));
    }
}

// Every number with a model keeps an entry: where the bmd files have no item for a model (an
// item added or named since the start), its number gets an entry without a name.
void GiveModelsAnEntry()
{
    std::vector<ItemDefinition> entries;
    for (int itemType = 0; itemType < MAX_ITEM; ++itemType)
    {
        if (g_ItemModelDatabase.Find(itemType) == nullptr || g_ItemDatabase.FindEntry(itemType) != nullptr)
        {
            continue;
        }
        ItemDefinition entry;
        entry.group = GetItemGroup(itemType);
        entry.number = GetItemNumber(itemType);
        entry.entryWithoutName = true;
        entries.push_back(std::move(entry));
    }
    if (!entries.empty())
    {
        g_ItemDatabase.SetMany(entries);
    }
}
#endif
} // namespace

CItemDataHandler::CItemDataHandler()
{
}

CItemDataHandler& CItemDataHandler::GetInstance()
{
    static CItemDataHandler instance;
    return instance;
}

ITEM_ATTRIBUTE* CItemDataHandler::GetItemAttributes()
{
    return ItemAttribute;
}

ITEM_ATTRIBUTE* CItemDataHandler::GetItemAttribute(int index)
{
    if (index >= 0 && index < MAX_ITEM)
        return &ItemAttribute[index];
    return nullptr;
}

int CItemDataHandler::GetItemCount() const
{
    return MAX_ITEM;
}

std::wstring CItemDataHandler::GetItemFilePath(const std::wstring& language)
{
    return L"Data\\Local\\" + language + L"\\Item_" + language + L".bmd";
}

bool CItemDataHandler::Load(std::string& errorMessage)
{
    const auto loadStart = std::chrono::steady_clock::now();
    ItemDataLoadResult result = LoadItemDataDirectory(GetItemDataDirectory());
    ValidateNameLocales(result.items, I18N::GetAvailableLocales(), result.issues);
    ItemStatsLoadResult stats = LoadItemStatsFile(GetItemStatsFile());
    result.issues.insert(result.issues.end(), stats.issues.begin(), stats.issues.end());
    ItemTooltipLoadResult tooltips = LoadItemTooltipFile(GetItemTooltipFile());
    ValidateTooltipTextLocales(tooltips.file, I18N::GetAvailableLocales(), GetItemTooltipFile().generic_string(),
                               tooltips.issues);
    result.issues.insert(result.issues.end(), tooltips.issues.begin(), tooltips.issues.end());
    ItemStatProfiles statProfiles;
    if (!HasFatal(stats.issues))
    {
        statProfiles = ItemStatProfiles(stats.file);
        ValidateStatProfiles(result.items, statProfiles, GetItemStatsFile().generic_string(), result.issues);
        if (!HasFatal(tooltips.issues))
        {
            ValidateDescriptionPlaceholders(result.items, statProfiles, tooltips.file,
                                            GetItemStatsFile().generic_string(), result.issues);
            ValidateTableUses(result.items, statProfiles, tooltips.file, GetItemStatsFile().generic_string(),
                              result.issues);
        }
    }
    if (!HasFatal(tooltips.issues))
    {
        ValidateDescriptionTexts(result.items, tooltips.file, GetItemTooltipFile().generic_string(), result.issues);
    }
    Data::LogDataIssues(ItemDataLabel, result.issues);
    if (HasFatal(result.issues))
    {
        errorMessage = Data::DescribeFatalIssues(ItemDataName, GetItemDataDirectory(), result.issues);
        return false;
    }
    const double loadMilliseconds = MillisecondsSince(loadStart);
#ifdef _EDITOR
    m_statFile = InWriterOrder(std::move(stats.file));
    m_statFileChanged = false;
#endif

    const auto buildStart = std::chrono::steady_clock::now();
    g_ItemDatabase.SetDisplayLocale(ItemNameLocale());
    g_ItemDatabase.SetStatProfiles(std::move(statProfiles));
    g_ItemDatabase.SetTooltipFile(std::move(tooltips.file));
    g_ItemDatabase.Build(result.items);
    FillItemAttributes();
    m_loaded = true;
    RegisterLocaleObserver();

    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} items from {} in {:.1f} ms (item database build {:.2f} ms)",
                g_ItemDatabase.GetExistingItemCount(), GetItemDataDirectory().string(), loadMilliseconds,
                MillisecondsSince(buildStart));
    return true;
}

bool CItemDataHandler::LoadModels(std::string& errorMessage)
{
    const auto loadStart = std::chrono::steady_clock::now();
    GlowColorsLoadResult colors = LoadGlowColorsFile(GetGlowColorsFile());
    ItemModelDataLoadResult result = LoadItemModelDataDirectory(GetItemModelDataDirectory());
    result.issues.insert(result.issues.begin(), colors.issues.begin(), colors.issues.end());
    if (!HasFatal(colors.issues))
    {
        ValidateItemModelGlowColors(result.models, colors.colors, result.issues);
    }
    Data::LogDataIssues(ItemDataLabel, result.issues);
    if (HasFatal(result.issues))
    {
        errorMessage = Data::DescribeFatalIssues(ItemDataName, GetItemModelDataDirectory(), result.issues);
        return false;
    }

    g_GlowColors.Build(colors.colors);
    g_ItemModelDatabase.Build(result.models, g_GlowColors);
    g_ItemModelDatabase.SetSharedModels(result.sharedModels);
#ifdef _EDITOR
    // The models are where their files have them again.
    m_modelMoves.clear();
    m_changedModelGroups.reset();
    m_sharedModelsChanged = false;
    m_glowColorsChanged = false;
    m_modelsToReopen.clear();
#endif
    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} item models from {} in {:.1f} ms", g_ItemModelDatabase.GetModelCount(),
                GetItemModelDataDirectory().string(), MillisecondsSince(loadStart));
    return true;
}

void CItemDataHandler::FillItemAttributes()
{
    for (int itemType = 0; itemType < MAX_ITEM; ++itemType)
    {
        const ItemDefinition* definition = g_ItemDatabase.Find(itemType);
        if (definition != nullptr)
        {
            ToItemAttribute(*definition, ItemAttribute[itemType]);
        }
        else
        {
            // Empty slots stay all zero, as they were with item.bmd.
            ItemAttribute[itemType] = ITEM_ATTRIBUTE{};
        }
    }
}

void CItemDataHandler::RegisterLocaleObserver()
{
    if (m_localeObserverRegistered)
    {
        return;
    }
    I18N::RegisterLocaleObserver(&CItemDataHandler::OnLocaleChanged, this);
    m_localeObserverRegistered = true;
}

void CItemDataHandler::OnLocaleChanged(void* context) noexcept
{
    static_cast<CItemDataHandler*>(context)->RefreshItemNames();
}

void CItemDataHandler::SetTranslateItemNames(bool translate)
{
    m_translateItemNames = translate;
    if (m_loaded)
    {
        RefreshItemNames();
    }
}

std::string_view CItemDataHandler::ItemNameLocale() const
{
    return Data::Items::GetItemNameLocale(I18N::GetCurrentLocale(), m_translateItemNames);
}

void CItemDataHandler::RefreshItemNames()
{
    g_ItemDatabase.SetDisplayLocale(ItemNameLocale());
    FillItemAttributes();
}

#ifdef _EDITOR
ItemDefinition CItemDataHandler::WithDerivedFields(const ItemDefinition& definition, const ItemDefinition* previous,
                                                   StatProfileStore profiles)
{
    ItemDefinition result = definition;
    if (previous == nullptr || definition.weaponKind == previous->weaponKind)
    {
        result.weaponKind = FollowGroupWeaponKind(previous, definition);
    }
    if (definition.statProfile.empty())
    {
        // "Follow the data" was picked: also over a profile picked before.
        result.statProfile = FollowStatProfile(g_ItemDatabase.GetStatProfiles(), nullptr, result);
    }
    else if (profiles == StatProfileStore::Follow &&
             (previous == nullptr || definition.statProfile == previous->statProfile))
    {
        result.statProfile = FollowStatProfile(g_ItemDatabase.GetStatProfiles(), previous, result);
    }
    return result;
}

void CItemDataHandler::RefillItemAttribute(int itemType)
{
    if (const ItemDefinition* definition = g_ItemDatabase.Find(itemType))
    {
        ToItemAttribute(*definition, ItemAttribute[itemType]);
        return;
    }
    ItemAttribute[itemType] = ITEM_ATTRIBUTE{};
}

void CItemDataHandler::SetItem(const ItemDefinition& definition)
{
    SetItems(std::span<const ItemDefinition>(&definition, 1));
}

void CItemDataHandler::SetItems(std::span<const ItemDefinition> definitions, StatProfileStore profiles)
{
    std::vector<ItemDefinition> stored;
    stored.reserve(definitions.size());
    for (const ItemDefinition& definition : definitions)
    {
        if (IsValidItemId(definition.group, definition.number))
        {
            stored.push_back(
                WithDerivedFields(definition, g_ItemDatabase.Find(definition.group, definition.number), profiles));
        }
    }
    g_ItemDatabase.SetMany(stored);
    for (const ItemDefinition& definition : stored)
    {
        RefillItemAttribute(MakeItemType(definition.group, definition.number));
    }
}

void CItemDataHandler::RestoreItems(std::span<const ItemDefinition> definitions)
{
    std::vector<ItemDefinition> stored;
    std::ranges::copy_if(definitions, std::back_inserter(stored), [](const ItemDefinition& definition)
                         { return IsValidItemId(definition.group, definition.number); });
    g_ItemDatabase.SetMany(stored);
    for (const ItemDefinition& definition : stored)
    {
        RefillItemAttribute(MakeItemType(definition.group, definition.number));
    }
}

bool CItemDataHandler::AddItem(const ItemDefinition& definition, const ItemModelDefinition* model,
                               const ItemDefinition* copiedFrom)
{
    if (!IsValidItemId(definition.group, definition.number) || !definition.Exists() ||
        !IsFreeItemType(MakeItemType(definition.group, definition.number)))
    {
        return false;
    }
    const ItemDefinition stored = WithDerivedFields(definition, copiedFrom);
    RestoreItems(std::span<const ItemDefinition>(&stored, 1));
    if (model != nullptr)
    {
        ItemModelDefinition placed = *model;
        placed.group = definition.group;
        placed.number = definition.number;
        SetItemModel(placed);
    }
    return true;
}

bool CItemDataHandler::RemoveItem(int itemType)
{
    // An entry without a name goes as an item does.
    if (g_ItemDatabase.FindEntry(itemType) == nullptr)
    {
        return false;
    }
    ItemDefinition empty;
    empty.group = GetItemGroup(itemType);
    empty.number = GetItemNumber(itemType);
    RestoreItems(std::span<const ItemDefinition>(&empty, 1));
    if (g_ItemModelDatabase.Find(itemType) != nullptr)
    {
        ItemModelDefinition noModel;
        noModel.group = empty.group;
        noModel.number = empty.number;
        SetItemModel(noModel);
    }
    return true;
}

bool CItemDataHandler::MoveItem(int fromItemType, int toItemType)
{
    if (g_ItemDatabase.FindEntry(fromItemType) == nullptr || !IsFreeItemType(toItemType))
    {
        return false;
    }
    std::swap(ItemAttribute[fromItemType], ItemAttribute[toItemType]);
    OnItemsSwapped(fromItemType, toItemType);
    return true;
}

void CItemDataHandler::SetItemModel(const ItemModelDefinition& model)
{
    if (!IsValidItemId(model.group, model.number))
    {
        return;
    }

    const int itemType = MakeItemType(model.group, model.number);
    ItemModelDefinition stored = model;
    if (const SharedItemModel* shared = g_ItemModelDatabase.FindSharedModel(stored.model))
    {
        stored.file = shared->file;
        stored.textureFolders = shared->textureFolders;
        stored.noneBlendMeshes = shared->noneBlendMeshes;
    }
    const ItemModelDefinition* before = g_ItemModelDatabase.Find(itemType);
    const bool opensDifferently = before == nullptr || !stored.Exists() || stored.model != before->model ||
                                  stored.file != before->file || stored.textureFolders != before->textureFolders ||
                                  stored.noneBlendMeshes != before->noneBlendMeshes ||
                                  stored.material != before->material;
    if (before == nullptr && !stored.Exists())
    {
        return;
    }

    g_ItemModelDatabase.Set(stored, g_GlowColors);
    m_changedModelGroups.set(static_cast<size_t>(model.group));
    if (opensDifferently && std::ranges::find(m_modelsToReopen, itemType) == m_modelsToReopen.end())
    {
        m_modelsToReopen.push_back(itemType);
    }
}

void CItemDataHandler::SetSharedModel(const SharedItemModel& sharedModel, std::string_view previousName)
{
    const std::string oldName = previousName.empty() ? sharedModel.name : std::string(previousName);
    if (oldName != sharedModel.name)
    {
        g_ItemModelDatabase.RemoveSharedModel(oldName);
    }
    g_ItemModelDatabase.SetSharedModel(sharedModel);
    m_sharedModelsChanged = true;

    for (const ItemModelDefinition& model : g_ItemModelDatabase.GetAllSlots())
    {
        if (model.Exists() && model.model == oldName)
        {
            ItemModelDefinition user = model;
            user.model = sharedModel.name;
            SetItemModel(user);
        }
    }
}

bool CItemDataHandler::RemoveSharedModel(std::string_view name)
{
    const auto slots = g_ItemModelDatabase.GetAllSlots();
    if (std::ranges::any_of(slots,
                            [name](const ItemModelDefinition& model) { return model.Exists() && model.model == name; }))
    {
        return false;
    }
    if (g_ItemModelDatabase.FindSharedModel(name) != nullptr)
    {
        g_ItemModelDatabase.RemoveSharedModel(name);
        m_sharedModelsChanged = true;
    }
    return true;
}

void CItemDataHandler::SetGlowColor(const Data::Effects::GlowColor& color)
{
    g_GlowColors.Set(color);
    m_glowColorsChanged = true;
    g_ItemModelDatabase.ResolveGlowColors(g_GlowColors);
}

bool CItemDataHandler::RemoveGlowColor(std::string_view name)
{
    if (name == ItemGlow::DefaultColor || name == ItemGlow::DefaultShineColor || name == ItemGlow::DefaultAncientColor)
    {
        return false;
    }
    const auto slots = g_ItemModelDatabase.GetAllSlots();
    if (std::ranges::any_of(slots,
                            [name](const ItemModelDefinition& model)
                            {
                                return model.Exists() && (model.glow.color == name || model.glow.shineColor == name ||
                                                          model.glow.ancientColor == name);
                            }))
    {
        return false;
    }
    if (g_GlowColors.Find(name) != nullptr)
    {
        g_GlowColors.Remove(name);
        m_glowColorsChanged = true;
        g_ItemModelDatabase.ResolveGlowColors(g_GlowColors);
    }
    return true;
}

void CItemDataHandler::RestoreGlowColors(std::span<const Data::Effects::GlowColor> colors)
{
    g_GlowColors.Build(colors);
    m_glowColorsChanged = true;
    g_ItemModelDatabase.ResolveGlowColors(g_GlowColors);
}

std::vector<ItemModelProblem> CItemDataHandler::ReopenChangedModels(const ModelLoader::LookNames& lookNames)
{
    std::vector<int> itemTypes;
    itemTypes.swap(m_modelsToReopen);
    std::ranges::sort(itemTypes);
    if (itemTypes.empty())
    {
        return {};
    }
    if (m_openedModelHooks.changing != nullptr)
    {
        m_openedModelHooks.changing();
    }
    std::vector<ItemModelProblem> problems = ModelLoader::ReopenModels(itemTypes, lookNames);
    if (m_openedModelHooks.changed != nullptr)
    {
        m_openedModelHooks.changed(itemTypes);
    }
    return problems;
}

bool CItemDataHandler::SetItemName(int itemType, std::string_view locale, std::string text)
{
    const ItemDefinition* current = g_ItemDatabase.Find(itemType);
    if (current == nullptr || text.find(Data::LocalizedString::Separator) != std::string::npos)
    {
        return false;
    }

    ItemDefinition definition = *current;
    definition.names.Set(locale, std::move(text));
    // Also for an item without an English name (from a bmd import): removing
    // its last translation would end the item in the database while
    // ItemAttribute kept it. Its English name comes first.
    if (definition.names.GetNeutral().empty())
    {
        return false;
    }
    g_ItemDatabase.Set(definition);
    if (const ItemDefinition* updated = g_ItemDatabase.Find(itemType))
    {
        ToItemAttribute(*updated, ItemAttribute[itemType]);
    }
    return true;
}

void CItemDataHandler::OnItemsSwapped(int firstItemType, int secondItemType)
{
    if (!IsValidItemType(firstItemType) || !IsValidItemType(secondItemType))
    {
        return;
    }

    const ItemDefinition first = g_ItemDatabase.GetAllSlots()[firstItemType];
    const ItemDefinition second = g_ItemDatabase.GetAllSlots()[secondItemType];
    g_ItemDatabase.Swap(firstItemType, secondItemType);
    // Each item keeps the model slot of its item type, so the model moves too.
    SwapItemModels(firstItemType, secondItemType);
    m_modelMoves.emplace_back(firstItemType, secondItemType);
    FollowDerivedFieldsAfterMove(first, secondItemType);
    FollowDerivedFieldsAfterMove(second, firstItemType);
}

void CItemDataHandler::SwapItemModels(int firstItemType, int secondItemType)
{
    g_ItemModelDatabase.Swap(firstItemType, secondItemType);
    if (m_openedModelHooks.changing != nullptr)
    {
        m_openedModelHooks.changing();
    }
    Data::Items::ModelLoader::SwapOpenedModels(firstItemType, secondItemType);
    if (m_openedModelHooks.changed != nullptr)
    {
        const int itemTypes[] = {firstItemType, secondItemType};
        // A call through a function pointer: GCC lets the array decay to a
        // pointer instead of making a span of it.
        m_openedModelHooks.changed(std::span<const int>(itemTypes));
    }
    m_changedModelGroups.set(static_cast<size_t>(GetItemGroup(firstItemType)));
    m_changedModelGroups.set(static_cast<size_t>(GetItemGroup(secondItemType)));
}

void CItemDataHandler::TakeBackModelMoves()
{
    for (auto move = m_modelMoves.rbegin(); move != m_modelMoves.rend(); ++move)
    {
        SwapItemModels(move->first, move->second);
    }
    m_modelMoves.clear();
}

void CItemDataHandler::FollowDerivedFieldsAfterMove(const ItemDefinition& before, int newItemType)
{
    const ItemDefinition* moved = g_ItemDatabase.Find(newItemType);
    if (moved == nullptr)
    {
        return;
    }

    ItemDefinition changed = *moved;
    changed.weaponKind = FollowGroupWeaponKind(&before, changed);
    changed.statProfile = FollowStatProfile(g_ItemDatabase.GetStatProfiles(), &before, changed);
    if (changed.weaponKind != moved->weaponKind || changed.statProfile != moved->statProfile)
    {
        g_ItemDatabase.Set(changed);
    }
}

void CItemDataHandler::SetLookNames(LookNameExists renderStyleExists, LookNameExists itemEffectExists)
{
    m_lookNames = LookNames{renderStyleExists, itemEffectExists};
}

void CItemDataHandler::Validate(std::vector<ItemDataIssue>& issues) const
{
    // A profile the item stat file does not have would stop the next start.
    ValidateStatProfiles(g_ItemDatabase.GetAllSlots(), g_ItemDatabase.GetStatProfiles(),
                         GetItemStatsFile().generic_string(), issues);
    ValidateDescriptionPlaceholders(g_ItemDatabase.GetAllSlots(), g_ItemDatabase.GetStatProfiles(),
                                    g_ItemDatabase.GetTooltipFile(), GetItemStatsFile().generic_string(), issues);
    ValidateDescriptionTexts(g_ItemDatabase.GetAllSlots(), g_ItemDatabase.GetTooltipFile(),
                             GetItemTooltipFile().generic_string(), issues);
    ValidateTableUses(g_ItemDatabase.GetAllSlots(), g_ItemDatabase.GetStatProfiles(), g_ItemDatabase.GetTooltipFile(),
                      GetItemStatsFile().generic_string(), issues);
    ValidateNameLocales(g_ItemDatabase.GetAllSlots(), I18N::GetAvailableLocales(), issues);
    std::vector<ItemDefinition> items;
    std::ranges::copy_if(g_ItemDatabase.GetAllSlots(), std::back_inserter(items), &ItemDefinition::HasEntry);
    ValidateItems(items, issues);
    // The file as Load read it was checked then.
    if (m_statFileChanged)
    {
        ValidateItemStatsAsWritten(m_statFile, issues);
    }

    std::vector<ItemModelDefinition> models;
    std::ranges::copy_if(g_ItemModelDatabase.GetAllSlots(), std::back_inserter(models), &ItemModelDefinition::Exists);
    ValidateItemModels(models, issues);
    ValidateModelsHaveItemEntries(g_ItemDatabase.GetAllSlots(), models, issues);
    std::vector<ItemModelDefinition> sharedUsers = models;
    ApplySharedItemModels(sharedUsers, g_ItemModelDatabase.GetSharedModels(), SharedItemModelsFileName, issues);
    // Without a color list (no models loaded) every color would be missing.
    if (!g_GlowColors.GetColors().empty())
    {
        ValidateItemModelGlowColors(models, g_GlowColors.GetColors(), issues);
        ValidateGlowColorsAsWritten(g_GlowColors.GetColors(), issues);
    }
    if (m_lookNames)
    {
        ValidateItemModelLooks(models, m_lookNames->renderStyle, m_lookNames->itemEffect, issues);
    }
    ValidateDirectlySharedModelFiles(models, issues);
    ValidateItemModelsAsWritten(g_ItemModelDatabase.GetAllSlots(), issues);
    ValidateSharedItemModelsAsWritten(g_ItemModelDatabase.GetSharedModels(), issues);
    ValidateItemModelFilesExist(models, issues);
}

ItemDataSaveResult CItemDataHandler::Save(std::vector<ItemDataIssue>& issues)
{
    // Everything is checked before anything is written, so all problems are
    // reported at once.
    Validate(issues);
    if (HasFatal(issues))
    {
        return ItemDataSaveResult::InvalidData;
    }

    // Only the model files of changed models are written; the others keep what
    // this client does not read from them.
    ItemModelSave models;
    models.directory = GetItemModelDataDirectory();
    models.models = g_ItemModelDatabase.GetAllSlots();
    models.groups = m_changedModelGroups;
    models.sharedModels = g_ItemModelDatabase.GetSharedModels();
    models.sharedModelsChanged = m_sharedModelsChanged;
    models.glowColorsFile = GetGlowColorsFile();
    models.glowColors = g_GlowColors.GetColors();
    models.glowColorsChanged = m_glowColorsChanged;
    models.statsFile = GetItemStatsFile();
    models.stats = &m_statFile;
    models.statsChanged = m_statFileChanged;
    return WriteItemAndModelDataDirectories(GetItemDataDirectory(), g_ItemDatabase.GetAllSlots(), models, issues);
}

void CItemDataHandler::SetStatFile(const ItemStatFile& file)
{
    m_statFile = InWriterOrder(file);
    m_statFileChanged = true;
    g_ItemDatabase.SetStatProfiles(ItemStatProfiles(m_statFile));
}

ItemBmdImportResult CItemDataHandler::ImportFromBmd()
{
    ItemBmdImportResult result = ImportItemBmdFiles();
    if (HasFatal(result.issues))
    {
        return result;
    }

    result.keptEnglishNameCount = KeepCurrentEnglishNames(result.items);
    KeepMissingNamesMissing(result.items, result.importedLocales);
    KeepFieldsNotInBmd(result.items, result.importedLocales);
    // With its stat profile known, a requirement the item keeps as a value takes the value the
    // bmd file's weight gave in the original client.
    for (ItemDefinition& item : result.items)
    {
        ToRequirementValues(g_ItemDatabase.GetStatProfiles(), item);
    }
    ValidateItems(result.items, result.validationIssues);
    ValidateStatProfiles(result.items, g_ItemDatabase.GetStatProfiles(), GetItemStatsFile().generic_string(),
                         result.validationIssues);
    ValidateDescriptionPlaceholders(result.items, g_ItemDatabase.GetStatProfiles(), g_ItemDatabase.GetTooltipFile(),
                                    GetItemStatsFile().generic_string(), result.validationIssues);
    ValidateDescriptionTexts(result.items, g_ItemDatabase.GetTooltipFile(), GetItemTooltipFile().generic_string(),
                             result.validationIssues);
    ValidateTableUses(result.items, g_ItemDatabase.GetStatProfiles(), g_ItemDatabase.GetTooltipFile(),
                      GetItemStatsFile().generic_string(), result.validationIssues);
    ValidateNameLocales(result.items, I18N::GetAvailableLocales(), result.validationIssues);

    // The entries without a name stay as they are: they are not from the bmd files, so they
    // are not among the imported items.
    std::vector<ItemDefinition> items = result.items;
    KeepEntriesWithoutName(items, m_modelMoves);
    g_ItemDatabase.Build(items);
    FillItemAttributes();
    // The bmd files have the items at their old numbers; their models go back
    // there too.
    TakeBackModelMoves();
    GiveModelsAnEntry();
    return result;
}

int CItemDataHandler::KeepCurrentEnglishNames(std::vector<ItemDefinition>& items)
{
    int keptCount = 0;
    for (ItemDefinition& item : items)
    {
        const ItemDefinition* current = g_ItemDatabase.Find(item.group, item.number);
        if (!item.names.GetNeutral().empty() || current == nullptr || current->names.GetNeutral().empty())
        {
            continue;
        }
        item.names.Set(Data::LocalizedString::NeutralLocale, current->names.GetNeutral());
        ++keptCount;
    }
    return keptCount;
}

void CItemDataHandler::KeepMissingNamesMissing(std::vector<ItemDefinition>& items,
                                               const std::vector<std::string>& importedLocales)
{
    for (ItemDefinition& item : items)
    {
        const ItemDefinition* current = g_ItemDatabase.Find(item.group, item.number);
        for (const std::string& locale : importedLocales)
        {
            if (locale == Data::LocalizedString::NeutralLocale ||
                Data::Items::GetOwnName(item, locale) != item.names.GetNeutral())
            {
                continue;
            }
            if (current == nullptr || !Data::Items::HasNameIn(*current, locale))
            {
                item.names.Set(locale, "");
            }
        }
    }
}

void CItemDataHandler::KeepFieldsNotInBmd(std::vector<ItemDefinition>& items,
                                          const std::vector<std::string>& importedLocales)
{
    const auto imported = [&](std::string_view locale)
    { return std::find(importedLocales.begin(), importedLocales.end(), locale) != importedLocales.end(); };
    for (ItemDefinition& item : items)
    {
        const ItemDefinition* current = g_ItemDatabase.Find(item.group, item.number);
        if (current != nullptr)
        {
            CopyFieldsNotInItemAttribute(*current, item);
            for (const auto& [locale, text] : current->names.GetTranslations())
            {
                if (!imported(locale))
                {
                    item.names.Set(locale, text);
                }
            }
        }
        item.weaponKind = FollowGroupWeaponKind(current, item);
        item.statProfile = FollowStatProfile(g_ItemDatabase.GetStatProfiles(), current, item);
    }
}

bool CItemDataHandler::ExportAsBmd(std::string& changeLog)
{
    // The values are the same in every language: check them once.
    const std::vector<ItemFileMisfit> misfits =
        FindItemFileMisfits(BuildItemAttributes(GetItemBmdLanguages().front().locale).get(), MAX_ITEM);
    if (!misfits.empty())
    {
        changeLog += DescribeItemFileMisfits(misfits);
        return false;
    }

    bool success = true;
    // The same for every language.
    std::string weightNotes;
    for (const ItemBmdLanguage& language : GetItemBmdLanguages())
    {
        weightNotes.clear();
        const auto attributes = BuildBmdItemAttributes(language.locale, weightNotes);
        std::string languageChangeLog;
        const bool saved =
            ItemDataSaver::Save(GetItemFilePath(language.folder).c_str(), attributes.get(), &languageChangeLog);
        // An unchanged file is not a failure.
        const bool unchanged = !saved && languageChangeLog == CommonDataSaver::NoChangesMessage;
        success = (saved || unchanged) && success;
        changeLog += languageChangeLog;
    }
    changeLog += weightNotes;
    return success;
}

bool CItemDataHandler::ExportAsS6E3(wchar_t* fileName, std::string& problems, std::string& notes)
{
    return ItemDataExportS6E3::SaveLegacy(fileName, BuildBmdItemAttributes(I18N::GetCurrentLocale(), notes).get(),
                                          &problems);
}

bool CItemDataHandler::ExportToCsv(wchar_t* fileName)
{
    return ItemDataExportAsCSV::ExportToCsv(fileName, BuildItemAttributes(I18N::GetCurrentLocale()).get());
}
#endif
