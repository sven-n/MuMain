#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterCodeReferences.h"

#include "MonsterCodeLine.h"

#include "../MuEditor/UI/Common/SourceText.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <string_view>
#include <system_error>

namespace MuEditor::Monsters
{
namespace
{
// Where the monster constants are defined, relative to the source folder.
constexpr std::string_view EnumFile = "Core/Globals/_enum.h";
// Only this enum holds monster numbers; the other MONSTER_ names of _enum.h
// (EMonsterModelType) are models.
constexpr std::string_view MonsterEnum = "enum EMonsterType";
constexpr std::string_view EnumEnd = "}";

// The monster constants that enum EMonsterType in the text of _enum.h defines.
MonsterConstants ReadMonsterConstants(std::string_view enumText)
{
    const std::string code = SourceText::Blank(enumText);
    MonsterConstants constants;
    const size_t enumStart = code.find(MonsterEnum);
    if (enumStart == std::string::npos)
    {
        return constants;
    }
    // From the line after "enum EMonsterType" to its closing brace.
    for (size_t start = std::min(code.find('\n', enumStart), code.size()) + 1; start < code.size();)
    {
        const size_t end = std::min(code.find('\n', start), code.size());
        const std::string_view line = std::string_view(code).substr(start, end - start);
        if (SourceText::Trimmed(line).starts_with(EnumEnd))
        {
            break;
        }
        if (const auto constant = ReadMonsterConstant(line))
        {
            constants.emplace(std::string(constant->first),
                              MonsterConstant{constant->second, IsMonsterMarker(constant->first)});
        }
        start = end + 1;
    }
    return constants;
}

// Adds the lines of one source file that name a monster to `references`, by
// monster number.
void AddFileReferences(const SourceText::SourceFile& file, const MonsterConstants& constants,
                       std::vector<std::vector<CodeReference>>& references)
{
    const std::string source = SourceText::ReadFile(file.path);
    const std::string code = SourceText::Blank(source);
    int line = 1;
    for (size_t start = 0; start < source.size();)
    {
        const size_t end = std::min(source.find('\n', start), source.size());
        const size_t length = end - start;
        const std::vector<MonsterMention> mentions =
            ReadMonsterMentions(std::string_view(code).substr(start, length), constants);
        if (!mentions.empty())
        {
            const std::string text = SourceText::Trimmed(std::string_view(source).substr(start, length));
            for (const MonsterMention& mention : mentions)
            {
                references[static_cast<size_t>(mention.number)].push_back({file.relative, line, text, mention.kind});
            }
        }
        start = end + 1;
        ++line;
    }
}
} // namespace

bool MonsterCodeReferences::Load(const std::filesystem::path& sourceFolder)
{
    m_references.clear();
    m_loaded = false;
    const std::filesystem::path enumFile = sourceFolder / std::filesystem::path(EnumFile);
    std::error_code error;
    if (!std::filesystem::is_regular_file(enumFile, error))
    {
        return false;
    }
    const MonsterConstants constants = ReadMonsterConstants(SourceText::ReadFile(enumFile));
    m_references.resize(static_cast<size_t>(Data::Monsters::MonsterNumberCount));
    for (const SourceText::SourceFile& file : SourceText::FindSourceFiles(sourceFolder))
    {
        // _enum.h defines the names; it does not use them.
        if (file.relative != EnumFile)
        {
            AddFileReferences(file, constants, m_references);
        }
    }
    m_loaded = true;
    return true;
}

std::span<const CodeReference> MonsterCodeReferences::Find(int number) const
{
    if (!m_loaded || number < 0 || number >= Data::Monsters::MonsterNumberCount)
    {
        return {};
    }
    return m_references[static_cast<size_t>(number)];
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
