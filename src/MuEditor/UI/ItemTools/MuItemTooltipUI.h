#pragma once

#ifdef _EDITOR

#include "ItemEditDraft.h"
#include "ItemLookUseCounts.h"

#include "Data/GameData/ItemData/ItemDefinition.h"
#include "Engine/Object/ZzzInventory.h"

#include <vector>

// Item Tooltip (D71): how the selected item is presented. For now its ground label (D68): a preview at a level and with
// options, drawn with the game's own label function, and the item's own label rules. From #815 step 6a on it shows the
// item's full tooltip too.
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
    // The rule's level fields; true when the rule changed.
    bool RenderRuleLevels(Data::Items::ItemGroundLabel& rule);
    void AddRule(Data::Items::ItemDefinition& item) const;
    // The label of the item as the game draws it on the ground, with the previewed level and options.
    GroundItemLabel DescribePreviewLabel(int itemType) const;

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
