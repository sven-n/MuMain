#pragma once

#ifdef _EDITOR

#include "ItemEditDraft.h"
#include "ItemRequirementTable.h"
#include "ItemUsers.h"
#include "ItemValueFormulaText.h"
#include "RequirementWeightPopup.h"

#include "Data/GameData/ItemData/ItemDefinition.h"

#include <array>
#include <optional>
#include <string>

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
    // Its formula beside it.
    void RenderByLevel(Data::Items::ItemDefinition& item, Data::Items::ItemRequirement requirement, const char* label,
                       WORD& baseValue, const MuEditor::Items::FormulaLine& formula);
    void RenderLevelTable(const Data::Items::ItemDefinition& item);
    void RenderClasses(Data::Items::ItemDefinition& item);
    // Stores the draft; a +0 value being typed that needs the weight list
    // (several weights give it, or none) is dropped instead when it is stored
    // without its field being left, which cannot ask.
    void StoreItem(const std::string& label, const Data::Items::ItemDefinition& item);

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
        // The requirement's label, the undo label its edit is stored under.
        const char* label = nullptr;
    };
    // One per requirement: a click from a +0 field into the +0 field of a
    // requirement drawn above it makes that one active, earlier in the same
    // frame, before the first one is reported left.
    std::array<std::optional<TypedZeroValue>, Data::Items::ItemRequirementCount> m_typed;
    // Between the draft's Begin and End: a store then comes from a field. The
    // +0 field drops a value that needs the weight list itself; the other
    // fields of its requirement (the weight, By item level) store under the
    // same label, and their store is not the +0 value's.
    bool m_drawingFields = false;
    MuEditor::Items::CRequirementWeightPopup m_weightPopup;
};

#define g_MuItemRequirementsUI CMuItemRequirementsUI::GetInstance()

#endif // _EDITOR
