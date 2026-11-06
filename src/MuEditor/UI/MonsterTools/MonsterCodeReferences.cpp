#include "stdafx.h"

#ifdef _EDITOR

#include "MonsterCodeReferences.h"

#include "MonsterCodeLine.h"

#include "../MuEditor/UI/Common/CodeReferenceScan.h"
#include "../MuEditor/UI/Common/SourceText.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace MuEditor::Monsters
{
namespace
{
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
} // namespace

bool MonsterCodeReferences::Load(const std::filesystem::path& sourceFolder)
{
    m_references.clear();
    m_loaded = false;
    const std::optional<std::string> enumText = CodeReferenceScan::ReadEnumFile(sourceFolder);
    if (!enumText)
    {
        return false;
    }
    const MonsterConstants constants = ReadMonsterConstants(*enumText);
    const auto readLine = [&constants](std::string_view code) { return ReadMonsterMentions(code, constants); };
    const auto monsterCount = static_cast<size_t>(Data::Monsters::MonsterNumberCount);
    m_references = CodeReferenceScan::FindReferences(sourceFolder, monsterCount, readLine);
    m_loaded = true;
    return true;
}

std::span<const CodeReference> MonsterCodeReferences::Find(int number) const
{
    return CodeReferenceScan::ReferencesOf(m_references, number);
}
} // namespace MuEditor::Monsters

#endif // _EDITOR
