#pragma once

#ifdef _EDITOR

#include "ItemEditDraft.h"
#include "ItemRequirementTable.h"
#include "ItemUsers.h"

#include "Data/GameData/ItemData/ItemDefinition.h"

// The item requirements tool (section 9): the selected item's level and stat
// requirements (the base values the item stat file's formula takes), those it
// gives by item level instead, what the game asks at each + level from them,
// the classes that can use it, and whether excellent options raise its level
// requirement.
class CMuItemRequirementsUI
{
public:
    static CMuItemRequirementsUI& GetInstance();

    void Render(bool* open);

private:
    CMuItemRequirementsUI();

    void RenderRequirements(Data::Items::ItemDefinition& item);
    // Whether the requirement is given by level, and its 16 values if so.
    // Turning it on fills them with what `baseValue` gives a normal item at
    // each level, turning it off finds the base value that gives them.
    void RenderByLevel(Data::Items::ItemDefinition& item, Data::Items::ItemRequirement requirement, const char* label,
                       WORD& baseValue);
    void RenderLevelTable(const Data::Items::ItemDefinition& item);
    void RenderClasses(Data::Items::ItemDefinition& item);

    MuEditor::Items::ItemEditDraft<Data::Items::ItemDefinition> m_draft;
    MuEditor::Items::RequirementVariant m_variant;
    MuEditor::Items::ItemStatProfileUseCounts m_profileCounts;
};

#define g_MuItemRequirementsUI CMuItemRequirementsUI::GetInstance()

#endif // _EDITOR
