#include "stdafx.h"

#ifdef _EDITOR

#include "ItemCodeReferences.h"

#include "ItemCodeLine.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "UI/Common/SourceText.h"

#include <algorithm>
#include <cstddef>
#include <system_error>

namespace MuEditor::Items
{
namespace
{
// Where the item constants are defined, relative to the source folder.
constexpr std::string_view EnumFile = "Core/Globals/_enum.h";
constexpr std::string_view ItemConstantPrefix = "ITEM_";

// The item constants that the text of _enum.h defines.
ItemConstants ReadItemConstants(std::string_view enumText)
{
    const std::string code = SourceText::Blank(enumText);
    ItemConstants constants;
    for (size_t start = 0; start < code.size();)
    {
        const size_t end = std::min(code.find('\n', start), code.size());
        if (const auto constant = ReadItemConstant(std::string_view(code).substr(start, end - start)))
            constants.emplace(std::string(constant->first), constant->second);
        start = end + 1;
    }
    return constants;
}

// The constant shown for each item type: its ITEM_ constant, else its MODEL_
// constant.
std::vector<std::string> CodeNamesOf(const ItemConstants& constants)
{
    std::vector<std::string> codeNames(static_cast<size_t>(MAX_ITEM));
    for (const auto& [name, itemType] : constants)
    {
        std::string& codeName = codeNames[static_cast<size_t>(itemType)];
        if (codeName.empty() || (name.starts_with(ItemConstantPrefix) && !codeName.starts_with(ItemConstantPrefix)))
            codeName = name;
    }
    return codeNames;
}

// Adds the lines of one source file that name an item to `references`, by
// item type.
void AddFileReferences(const SourceText::SourceFile& file, const ItemConstants& constants,
                       std::vector<std::vector<ItemCodeReference>>& references)
{
    const std::string source = SourceText::ReadFile(file.path);
    const std::string code = SourceText::Blank(source);
    int line = 1;
    for (size_t start = 0; start < source.size();)
    {
        const size_t end = std::min(source.find('\n', start), source.size());
        const size_t length = end - start;
        const std::vector<ItemMention> mentions =
            ReadItemMentions(std::string_view(code).substr(start, length), constants);
        if (!mentions.empty())
        {
            const std::string text = SourceText::Trimmed(std::string_view(source).substr(start, length));
            for (const ItemMention& mention : mentions)
                references[static_cast<size_t>(mention.itemType)].push_back({file.relative, line, text, mention.kind});
        }
        start = end + 1;
        ++line;
    }
}
} // namespace

bool ItemCodeReferences::Load(const std::filesystem::path& sourceFolder)
{
    m_codeNames.clear();
    m_references.clear();
    m_loaded = false;
    const std::filesystem::path enumFile = sourceFolder / std::filesystem::path(EnumFile);
    std::error_code error;
    if (!std::filesystem::is_regular_file(enumFile, error))
        return false;
    const ItemConstants constants = ReadItemConstants(SourceText::ReadFile(enumFile));
    m_codeNames = CodeNamesOf(constants);
    m_references.resize(static_cast<size_t>(MAX_ITEM));
    for (const SourceText::SourceFile& file : SourceText::FindSourceFiles(sourceFolder))
    {
        // _enum.h defines the names; it does not use them.
        if (file.relative != EnumFile)
            AddFileReferences(file, constants, m_references);
    }
    m_loaded = true;
    return true;
}

std::span<const ItemCodeReference> ItemCodeReferences::Find(int itemType) const
{
    if (!m_loaded || !Data::Items::IsValidItemType(itemType))
        return {};
    return m_references[static_cast<size_t>(itemType)];
}

std::string_view ItemCodeReferences::GetCodeName(int itemType) const
{
    if (!m_loaded || !Data::Items::IsValidItemType(itemType))
        return {};
    return m_codeNames[static_cast<size_t>(itemType)];
}
} // namespace MuEditor::Items

#endif // _EDITOR
