#pragma once

#ifdef _EDITOR

#include <functional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace MuEditor::Effects
{
struct PreviewItem
{
    int type = 0;
    std::string name;
    // The name in lowercase and the type number, for the search.
    std::string searchText;
};

// The items the effect preview can show a type on: those with a name and a
// model to draw, found by a part of their name or their number.
class EffectPreviewItems
{
public:
    using NameOf = std::function<std::string(int itemType)>;
    using IsDrawable = std::function<bool(int itemType)>;

    // Lists the item types below `itemCount` that have a name (UTF-8) and are
    // drawable.
    void Build(int itemCount, const NameOf& nameOf, const IsDrawable& drawable);

    std::span<const PreviewItem> GetItems() const
    {
        return m_items;
    }
    const PreviewItem* Find(int itemType) const;
    // The indexes in GetItems() of the items whose name or number contains
    // `search` (any case).
    std::vector<int> Filter(std::string_view search) const;

private:
    std::vector<PreviewItem> m_items;
};
} // namespace MuEditor::Effects

#endif // _EDITOR
