#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterNpcNameFiles.h"

#include "MonsterEdits.h"
#include "MonsterStepLabel.h"

#include "Core/Globals/_enum.h"
#include "Core/Platform/PathResolve.h"
#include "Data/DataHandler/CommonDataSaver.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "Data/GameData/MonsterData/NpcNameText.h"

#include <map>
#include <optional>
#include <utility>

namespace MuEditor::Monsters::NpcNameFiles
{
namespace
{
using Data::LocalizedString;
using Data::Items::ItemDataIssue;
using Data::Items::ItemDataIssueSeverity;
using Data::Monsters::MonsterDefinition;

constexpr const char* FilePrefix = "NpcName_";
constexpr const char* FileExtension = ".txt";

enum class NameOutcome
{
    Unchanged,
    Changed,
    Skipped,
};

// The file as it is spelled on disk: Linux tells the case of names apart, and
// the client folder does not always spell them as the code does.
std::filesystem::path OnDisk(const std::filesystem::path& file)
{
#ifdef _WIN32
    return file;
#else
    return std::filesystem::path(MuResolvePath(file.generic_string().c_str()));
#endif
}

void Warn(std::vector<ItemDataIssue>& issues, const std::string& source, int number, const std::string& message)
{
    issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, number,
                      "monster " + std::to_string(number) + " name", message});
}

void AddFileError(std::vector<ItemDataIssue>& issues, const std::string& source, const std::string& message)
{
    issues.push_back({ItemDataIssueSeverity::Error, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, "", message});
}

// Sets one name read from a file of `locale`, unless the import leaves it.
NameOutcome ApplyName(int number, std::string name, const std::string& locale, const std::string& source,
                      std::vector<ItemDataIssue>& issues)
{
    if (IsLeftByImport(number))
    {
        return NameOutcome::Skipped;
    }
    const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
    if (monster == nullptr)
    {
        // The file knows nothing else about the monster (kind, model): the
        // Monster Browser adds it.
        Warn(issues, source, number, "the number has no entry in the monster list; skipped");
        return NameOutcome::Skipped;
    }
    if (monster->names.GetOwn(locale) == name)
    {
        return NameOutcome::Unchanged;
    }
    // Export writes the English name where a language has none, so such a
    // name stays missing, unless the monster has a name of its own in the
    // language (it is the same there). The item bmd import does the same.
    const bool isEnglish = locale == LocalizedString::NeutralLocale;
    if (!isEnglish && !monster->names.HasOwn(locale) && name == monster->names.GetNeutral())
    {
        return NameOutcome::Unchanged;
    }
    if (isEnglish && name.empty())
    {
        Warn(issues, source, number, "the English name is empty; skipped, the monster keeps its name");
        return NameOutcome::Skipped;
    }
    const std::string label = StepLabel::Name(*monster, locale.c_str());
    // Refused only for a name with the separator, which no save would take.
    if (!g_MonsterEdits.SetMonsterName(label, number, locale, std::move(name)))
    {
        Warn(issues, source, number, "the name holds \"" + std::string(LocalizedString::Separator) + "\"; skipped");
        return NameOutcome::Skipped;
    }
    return NameOutcome::Changed;
}

LanguageImport ImportLanguage(const std::filesystem::path& clientFolder, const Data::LegacyLanguage& language)
{
    LanguageImport result;
    result.locale = language.locale;
    result.file = GetFile(clientFolder, language);
    const std::optional<std::string> text = Data::ReadTextFile(OnDisk(result.file));
    if (!text)
    {
        return result;
    }
    result.found = true;

    const std::string source = result.file.generic_string();
    std::map<int, std::string> names;
    result.cut = Data::Monsters::ReadNpcNameText(*text, source, names, result.issues);
    result.read = static_cast<int>(names.size());
    for (auto& [number, name] : names)
    {
        switch (ApplyName(number, std::move(name), language.locale, source, result.issues))
        {
        case NameOutcome::Changed:
            ++result.changed;
            break;
        case NameOutcome::Skipped:
            ++result.skipped;
            break;
        case NameOutcome::Unchanged:
            break;
        }
    }
    return result;
}

// The names of all monsters with an entry in the language; `english` counts
// those that fall back to the English name.
std::map<int, std::string> CollectNames(const std::string& locale, int& english)
{
    std::map<int, std::string> names;
    const bool isEnglish = locale == LocalizedString::NeutralLocale;
    for (int number = 0; number < Data::Monsters::MonsterNumberCount; ++number)
    {
        const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
        if (monster == nullptr)
        {
            continue;
        }
        if (!isEnglish && !monster->names.HasOwn(locale))
        {
            ++english;
        }
        names.emplace(number, monster->names.Get(locale));
    }
    return names;
}

LanguageExport ExportLanguage(const std::filesystem::path& clientFolder, const Data::LegacyLanguage& language)
{
    LanguageExport result;
    result.locale = language.locale;
    result.file = GetFile(clientFolder, language);
    const std::string source = result.file.generic_string();

    const std::map<int, std::string> names = CollectNames(language.locale, result.english);
    const std::string text = Data::Monsters::WriteNpcNameText(names, result.issues);
    // The writer leaves out each name it cannot write, with a warning.
    result.names = static_cast<int>(names.size() - result.issues.size());
    for (ItemDataIssue& issue : result.issues)
    {
        issue.source = source;
    }

    const std::filesystem::path target = OnDisk(result.file);
    if (const std::optional<std::string> old = Data::ReadTextFile(target); old && *old == text)
    {
        result.unchanged = true;
        return result;
    }
    // Without the backup the old names would be lost.
    if (!CommonDataSaver::CreateBackup(target.wstring().c_str()))
    {
        AddFileError(result.issues, source, "could not be backed up; not written");
        return result;
    }
    std::string error;
    if (!Data::WriteTextFile(target, text, error))
    {
        AddFileError(result.issues, source, error);
        return result;
    }
    result.written = true;
    return result;
}
} // namespace

std::filesystem::path GetLocalFolder(const std::filesystem::path& clientFolder)
{
    return clientFolder / "Data" / "Local";
}

std::filesystem::path GetFile(const std::filesystem::path& clientFolder, const Data::LegacyLanguage& language)
{
    std::filesystem::path name(FilePrefix);
    name += language.folder;
    name += FileExtension;
    return GetLocalFolder(clientFolder) / language.folder / name;
}

bool IsLeftByImport(int number)
{
    return number >= MONSTER_GATE_TO_KALIMA_1 && number <= MONSTER_GATE_TO_KALIMA_7;
}

std::vector<LanguageImport> Import(const std::filesystem::path& clientFolder, std::string label,
                                   std::span<const Data::LegacyLanguage> languages)
{
    std::vector<LanguageImport> results;
    g_MonsterEdits.BeginGroup(std::move(label));
    for (const Data::LegacyLanguage& language : languages)
    {
        results.push_back(ImportLanguage(clientFolder, language));
    }
    g_MonsterEdits.EndGroup();
    return results;
}

std::vector<LanguageExport> Export(const std::filesystem::path& clientFolder,
                                   std::span<const Data::LegacyLanguage> languages)
{
    std::vector<LanguageExport> results;
    for (const Data::LegacyLanguage& language : languages)
    {
        results.push_back(ExportLanguage(clientFolder, language));
    }
    return results;
}
} // namespace MuEditor::Monsters::NpcNameFiles

#endif // _EDITOR
