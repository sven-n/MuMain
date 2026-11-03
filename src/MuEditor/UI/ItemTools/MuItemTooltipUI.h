#pragma once

#ifdef _EDITOR

#include "ItemEditDraft.h"
#include "ItemLookUseCounts.h"

#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Engine/Object/ZzzInventory.h"

#include <vector>

// Item Tooltip (D71): how the selected item is presented. For now its name in the tooltip and on the ground (its ground
// label, D68), previewed at a level and with options by the game's own functions, and the item's own label rules; and
// the tooltip layout it uses (D73). From #815 step 6a on it shows the item's full tooltip too.
class CMuItemTooltipUI
{
public:
    static CMuItemTooltipUI& GetInstance();

    void Render(bool* open);

private:
    CMuItemTooltipUI();

    void RenderPreview(const Data::Items::ItemDefinition& item, int itemType);
    void RenderSetCombo(int itemType);
    void RenderRules(Data::Items::ItemDefinition& item);
    // The tooltip layout the item uses (D73), picked from the tooltip file's layouts, and a note of its own changes.
    void RenderLayout(Data::Items::ItemDefinition& item);
    // The level fields of the rule at `index`; true when the rule changed.
    bool RenderRuleLevels(std::vector<Data::Items::ItemGroundLabel>& rules, size_t index);
    void AddRule(Data::Items::ItemDefinition& item) const;
    // The item at the previewed level and with its options, as the server would send it.
    ITEM MakePreviewItem(int itemType) const;
    // The label of the item as the game draws it on the ground, with the previewed level and options.
    GroundItemLabel DescribePreviewLabel(int itemType) const;
    GroundItemLabel DescribePreviewLabel(const ITEM& item) const;

    // The previewed item: its level and options. They stay when another item is selected.
    int m_level = 0;
    bool m_skill = false;
    bool m_luck = false;
    bool m_option = false;
    bool m_excellent = false;
    // The set (the ancient discriminator); 0 for none.
    int m_set = 0;
    MuEditor::Items::ItemEditDraft<Data::Items::ItemDefinition> m_draft;
    MuEditor::Items::ItemLookUseCounts m_counts;
};

#define g_MuItemTooltipUI CMuItemTooltipUI::GetInstance()

#endif // _EDITOR
