#include "stdafx.h"

#ifdef _EDITOR

#include "ItemCodeReferences.h"

#include "ItemCodeLine.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "UI/Common/CodeReferenceScan.h"
#include "UI/Common/SourceText.h"

#include <algorithm>
#include <cstddef>
#include <optional>

namespace MuEditor::Items
{
namespace
{
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
} // namespace

bool ItemCodeReferences::Load(const std::filesystem::path& sourceFolder)
{
    m_codeNames.clear();
    m_references.clear();
    m_loaded = false;
    const std::optional<std::string> enumText = CodeReferenceScan::ReadEnumFile(sourceFolder);
    if (!enumText)
        return false;
    const ItemConstants constants = ReadItemConstants(*enumText);
    m_codeNames = CodeNamesOf(constants);
    const auto readLine = [&constants](std::string_view code) { return ReadItemMentions(code, constants); };
    m_references = CodeReferenceScan::FindReferences(sourceFolder, static_cast<size_t>(MAX_ITEM), readLine);
    m_loaded = true;
    return true;
}

std::span<const ItemCodeReference> ItemCodeReferences::Find(int itemType) const
{
    return CodeReferenceScan::ReferencesOf(m_references, itemType);
}

std::string_view ItemCodeReferences::GetCodeName(int itemType) const
{
    if (!m_loaded || !Data::Items::IsValidItemType(itemType))
        return {};
    return m_codeNames[static_cast<size_t>(itemType)];
}
} // namespace MuEditor::Items

#endif // _EDITOR
