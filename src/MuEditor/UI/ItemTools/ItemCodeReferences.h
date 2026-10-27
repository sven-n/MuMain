#pragma once

#ifdef _EDITOR

#include "UI/Common/CodeReference.h"

#include <filesystem>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace MuEditor::Items
{
// A source line that names an item, which the item tools show before the
// item is removed: by its ITEM_ or MODEL_ constant (Name), its group and number
// (RawId, ITEM_POTION + 100) or a range check (Range).
using ItemCodeReference = CodeReference;

// The lines of a source folder that name an item, by item type. The item
// names are the constants of Core/Globals/_enum.h; comments and string
// literals name nothing.
class ItemCodeReferences
{
public:
    // Reads _enum.h (Core/Globals/_enum.h below the folder) for the item
    // names, then every .cpp and .h file below the folder except _enum.h.
    // False when the folder or _enum.h is missing; then nothing is known.
    bool Load(const std::filesystem::path& sourceFolder);
    bool IsLoaded() const
    {
        return m_loaded;
    }

    // The places that name the item, by file and line: its ITEM_ or MODEL_
    // constant, a raw id ITEM_<GROUP> + N or MODEL_<GROUP> + N, or a range on
    // one line whose two bounds (>=, >, <=, <, the item on either side)
    // enclose it. A group base alone (ITEM_SWORD) names no item, but bounds a
    // range; a range between two group bases tests whole groups and names no
    // item either.
    std::span<const ItemCodeReference> Find(int itemType) const;

    // "ITEM_KRIS"; the MODEL_ constant when the item has no ITEM_ constant,
    // empty when it has neither.
    std::string_view GetCodeName(int itemType) const;

private:
    // By item type; empty until loaded.
    std::vector<std::string> m_codeNames;
    std::vector<std::vector<ItemCodeReference>> m_references;
    bool m_loaded = false;
};
} // namespace MuEditor::Items

#endif // _EDITOR
