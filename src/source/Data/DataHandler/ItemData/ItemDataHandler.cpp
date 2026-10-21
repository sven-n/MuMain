#include "stdafx.h"
#include "ItemDataHandler.h"
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
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemNames.h"
#include "Engine/Object/ZzzInfomation.h"
#include "I18N/All.h"

#include <algorithm>
#include <chrono>
#include <memory>

#ifdef _EDITOR
#include "Data/DataHandler/CommonDataSaver.h"
#include "ItemDataSaver.h"
#include "ItemDataExportS6E3.h"
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
// The stat profile FollowStatProfile gives, if the item stat file has it. One the file
// lacks (renamed or removed) would stop the next start, and the editor cannot change it,
// so the item keeps the default and `issues` gets a warning; loading warns about the item
// until an edit, a move or a bmd import gives it the profile, once the file has it again.
std::string FollowKnownStatProfile(const ItemDefinition* previous, const ItemDefinition& updated,
                                   std::vector<ItemDataIssue>& issues)
{
    std::string profile = FollowStatProfile(previous, updated);
    if (g_ItemDatabase.GetStatProfiles().Find(profile) != ItemStatProfiles::UnknownProfile)
    {
        return profile;
    }
    issues.push_back({ItemDataIssueSeverity::Warning, "", updated.group, updated.number, "statProfile",
                      "its data gives \"" + profile + "\", which " + GetItemStatsFile().generic_string() +
                          " does not have; it keeps the default"});
    return "";
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
    ItemStatProfiles statProfiles;
    if (!HasErrors(stats.issues))
    {
        statProfiles = ItemStatProfiles(stats.file);
        ValidateStatProfiles(result.items, statProfiles, GetItemStatsFile().generic_string(), result.issues);
        ValidateDescriptionPlaceholders(result.items, statProfiles, GetItemStatsFile().generic_string(), result.issues);
    }
    ItemTooltipLoadResult tooltips = LoadItemTooltipFile(GetItemTooltipFile());
    result.issues.insert(result.issues.end(), tooltips.issues.begin(), tooltips.issues.end());
    Data::LogDataIssues(ItemDataLabel, result.issues);
    if (HasErrors(result.issues))
    {
        errorMessage = Data::DescribeDataErrors(ItemDataName, GetItemDataDirectory(), result.issues);
        return false;
    }
    const double loadMilliseconds = MillisecondsSince(loadStart);

    const auto buildStart = std::chrono::steady_clock::now();
    g_ItemDatabase.SetDisplayLocale(I18N::GetCurrentLocale());
    g_ItemDatabase.SetStatProfiles(std::move(statProfiles));
    g_ItemDatabase.SetTooltipFile(std::move(tooltips.file));
    g_ItemDatabase.Build(result.items);
    FillItemAttributes();
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
    if (!HasErrors(colors.issues))
    {
        ValidateItemModelGlowColors(result.models, colors.colors, result.issues);
    }
    Data::LogDataIssues(ItemDataLabel, result.issues);
    if (HasErrors(result.issues))
    {
        errorMessage = Data::DescribeDataErrors(ItemDataName, GetItemModelDataDirectory(), result.issues);
        return false;
    }

    g_GlowColors.Build(colors.colors);
    g_ItemModelDatabase.Build(result.models, g_GlowColors);
#ifdef _EDITOR
    // The models are where their files have them again.
    m_modelMoves.clear();
    m_movedModelGroups.reset();
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
    auto* handler = static_cast<CItemDataHandler*>(context);
    g_ItemDatabase.SetDisplayLocale(I18N::GetCurrentLocale());
    handler->FillItemAttributes();
}

