#pragma once

#ifdef _EDITOR

#include "MonsterAttackPreview.h"
#include "MonsterOwnValues.h"
#include "MonsterShownValues.h"

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
// next attack. Play plays an entry's animation in the Monster Preview, Play
// next attack the animation the next other attack plays.
// A monster with several numbers as a whole edits the attacks its numbers
// share; a number's shared attacks are locked until they are made its own
// (Q-MN96).
class MuMonsterAttacksUI
{
public:
    static MuMonsterAttacksUI& GetInstance();

    void Render(bool* open);

    // The animation Play next attack played last, until the counts start
    // again.
    std::optional<int> GetLastPlayed() const
    {
        return m_lastPlayed;
    }

    // When the tool shows the selection of `key` (ShownValues::GetSelectedKey:
    // a number, or a monster as a whole) next, it shows its attacks of all
    // maps (nullopt) or of the map variant at `variant` (Data Checks' problems).
    void ShowMapVariant(int key, std::optional<int> variant);

private:
    using MonsterDraft = Items::ItemEditDraft<ShownValues::ShownMonster>;

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
    // Play next attack, and From the start for its counts.
    void RenderPlayNext(Data::Monsters::MonsterDefinition& monster);
    // The animation once in the Monster Preview, with the attack sounds.
    void RenderPlayButton(Data::Monsters::MonsterDefinition& monster, int animation);
    void PlayInPreview(Data::Monsters::MonsterDefinition& monster, int animation);
    void RenderClearSkill(Data::Monsters::MonsterDefinition& monster);
    void RenderSound(Data::Monsters::MonsterDefinition& monster);
    // A combo of the animations the model has (with a speed above 0) and
    // none; true when one was picked.
    bool AnimationCombo(const char* id, int& animation, int modelSlot);
    // "Death Centurion (145): attacks", or "(3 numbers)" for a whole monster.
    std::string AttacksLabel() const;

    MonsterDraft m_draft;
    // The shown values while the tool draws them, and the states of their
    // attacks and map variants.
    const ShownValues::ShownMonster* m_shown = nullptr;
    OwnValues::State m_attacksState = OwnValues::State::Plain;
    OwnValues::State m_variantsState = OwnValues::State::Plain;
    // The state of the field the shown attacks are in (BeginField), which the
    // Play buttons leave for their row.
    OwnValues::State m_fieldState = OwnValues::State::Plain;
    int m_newMonsterSkill = 0;
    int m_newSkill = 0;
    // The text of the sound field while it is edited, and whether the last
    // one left named no sound.
    std::string m_soundText;
    bool m_soundTextUnknown = false;
    int m_soundKey = -1;
    int m_soundVariant = AllMaps;
    int m_variant = AllMaps;
    // The selection (ShownValues::GetSelectedKey) m_variant is about: a
    // monster as a whole and its first number are two; another one shows all
    // maps first.
    int m_variantKey = -1;
    // Play next attack's counts, for this monster and map row choice.
    AttackPreview::Counts m_counts;
    std::optional<int> m_lastPlayed;
    int m_countsKey = -1;
    int m_countsVariant = AllMaps;
};
} // namespace MuEditor::Monsters

#define g_MuMonsterAttacksUI MuEditor::Monsters::MuMonsterAttacksUI::GetInstance()

#endif // _EDITOR
