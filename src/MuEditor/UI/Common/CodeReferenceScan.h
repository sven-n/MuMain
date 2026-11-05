#pragma once

#ifdef _EDITOR

#include "CodeReference.h"

#include <cstddef>
#include <filesystem>
#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Reading a source folder for the lines that name an item or a monster. The
// names are the constants of Core/Globals/_enum.h, which the item and monster
// tools read each in their own way; a line reader then reads every line of the
// other .cpp and .h files for the numbers it names.
namespace MuEditor::CodeReferenceScan
{
// A number a line names (an item type, a monster number), and how.
struct Mention
{
    int number = 0;
    CodeReference::Kind kind = CodeReference::Kind::Name;
};

// The numbers one line of code names, each below the count FindReferences
// takes; the line has its comments and string literals blanked
// (SourceText::Blank).
using LineReader = std::function<std::vector<Mention>(std::string_view code)>;

// The source lines that name each number, by number.
using ReferencesByNumber = std::vector<std::vector<CodeReference>>;

// The sources the editor was built from (src/source).
std::filesystem::path EditorSourceFolder();

// The text of _enum.h (Core/Globals/_enum.h below the folder); nothing when
// the folder or the file is missing.
std::optional<std::string> ReadEnumFile(const std::filesystem::path& sourceFolder);

// The lines of every .cpp and .h file below the folder except _enum.h that
// name a number from 0 to `count` - 1, by number. _enum.h defines the names;
// it does not use them.
ReferencesByNumber FindReferences(const std::filesystem::path& sourceFolder, size_t count, const LineReader& readLine);

// The lines that name the number; empty when the number has no list.
std::span<const CodeReference> ReferencesOf(const ReferencesByNumber& references, int number);
} // namespace MuEditor::CodeReferenceScan

#endif // _EDITOR
