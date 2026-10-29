#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/ItemTools/ItemEditDraft.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <optional>
#include <string>
#include <vector>

namespace MuEditor::Monsters
{
// The Monster Attacks tool (monsters design, section 9): the attacks of the
// selected monster: which animation each monster skill and player skill
// plays, what any other attack plays (or a draw), and when the monster skill
// is cleared; with the handlers that read them, so where they work. Each
// change is an undo step of the monster tools and reaches the monsters'
// next attack.
class MuMonsterAttacksUI
{
public:
    static MuMonsterAttacksUI& GetInstance();

    void Render(bool* open);

private:
    using MonsterDraft = Items::ItemEditDraft<Data::Monsters::MonsterDefinition>;

    MuMonsterAttacksUI();

    // The map row's choice: all maps, or the map variant at this index.
    static constexpr int AllMaps = -1;

    void RenderAttacks(Data::Monsters::MonsterDefinition& monster, int modelSlot);
    // The attacks the tool edits: the monster's, or the chosen map variant's.
    std::optional<Data::Monsters::MonsterAttacks>& ShownAttacks(Data::Monsters::MonsterDefinition& monster) const;
    // All maps, each map variant, and Add a map variant.
    void RenderMapRow(Data::Monsters::MonsterDefinition& monster);
    // The chosen variant's maps in the map picker, and Remove the map variant.
    void RenderVariantMaps(Data::Monsters::MonsterDefinition& monster);
    void RenderReaders(const Data::Monsters::MonsterDefinition& monster, int modelSlot);
    void RenderEntries(Data::Monsters::MonsterDefinition& monster, int modelSlot, bool monsterSkills);
    void RenderOtherwise(Data::Monsters::MonsterDefinition& monster, int modelSlot);
    void RenderRotation(Data::Monsters::MonsterDefinition& monster, int modelSlot);
    void RenderClearSkill(Data::Monsters::MonsterDefinition& monster);
    void RenderSound(Data::Monsters::MonsterDefinition& monster);
    // A combo of the animations the model has (with a speed above 0) and
    // none; true when one was picked.
    bool AnimationCombo(const char* id, int& animation, int modelSlot);

    MonsterDraft m_draft;
    int m_newMonsterSkill = 0;
    int m_newSkill = 0;
    // The text of the sound field while it is edited, and whether the last
    // one left named no sound.
    std::string m_soundText;
    bool m_soundTextUnknown = false;
    int m_soundMonster = -1;
    int m_soundVariant = AllMaps;
    int m_variant = AllMaps;
    // The monster m_variant is about; another one shows all maps first.
    int m_variantMonster = -1;
};
} // namespace MuEditor::Monsters

#define g_MuMonsterAttacksUI MuEditor::Monsters::MuMonsterAttacksUI::GetInstance()

#endif // _EDITOR
