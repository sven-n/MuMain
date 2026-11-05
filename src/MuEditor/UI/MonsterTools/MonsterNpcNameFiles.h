#pragma once

#ifdef _EDITOR

#include "Data/DataHandler/LegacyLanguages.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <filesystem>
#include <span>
#include <string>
#include <vector>

// Monster Translations' import and export of the original client's monster
// name files, Data/Local/<Eng|Por|Spn>/NpcName_<Eng|Por|Spn>.txt (see
// NpcNameText.h), by number and language. Kept apart from ImGui so it can be
// tested.
namespace MuEditor::Monsters::NpcNameFiles
{
// What the import did with one language's file.
struct LanguageImport
{
    std::string locale;
    std::filesystem::path file;
    // False: the file could not be read, and the language was skipped.
    bool found = false;
    int read = 0;
    int changed = 0;
    // Names not set: the Kalima gates, numbers without an entry, an empty
    // English name, a name with the separator (the last three with a warning).
    int skipped = 0;
    // Names longer than the game holds, cut when they were read.
    int cut = 0;
    std::vector<Data::Items::ItemDataIssue> issues;
};

// What the export did with one language's file.
struct LanguageExport
{
    std::string locale;
    std::filesystem::path file;
    int names = 0;
    // Names written in English: the language has no name of its own for them.
    int english = 0;
    // The file had these names already and was left as it was.
    bool unchanged = false;
    bool written = false;
    std::vector<Data::Items::ItemDataIssue> issues;
};

// Data/Local below the client folder; an empty folder is the working
// directory, the client folder of the running game.
std::filesystem::path GetLocalFolder(const std::filesystem::path& clientFolder);
// Data/Local/<folder>/NpcName_<folder>.txt below the client folder.
std::filesystem::path GetFile(const std::filesystem::path& clientFolder, const Data::LegacyLanguage& language);

// Whether the import leaves the number's names alone: the Kalima gates
// 152-158. Their names in the files were formats for the owner's name, which
// the game now composes with a UI text.
bool IsLeftByImport(int number);

// Reads each language's file below the client folder and sets its names on
// the monsters that have an entry, through the monster edits: all languages
// are one undo step `label`. An empty name removes the language's translation;
// an empty English name is skipped. A name equal to the English one stays
// missing where the monster has no name of its own in the language (Export
// writes the English name there). A file that cannot be read is skipped.
std::vector<LanguageImport> Import(const std::filesystem::path& clientFolder, std::string label,
                                   std::span<const Data::LegacyLanguage> languages = Data::GetLegacyLanguages());

// Writes each language's file below the client folder with the names of all
// monsters with an entry, in English where the language has no name of its
// own. A file that already has these names is not written; one that changes is
// backed up first, as the item bmd export does. A file whose backup fails is not
// written, with an error; the other languages go on.
std::vector<LanguageExport> Export(const std::filesystem::path& clientFolder,
                                   std::span<const Data::LegacyLanguage> languages = Data::GetLegacyLanguages());
} // namespace MuEditor::Monsters::NpcNameFiles

#endif // _EDITOR
