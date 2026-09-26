#include "stdafx.h"
#include "ItemDataHandler.h"
#include "ItemJsonStorage.h"
#include "Core/Globals/_struct.h"
#include "Core/Globals/_define.h"
#include "Core/Text/Utf8.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Engine/Object/ZzzInfomation.h"
#include "I18N/All.h"

#include <chrono>
#include <memory>

#ifdef _EDITOR
#include "Data/DataHandler/CommonDataSaver.h"
#include "ItemBmdLanguages.h"
#include "ItemDataSaver.h"
#include "ItemDataExportS6E3.h"
#include "ItemDataExportAsCSV.h"
#endif

// External references
extern ITEM_ATTRIBUTE* ItemAttribute;

using namespace Data::Items;

namespace
{
// How many errors the player sees; all of them go to the log.
constexpr size_t MaxErrorsInMessage = 10;

double MillisecondsSince(std::chrono::steady_clock::time_point start)
{
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - start;
    return elapsed.count();
}

void LogIssues(const std::vector<ItemDataIssue>& issues)
{
    const auto logger = mu::log::Get("data");
    for (const ItemDataIssue& issue : issues)
    {
        if (issue.severity == ItemDataIssueSeverity::Error)
        {
            MU_LOG_ERROR(logger, "Item data {}", issue.ToString());
        }
        else
        {
            MU_LOG_WARN(logger, "Item data {}", issue.ToString());
        }
    }
}

std::string DescribeErrors(const std::vector<ItemDataIssue>& issues)
{
    std::string message = "The item data in " + GetItemDataDirectory().string() + " has errors:\n";
    size_t errorCount = 0;
    for (const ItemDataIssue& issue : issues)
    {
        if (issue.severity != ItemDataIssueSeverity::Error)
        {
            continue;
        }
        if (++errorCount <= MaxErrorsInMessage)
        {
            message += "\n" + issue.ToString();
        }
    }
    if (errorCount > MaxErrorsInMessage)
    {
        message += "\n... and " + std::to_string(errorCount - MaxErrorsInMessage) + " more";
    }
    return message + "\n\nAll problems are listed in MuError.log.";
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
    LogIssues(result.issues);
    if (HasErrors(result.issues))
    {
        errorMessage = DescribeErrors(result.issues);
        return false;
    }
    const double loadMilliseconds = MillisecondsSince(loadStart);

    const auto buildStart = std::chrono::steady_clock::now();
    g_ItemDatabase.SetDisplayLocale(I18N::GetCurrentLocale());
    g_ItemDatabase.Build(result.items);
    FillItemAttributes();
    RegisterLocaleObserver();

    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} items from {} in {:.1f} ms (item database build {:.2f} ms)",
                g_ItemDatabase.GetExistingItemCount(), GetItemDataDirectory().string(), loadMilliseconds,
                MillisecondsSince(buildStart));
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

void CItemDataHandler::OnItemsSwapped(int firstItemType, int secondItemType)
{
    g_ItemDatabase.Swap(firstItemType, secondItemType);
}

ItemDataSaveResult CItemDataHandler::Save(std::vector<ItemDataIssue>& issues)
{
    return SaveItemDataDirectory(GetItemDataDirectory(), g_ItemDatabase.GetAllSlots(), issues);
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
