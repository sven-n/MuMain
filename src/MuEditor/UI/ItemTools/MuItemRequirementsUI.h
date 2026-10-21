#pragma once

#ifdef _EDITOR

#include "ItemEditDraft.h"
#include "ItemRequirementTable.h"
#include "ItemUsers.h"
#include "RequirementWeightPopup.h"

#include "Data/GameData/ItemData/ItemDefinition.h"

#include <optional>

// The item requirements tool (section 9): the selected item's stat profile, its
// level and stat requirements (a stat requirement as its +0 value and the
// weight the item stat file's formula takes, with the weights that give the
// same +0 value), those it gives by item level instead, what the game asks at
// each + level from them, the classes that can use it, and whether excellent
// options raise its level requirement.
class CMuItemRequirementsUI
{
public:
    static CMuItemRequirementsUI& GetInstance();

    void Render(bool* open);

private:
    CMuItemRequirementsUI();

    void RenderRequirements(Data::Items::ItemDefinition& item);
    // The +0 value of a stat requirement with a formula, its weight and the
    // weights that give the same +0 value. A typed +0 value takes the weight
    // that gives it, or opens the list to pick one.
    void RenderWeightedRequirement(Data::Items::ItemDefinition& item, Data::Items::ItemRequirement requirement,
                                   const char* label, WORD& weight, bool byLevel);
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

    // A +0 value being typed, and the weight before it.
    struct TypedZeroValue
    {
        int itemType = 0;
        Data::Items::ItemRequirement requirement = Data::Items::ItemRequirement::Strength;
        int value = 0;
        int weightBefore = 0;
    };
    std::optional<TypedZeroValue> m_typed;
    MuEditor::Items::CRequirementWeightPopup m_weightPopup;
};

#define g_MuItemRequirementsUI CMuItemRequirementsUI::GetInstance()

#endif // _EDITOR
