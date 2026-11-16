#include "stdafx.h"

#include "BuffStorage.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/DataHandler/DataIssueReport.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/BuffData/BuffDatabase.h"
#include "Data/GameData/BuffData/BuffsJson.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Core/Utilities/StringUtils.h"
#include "I18N/All.h"

#include <algorithm>
#include <chrono>

namespace Data::Buffs
{
namespace
{
constexpr const char* BuffsFile = "Data/Buffs/Buffs.json";
constexpr std::string_view BuffsLabel = "Buffs";
constexpr std::string_view BuffsName = "buffs";

void OnLocaleChanged(void* /*context*/) noexcept
{
    g_BuffDatabase.SetDisplayLocale(I18N::GetCurrentLocale());
}

void AddWarning(std::vector<Items::ItemDataIssue>& issues, const std::string& source, const std::string& field,
                const std::string& message)
{
    issues.push_back({Items::ItemDataIssueSeverity::Warning, source, Items::ItemDataIssue::NoItem,
                      Items::ItemDataIssue::NoItem, field, message});
}

// The languages of a text that the game has no texts in: they never show.
void WarnAboutUnshownLocales(const LocalizedString& text, std::span<const char* const> uiLocales,
                             const std::string& source, const std::string& field,
                             std::vector<Items::ItemDataIssue>& issues)
{
    for (const auto& [locale, translation] : text.GetTranslations())
    {
        if (std::none_of(uiLocales.begin(), uiLocales.end(), [&](const char* uiLocale) { return locale == uiLocale; }))
        {
            AddWarning(issues, source, field,
                       "the " + locale + " text is never shown: the game has no texts in that language");
        }
    }
}

void FollowLocaleChanges()
{
    static bool registered = false;
    if (registered)
    {
        return;
    }
    I18N::RegisterLocaleObserver(&OnLocaleChanged, nullptr);
    registered = true;
}
} // namespace

std::filesystem::path GetBuffsFile()
{
    return BuffsFile;
}

BuffsLoadResult LoadBuffsFile(const std::filesystem::path& file)
{
    BuffsLoadResult result;
    const std::string source = file.generic_string();
    const std::optional<std::string> text = ReadTextFile(file);
    if (!text)
    {
        result.issues.push_back({Items::ItemDataIssueSeverity::Fatal, source, Items::ItemDataIssue::NoItem,
                                 Items::ItemDataIssue::NoItem, "", "could not be read"});
        return result;
    }
    std::vector<size_t> entries;
    ReadBuffsJson(*text, source, result.buffs, result.issues, &entries);
    ValidateBuffs(result.buffs, entries, source, result.issues);
    ValidateBuffsForGame(result.buffs, entries, I18N::GetAvailableLocales(), source, result.issues);
    return result;
}

void ValidateBuffsForGame(std::span<const BuffDefinition> buffs, std::span<const size_t> entries,
                          std::span<const char* const> uiLocales, const std::string& source,
                          std::vector<Items::ItemDataIssue>& issues)
{
    for (size_t position = 0; position < buffs.size(); ++position)
    {
        const BuffDefinition& buff = buffs[position];
        const size_t entry = position < entries.size() ? entries[position] : position;
        if (buff.number >= eBuff_Count)
        {
            AddWarning(issues, source, BuffField(entry, "number"),
                       std::to_string(buff.number) + " never shows: the game knows buffs 1 to " +
                           std::to_string(eBuff_Count - 1));
        }
        WarnAboutUnshownLocales(buff.displayName, uiLocales, source, BuffField(entry, "displayName"), issues);
        WarnAboutUnshownLocales(buff.description, uiLocales, source, BuffField(entry, "description"), issues);
        const auto warnIfCut = [&](const std::string& locale, const std::string& name)
        {
            if (StringUtils::NarrowToWide(name).size() > MaxDisplayNameLength)
            {
                AddWarning(issues, source, BuffField(entry, "displayName"),
                           "the " + locale + " name is cut to " + std::to_string(MaxDisplayNameLength) + " letters");
            }
        };
        warnIfCut(std::string(LocalizedString::NeutralLocale), buff.displayName.GetNeutral());
        for (const auto& [locale, name] : buff.displayName.GetTranslations())
        {
            warnIfCut(locale, name);
        }
    }
}

void ValidateBuffItemsInGame(std::span<const BuffDefinition> buffs, const std::string& source,
                             std::vector<Items::ItemDataIssue>& issues)
{
    ValidateBuffItems(
        buffs, {}, [](int group, int number) { return g_ItemDatabase.FindEntry(group, number) != nullptr; }, source,
        issues);
}

bool LoadBuffs(std::string& errorMessage)
{
    const auto loadStart = std::chrono::steady_clock::now();
    BuffsLoadResult result = LoadBuffsFile(GetBuffsFile());
    if (!Items::HasFatal(result.issues))
    {
        // The item list loads before the buffs.
        ValidateBuffItemsInGame(result.buffs, GetBuffsFile().generic_string(), result.issues);
    }
    LogDataIssues(BuffsLabel, result.issues);
    if (Items::HasFatal(result.issues))
    {
        errorMessage = DescribeFatalIssues(BuffsName, GetBuffsFile(), result.issues);
        return false;
    }
    g_BuffDatabase.Build(result.buffs, I18N::GetCurrentLocale());
    FollowLocaleChanges();
    const std::chrono::duration<double, std::milli> elapsed = std::chrono::steady_clock::now() - loadStart;
    MU_LOG_INFO(mu::log::Get("data"), "Loaded {} buffs from {} in {:.1f} ms", result.buffs.size(),
                GetBuffsFile().generic_string(), elapsed.count());
    return true;
}
} // namespace Data::Buffs
