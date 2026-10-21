#pragma once

#ifdef _EDITOR

#include "Data/GameData/ItemData/ItemDefinition.h"

#include <optional>
#include <string>

namespace MuEditor::Items
{
// The weights that give a typed +0 requirement, to pick one (Item
// Requirements): each with what it gives at +15 and for excellent items. For a
// value no weight gives, the nearest values that some do.
class CRequirementWeightPopup
{
public:
    // Opens it for `value` of the requirement; `label` names the requirement.
    void Open(int itemType, Data::Items::ItemRequirement requirement, std::string label, int value);
    // Draws it while it is open, in the tool's item scope. Returns the weight
    // taken this frame.
    std::optional<int> Render(const Data::Items::ItemDefinition& item);

    Data::Items::ItemRequirement GetRequirement() const
    {
        return m_requirement;
    }
    const std::string& GetLabel() const
    {
        return m_label;
    }

private:
    void Select(int value, int weight);

    int m_itemType = -1;
    Data::Items::ItemRequirement m_requirement = Data::Items::ItemRequirement::Strength;
    std::string m_label;
    int m_value = 0;
    int m_selected = 0;
    bool m_opening = false;
    bool m_scrollToSelected = false;
};
} // namespace MuEditor::Items

#endif // _EDITOR
