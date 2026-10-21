#pragma once

#ifdef _EDITOR

#include "ItemEditDraft.h"
#include "ItemRequirementTable.h"
#include "ItemUsers.h"

#include "Data/GameData/ItemData/ItemDefinition.h"

// The item basics and stats tool (section 9): the selected item's English
// name, size, slot, kinds, stats, prices, resistances and stat profile.
class CMuItemBasicsUI
{
public:
    static CMuItemBasicsUI& GetInstance();

    void Render(bool* open);

private:
    CMuItemBasicsUI();

    void RenderName(Data::Items::ItemDefinition& item);
    void RenderShape(Data::Items::ItemDefinition& item, int itemType);
    void RenderStats(Data::Items::ItemDefinition& item);
    // What the game computes from the stats at each + level, also as
    // excellent or ancient, while a value is still being typed.
    void RenderStatTable(const Data::Items::ItemDefinition& item);
    void RenderPrices(Data::Items::ItemDefinition& item);
    void RenderResistances(Data::Items::ItemDefinition& item);

    MuEditor::Items::ItemEditDraft<Data::Items::ItemDefinition> m_draft;
    MuEditor::Items::ItemStatProfileUseCounts m_profileCounts;
    MuEditor::Items::RequirementVariant m_statVariant;
};

#define g_MuItemBasicsUI CMuItemBasicsUI::GetInstance()

#endif // _EDITOR
