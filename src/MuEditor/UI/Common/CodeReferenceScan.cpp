#include "stdafx.h"

#ifdef _EDITOR

#include "CodeReferenceScan.h"

#include "MuEditorSourceDir.h"
#include "SourceText.h"

#include <algorithm>
#include <system_error>

namespace MuEditor::CodeReferenceScan
{
namespace
{
// The sources the editor was built from; UTF-8, as the path may hold any
// letter.
constexpr const char8_t* SourceDirectory = MU_EDITOR_SOURCE_DIR;

// Where the item and monster constants are defined, relative to the source
// folder.
constexpr std::string_view EnumFile = "Core/Globals/_enum.h";

// Adds the lines of one source file that name a number to `references`, by
// number.
void AddFileReferences(const SourceText::SourceFile& file, const LineReader& readLine, ReferencesByNumber& references)
{
    const std::string source = SourceText::ReadFile(file.path);
    const std::string code = SourceText::Blank(source);
    int line = 1;
    for (size_t start = 0; start < source.size();)
    {
        const size_t end = std::min(source.find('\n', start), source.size());
        const size_t length = end - start;
        const std::vector<Mention> mentions = readLine(std::string_view(code).substr(start, length));
        if (!mentions.empty())
        {
            const std::string text = SourceText::Trimmed(std::string_view(source).substr(start, length));
            for (const Mention& mention : mentions)
            {
                references[static_cast<size_t>(mention.number)].push_back({file.relative, line, text, mention.kind});
            }
        }
        start = end + 1;
        ++line;
    }
}
} // namespace

std::filesystem::path EditorSourceFolder()
{
    return std::filesystem::path(SourceDirectory);
}

std::optional<std::string> ReadEnumFile(const std::filesystem::path& sourceFolder)
{
    const std::filesystem::path enumFile = sourceFolder / std::filesystem::path(EnumFile);
    std::error_code error;
    if (!std::filesystem::is_regular_file(enumFile, error))
    {
        return std::nullopt;
    }
    return SourceText::ReadFile(enumFile);
}

ReferencesByNumber FindReferences(const std::filesystem::path& sourceFolder, size_t count, const LineReader& readLine)
{
    ReferencesByNumber references(count);
    for (const SourceText::SourceFile& file : SourceText::FindSourceFiles(sourceFolder))
    {
        if (file.relative != EnumFile)
        {
            AddFileReferences(file, readLine, references);
        }
    }
    return references;
}

std::span<const CodeReference> ReferencesOf(const ReferencesByNumber& references, int number)
{
    if (number < 0 || static_cast<size_t>(number) >= references.size())
    {
        return {};
    }
    return references[static_cast<size_t>(number)];
}
} // namespace MuEditor::CodeReferenceScan

#endif // _EDITOR