#ifdef _EDITOR
void CItemDataHandler::OnItemEdited(int itemType)
{
    if (!IsValidItemType(itemType))
    {
        return;
    }

    ITEM_ATTRIBUTE& attribute = ItemAttribute[itemType];
    ItemDefinition definition = g_ItemDatabase.GetAllSlots()[itemType];
    definition.group = GetItemGroup(itemType);
    definition.number = GetItemNumber(itemType);
    CopyItemAttributeStats(attribute, definition);
    // The editor does not show the weapon kind or the stat profile.
    definition.weaponKind = FollowGroupWeaponKind(g_ItemDatabase.Find(itemType), definition);
    std::vector<ItemDataIssue> issues;
    definition.statProfile = FollowKnownStatProfile(g_ItemDatabase.Find(itemType), definition, issues);
    Data::LogDataIssues(ItemDataLabel, issues);

    // Only a changed name is stored; otherwise every stat edit would turn the
    // shown English fallback into a translation. ITEM_ATTRIBUTE holds a cut
    // name, so compare against the cut name, or a stat edit would store the
    // cut version of a long name.
    const std::wstring editedName = ReadItemAttributeName(attribute);
    if (editedName != CutToItemAttributeName(definition.name))
    {
        const std::string editedNameUtf8 = Core::Text::ToUtf8(editedName.c_str());
        definition.names.Set(g_ItemDatabase.GetDisplayLocale(), editedNameUtf8);
        if (definition.names.GetNeutral().empty())
        {
            // A new item needs an English name.
            definition.names.Set(Data::LocalizedString::NeutralLocale, editedNameUtf8);
        }
    }

    g_ItemDatabase.Set(definition);

    // A removed translation shows the English name again.
    const ItemDefinition* updated = g_ItemDatabase.Find(itemType);
    if (updated != nullptr && CutToItemAttributeName(updated->name) != editedName)
    {
        ToItemAttribute(*updated, attribute);
    }
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
    Data::Items::ModelLoader::SwapOpenedModels(firstItemType, secondItemType);
    m_movedModelGroups.set(static_cast<size_t>(GetItemGroup(firstItemType)));
    m_movedModelGroups.set(static_cast<size_t>(GetItemGroup(secondItemType)));
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
    std::vector<ItemDataIssue> issues;
    changed.statProfile = FollowKnownStatProfile(&before, changed, issues);
    Data::LogDataIssues(ItemDataLabel, issues);
    if (changed.weaponKind != moved->weaponKind || changed.statProfile != moved->statProfile)
    {
        g_ItemDatabase.Set(changed);
    }
}

ItemDataSaveResult CItemDataHandler::Save(std::vector<ItemDataIssue>& issues)
{
    // A profile the item stat file does not have would stop the next start. The save checks
    // the items too and refuses on any error, so everything is reported at once.
    ValidateStatProfiles(g_ItemDatabase.GetAllSlots(), g_ItemDatabase.GetStatProfiles(),
                         GetItemStatsFile().generic_string(), issues);
    ValidateDescriptionPlaceholders(g_ItemDatabase.GetAllSlots(), g_ItemDatabase.GetStatProfiles(),
                                    GetItemStatsFile().generic_string(), issues);
    ValidateNameLocales(g_ItemDatabase.GetAllSlots(), I18N::GetAvailableLocales(), issues);
    // Only the model files of moved models are written; the others keep what
    // this client does not read from them.
    return SaveItemAndModelDataDirectories(GetItemDataDirectory(), g_ItemDatabase.GetAllSlots(),
                                           GetItemModelDataDirectory(), g_ItemModelDatabase.GetAllSlots(),
                                           m_movedModelGroups, issues);
}

ItemBmdImportResult CItemDataHandler::ImportFromBmd()
{
    ItemBmdImportResult result = ImportItemBmdFiles();
    if (HasErrors(result.issues))
    {
        return result;
    }

    result.keptEnglishNameCount = KeepCurrentEnglishNames(result.items);
    KeepMissingNamesMissing(result.items, result.importedLocales);
    KeepFieldsNotInBmd(result.items, result.importedLocales, result.validationIssues);
    ValidateItems(result.items, result.validationIssues);
    ValidateStatProfiles(result.items, g_ItemDatabase.GetStatProfiles(), GetItemStatsFile().generic_string(),
                         result.validationIssues);
    ValidateDescriptionPlaceholders(result.items, g_ItemDatabase.GetStatProfiles(), GetItemStatsFile().generic_string(),
                                    result.validationIssues);
    ValidateNameLocales(result.items, I18N::GetAvailableLocales(), result.validationIssues);

    g_ItemDatabase.Build(result.items);
    FillItemAttributes();
    // The bmd files have the items at their old numbers; their models go back
    // there too.
    TakeBackModelMoves();
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
                                          const std::vector<std::string>& importedLocales,
                                          std::vector<ItemDataIssue>& issues)
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
        item.statProfile = FollowKnownStatProfile(current, item, issues);
    }
}

bool CItemDataHandler::ExportAsBmd(std::string& changeLog)
{
    bool success = true;
    for (const ItemBmdLanguage& language : GetItemBmdLanguages())
    {
        const auto attributes = BuildItemAttributes(language.locale);
        std::string languageChangeLog;
        const bool saved =
            ItemDataSaver::Save(GetItemFilePath(language.folder).c_str(), attributes.get(), &languageChangeLog);
        // An unchanged file is not a failure.
        const bool unchanged = !saved && languageChangeLog == CommonDataSaver::NoChangesMessage;
        success = (saved || unchanged) && success;
        changeLog += languageChangeLog;
    }
    return success;
}

bool CItemDataHandler::ExportAsS6E3(wchar_t* fileName)
{
    return ItemDataExportS6E3::SaveLegacy(fileName);
}

bool CItemDataHandler::ExportToCsv(wchar_t* fileName)
{
    return ItemDataExportAsCSV::ExportToCsv(fileName);
}
#endif
