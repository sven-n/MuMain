#include "stdafx.h"

#include "doctest.h"

// Monster Translations' import and export of the original client's monster
// name files, Data/Local/<Eng|Por|Spn>/NpcName_<...>.txt (MN1).
#ifdef _EDITOR

#include "TemporaryMonsterFolder.h"

#include "Data/DataHandler/LegacyLanguages.h"
#include "Data/DataHandler/TextFile.h"
#include "Data/GameData/MonsterData/MonsterDatabase.h"
#include "UI/MonsterTools/MonsterEdits.h"
#include "UI/MonsterTools/MonsterNpcNameFiles.h"

#include <algorithm>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

#ifdef _WIN32
#include <aclapi.h>
#endif

using Data::Items::ItemDataIssue;
using Data::Monsters::MonsterDefinition;
namespace NpcNameFiles = MuEditor::Monsters::NpcNameFiles;

namespace
{
constexpr char ImportLabel[] = "Import names";

MonsterDefinition Monster(int number, const char* names)
{
    MonsterDefinition monster;
    monster.number = number;
    monster.names = Data::LocalizedString::Parse(names);
    return monster;
}

// The monster table with these monsters, and an empty history that counts as
// saved.
void StartWith(const std::vector<MonsterDefinition>& monsters)
{
    g_MonsterDatabase.Build(monsters, "en");
    g_MonsterEdits.OnDataReplaced();
    g_MonsterEdits.GetHistory().MarkSaved();
}

const Data::LegacyLanguage& Language(std::string_view locale)
{
    const auto languages = Data::GetLegacyLanguages();
    const auto found = std::ranges::find(languages, locale, &Data::LegacyLanguage::locale);
    REQUIRE(found != languages.end());
    return *found;
}

void WriteNameFile(const TemporaryMonsterFolder& folder, std::string_view locale, const std::string& text)
{
    std::string error;
    REQUIRE(Data::WriteTextFile(NpcNameFiles::GetFile(folder.Path(), Language(locale)), text, error));
}

std::string ReadNameFile(const TemporaryMonsterFolder& folder, std::string_view locale)
{
    const std::optional<std::string> text = Data::ReadTextFile(NpcNameFiles::GetFile(folder.Path(), Language(locale)));
    REQUIRE(text.has_value());
    return *text;
}

template <typename TResult> const TResult& Of(const std::vector<TResult>& results, std::string_view locale)
{
    const auto found = std::ranges::find(results, locale, &TResult::locale);
    REQUIRE(found != results.end());
    return *found;
}

bool HasWarningAbout(const std::vector<ItemDataIssue>& issues, int number, const std::string& message)
{
    return std::ranges::any_of(issues,
                               [&](const ItemDataIssue& issue)
                               {
                                   return issue.severity == Data::Items::ItemDataIssueSeverity::Warning &&
                                          issue.number == number && issue.message.find(message) != std::string::npos;
                               });
}

const Data::LocalizedString& NamesOf(int number)
{
    const MonsterDefinition* monster = g_MonsterDatabase.Find(number);
    REQUIRE(monster != nullptr);
    return monster->names;
}

int CountBackups(const std::filesystem::path& folder)
{
    return static_cast<int>(std::ranges::count_if(std::filesystem::directory_iterator(folder),
                                                  [](const std::filesystem::directory_entry& entry)
                                                  { return entry.path().extension() == ".bak"; }));
}

#ifdef _WIN32
// Denies this user reading the file, as for a file of another user: it cannot be
// copied (so not backed up), but it can still be replaced. The file's old access
// comes back at the end.
class UnreadableFile
{
public:
    explicit UnreadableFile(const std::filesystem::path& file) : m_file(file.wstring())
    {
        PACL oldAcl = nullptr;
        if (GetNamedSecurityInfoW(m_file.c_str(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, &oldAcl,
                                  nullptr, &m_oldSecurity) != ERROR_SUCCESS)
        {
            return;
        }
        m_oldAcl = oldAcl;
        EXPLICIT_ACCESSW deny{};
        BuildExplicitAccessWithNameW(&deny, const_cast<LPWSTR>(L"CURRENT_USER"), FILE_READ_DATA, DENY_ACCESS,
                                     NO_INHERITANCE);
        PACL deniedAcl = nullptr;
        if (SetEntriesInAclW(1, &deny, oldAcl, &deniedAcl) != ERROR_SUCCESS)
        {
            return;
        }
        m_denied = SetAcl(deniedAcl);
        LocalFree(deniedAcl);
    }
    ~UnreadableFile()
    {
        if (m_denied)
        {
            SetAcl(m_oldAcl);
        }
        LocalFree(m_oldSecurity);
    }
    UnreadableFile(const UnreadableFile&) = delete;
    UnreadableFile& operator=(const UnreadableFile&) = delete;

    bool IsDenied() const
    {
        return m_denied;
    }

private:
    bool SetAcl(PACL acl)
    {
        return SetNamedSecurityInfoW(m_file.data(), SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, acl,
                                     nullptr) == ERROR_SUCCESS;
    }

    std::wstring m_file;
    PSECURITY_DESCRIPTOR m_oldSecurity = nullptr;
    PACL m_oldAcl = nullptr;
    bool m_denied = false;
};
#endif
} // namespace

TEST_CASE("NpcName txt: the files are below Data/Local in the language's folder [editor][monsters]")
{
    CHECK(NpcNameFiles::GetFile({}, Language("en")).generic_string() == "Data/Local/Eng/NpcName_Eng.txt");
    CHECK(NpcNameFiles::GetFile({}, Language("pt")).generic_string() == "Data/Local/Por/NpcName_Por.txt");
    CHECK(NpcNameFiles::GetFile({}, Language("es")).generic_string() == "Data/Local/Spn/NpcName_Spn.txt");
}

TEST_CASE("NpcName txt: the import sets the names by number and language, as one undo step [editor][monsters]")
{
    StartWith({Monster(2, "Budge Dragon||pt=Drag\xC3\xA3o"), Monster(3, "Spider"), Monster(152, "Gate to Kalima 1"),
               Monster(252, "")});
    const TemporaryMonsterFolder folder;
    WriteNameFile(folder, "en",
                  "// MAX : 512\r\n"
                  "2\t1\t\"Budge Dragon\"\r\n"
                  "3\t1\t\"Giant Spider\"\r\n"
                  "152\t1\t\"Gate to Kalima %s\"\r\n"
                  "252\t1\t\"\"\r\n"
                  "600\t1\t\"Nobody\"\r\n"
                  "end\r\n");
    WriteNameFile(folder, "pt",
                  "2\t1\t\"Drag\xC3\xA3o Lento\"\r\n"
                  "252\t1\t\"Sem nome\"\r\n"
                  "end\r\n");

    const auto results = NpcNameFiles::Import(folder.Path(), ImportLabel);

    const auto& english = Of(results, "en");
    CHECK(english.found);
    CHECK(english.read == 5);
    CHECK(english.changed == 1);
    // The Kalima gate and 600, which has no entry.
    CHECK(english.skipped == 2);
    CHECK(english.cut == 0);
    CHECK(HasWarningAbout(english.issues, 600, "no entry"));
    CHECK_FALSE(HasWarningAbout(english.issues, 152, ""));
    const auto& portuguese = Of(results, "pt");
    CHECK(portuguese.read == 2);
    CHECK(portuguese.changed == 2);
    CHECK_FALSE(Of(results, "es").found);

    CHECK(NamesOf(3).GetNeutral() == "Giant Spider");
    CHECK(NamesOf(2).GetOwn("pt") == "Drag\xC3\xA3o Lento");
    CHECK(NamesOf(252).GetOwn("pt") == "Sem nome");
    CHECK(NamesOf(152).GetNeutral() == "Gate to Kalima 1");
    CHECK(g_MonsterDatabase.Find(600) == nullptr);

    // One step for both files.
    CHECK(g_MonsterEdits.GetHistory().GetUndoLabel() == ImportLabel);
    REQUIRE(g_MonsterEdits.Undo());
    CHECK_FALSE(g_MonsterEdits.GetHistory().CanUndo());
    CHECK(NamesOf(3).GetNeutral() == "Spider");
    CHECK(NamesOf(2).GetOwn("pt") == "Drag\xC3\xA3o");
    CHECK_FALSE(NamesOf(252).HasOwn("pt"));
    REQUIRE(g_MonsterEdits.Redo());
    CHECK(NamesOf(3).GetNeutral() == "Giant Spider");
    CHECK(NamesOf(252).GetOwn("pt") == "Sem nome");
}

TEST_CASE("NpcName txt: an empty name removes a translation, never the English name [editor][monsters]")
{
    StartWith({Monster(2, "Budge Dragon||pt=Drag\xC3\xA3o||es=Drag\xC3\xB3n")});
    const TemporaryMonsterFolder folder;
    WriteNameFile(folder, "en", "2\t1\t\"\"\r\nend\r\n");
    WriteNameFile(folder, "pt", "2\t1\t\"\"\r\nend\r\n");
    WriteNameFile(folder, "es", "2\t1\t\"Drag\xC3\xB3n\"\r\nend\r\n");

    const auto results = NpcNameFiles::Import(folder.Path(), ImportLabel);

    CHECK(Of(results, "en").skipped == 1);
    CHECK(HasWarningAbout(Of(results, "en").issues, 2, "English name is empty"));
    CHECK(Of(results, "pt").changed == 1);
    CHECK(Of(results, "es").changed == 0);
    CHECK(NamesOf(2).GetNeutral() == "Budge Dragon");
    CHECK_FALSE(NamesOf(2).HasOwn("pt"));
    CHECK(NamesOf(2).GetOwn("es") == "Drag\xC3\xB3n");
}

TEST_CASE("NpcName txt: a name equal to the English one stays missing unless the language has a name of its own "
          "[editor][monsters]")
{
    StartWith({Monster(2, "Kundun||pt=Kundun Antigo"), Monster(3, "Spider")});
    const TemporaryMonsterFolder folder;
    WriteNameFile(folder, "pt",
                  "2\t1\t\"Kundun\"\r\n"
                  "3\t1\t\"Spider\"\r\n"
                  "end\r\n");

    const auto results = NpcNameFiles::Import(folder.Path(), ImportLabel);

    CHECK(Of(results, "pt").changed == 1);
    CHECK(NamesOf(2).GetOwn("pt") == "Kundun");
    CHECK_FALSE(NamesOf(3).HasOwn("pt"));
}

TEST_CASE("NpcName txt: a name with the separator is skipped and a cut name is counted [editor][monsters]")
{
    StartWith({Monster(2, "Budge Dragon"), Monster(3, "Spider")});
    const TemporaryMonsterFolder folder;
    WriteNameFile(folder, "es",
                  "2\t1\t\"Drag\xC3\xB3n||Lento\"\r\n"
                  "3\t1\t\"" +
                      std::string(70, 'x') + "\"\r\nend\r\n");

    const auto results = NpcNameFiles::Import(folder.Path(), ImportLabel);

    const auto& spanish = Of(results, "es");
    CHECK(spanish.skipped == 1);
    CHECK(spanish.changed == 1);
    CHECK(spanish.cut == 1);
    CHECK(HasWarningAbout(spanish.issues, 2, "||"));
    CHECK_FALSE(NamesOf(2).HasOwn("es"));
    CHECK(NamesOf(3).GetOwn("es") == std::string(64, 'x'));
}

TEST_CASE("NpcName txt: the exported files import without a change [editor][monsters]")
{
    const std::vector monsters = {Monster(2, "Budge Dragon||pt=Drag\xC3\xA3o Lento||es=Drag\xC3\xB3n Lento"),
                                  Monster(3, "Spider||es=Tar\xC3\xA1ntula"), Monster(152, "Gate to Kalima 1"),
                                  Monster(252, "")};
    StartWith(monsters);
    const TemporaryMonsterFolder folder;

    const auto exported = NpcNameFiles::Export(folder.Path());

    for (const auto& language : exported)
    {
        INFO(language.locale);
        CHECK(language.written);
        CHECK(language.names == 4);
        CHECK(language.issues.empty());
    }
    CHECK(Of(exported, "en").english == 0);
    // 3, 152 and 252 have no Portuguese name; 152 and 252 no Spanish one.
    CHECK(Of(exported, "pt").english == 3);
    CHECK(Of(exported, "es").english == 2);
    CHECK(ReadNameFile(folder, "pt") == "2\t1\t\"Drag\xC3\xA3o Lento\"\r\n"
                                        "3\t1\t\"Spider\"\r\n"
                                        "152\t1\t\"Gate to Kalima 1\"\r\n"
                                        "252\t1\t\"\"\r\n"
                                        "end\r\n");

    const auto imported = NpcNameFiles::Import(folder.Path(), ImportLabel);

    for (const auto& language : imported)
    {
        INFO(language.locale);
        CHECK(language.found);
        CHECK(language.read == 4);
        CHECK(language.changed == 0);
        CHECK(language.issues.empty());
    }
    CHECK_FALSE(g_MonsterEdits.GetHistory().CanUndo());
    for (const MonsterDefinition& monster : monsters)
    {
        CHECK(NamesOf(monster.number) == monster.names);
    }
}

TEST_CASE("NpcName txt: an unchanged file is not written again, a changed one is backed up first [editor][monsters]")
{
    StartWith({Monster(2, "Budge Dragon")});
    const TemporaryMonsterFolder folder;
    REQUIRE(Of(NpcNameFiles::Export(folder.Path()), "en").written);
    const std::filesystem::path englishFolder = NpcNameFiles::GetFile(folder.Path(), Language("en")).parent_path();
    CHECK(CountBackups(englishFolder) == 0);

    CHECK(Of(NpcNameFiles::Export(folder.Path()), "en").unchanged);
    CHECK(CountBackups(englishFolder) == 0);

    REQUIRE(g_MonsterEdits.SetMonsterName("Name", 2, "en", "Red Dragon"));
    const auto exported = NpcNameFiles::Export(folder.Path());

    CHECK(Of(exported, "en").written);
    CHECK(CountBackups(englishFolder) == 1);
    CHECK(ReadNameFile(folder, "en") == "2\t1\t\"Red Dragon\"\r\nend\r\n");
}

#ifdef _WIN32
TEST_CASE("NpcName txt: a file that cannot be backed up is not written, the others are [editor][monsters]")
{
    StartWith({Monster(2, "Budge Dragon")});
    const TemporaryMonsterFolder folder;
    const std::string oldText = "2\t1\t\"Red Dragon\"\r\nend\r\n";
    WriteNameFile(folder, "en", oldText);
    const std::filesystem::path englishFile = NpcNameFiles::GetFile(folder.Path(), Language("en"));

    std::vector<NpcNameFiles::LanguageExport> exported;
    {
        const UnreadableFile unreadable(englishFile);
        REQUIRE(unreadable.IsDenied());
        exported = NpcNameFiles::Export(folder.Path());
    }

    const auto& english = Of(exported, "en");
    CHECK_FALSE(english.written);
    REQUIRE(english.issues.size() == 1);
    CHECK(english.issues.front().severity == Data::Items::ItemDataIssueSeverity::Fatal);
    CHECK(english.issues.front().message.find("backed up") != std::string::npos);
    CHECK(CountBackups(englishFile.parent_path()) == 0);
    CHECK(ReadNameFile(folder, "en") == oldText);
    CHECK(Of(exported, "pt").written);
    CHECK(Of(exported, "es").written);
}
#endif

TEST_CASE("NpcName txt: a name with a quote is left out of the export, with a warning [editor][monsters]")
{
    StartWith({Monster(2, "Budge Dragon"), Monster(7, "Say \"hi\"")});
    const TemporaryMonsterFolder folder;

    const auto exported = NpcNameFiles::Export(folder.Path());

    const auto& english = Of(exported, "en");
    CHECK(english.written);
    CHECK(english.names == 1);
    REQUIRE(english.issues.size() == 1);
    CHECK(english.issues.front().source == english.file.generic_string());
    CHECK(ReadNameFile(folder, "en") == "2\t1\t\"Budge Dragon\"\r\nend\r\n");
}

#endif // _EDITOR
