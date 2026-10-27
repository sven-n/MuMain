#pragma once

#ifdef _EDITOR

#include "../MuEditor/UI/ItemTools/ItemEditDraft.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"

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

    void RenderAttacks(Data::Monsters::MonsterDefinition& monster, int modelSlot);
    void RenderReaders(const Data::Monsters::MonsterDefinition& monster, int modelSlot);
    void RenderEntries(Data::Monsters::MonsterDefinition& monster, int modelSlot, bool monsterSkills);
    void RenderOtherwise(Data::Monsters::MonsterDefinition& monster, int modelSlot);
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
};
} // namespace MuEditor::Monsters

#define g_MuMonsterAttacksUI MuEditor::Monsters::MuMonsterAttacksUI::GetInstance()

#endif // _EDITOR
