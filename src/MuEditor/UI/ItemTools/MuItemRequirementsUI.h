#pragma once

#ifdef _EDITOR

#include "ItemEditDraft.h"

#include "Data/GameData/ItemData/ItemDefinition.h"

// The item requirements tool (section 9): the selected item's level and stat
// requirements, those it gives by item level instead, the classes that can
// use it, and whether excellent options raise its level requirement.
class CMuItemRequirementsUI
{
public:
    static CMuItemRequirementsUI& GetInstance();

    void Render(bool* open);

private:
    CMuItemRequirementsUI();

    void RenderRequirements(Data::Items::ItemDefinition& item);
    // Whether the requirement is given by level, and its 16 values if so.
    // Turning it on moves `baseValue` into the 16 values, turning it off
    // takes the first one back.
    void RenderByLevel(Data::Items::ItemDefinition& item, Data::Items::ItemRequirement requirement, const char* label,
                       WORD& baseValue);
    void RenderClasses(Data::Items::ItemDefinition& item);

    MuEditor::Items::ItemEditDraft<Data::Items::ItemDefinition> m_draft;
};

#define g_MuItemRequirementsUI CMuItemRequirementsUI::GetInstance()

#endif // _EDITOR
