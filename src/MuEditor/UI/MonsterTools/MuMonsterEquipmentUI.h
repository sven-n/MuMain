#pragma once

#ifdef _EDITOR

#include "MonsterOwnValues.h"
#include "MonsterShownValues.h"

#include "../MuEditor/UI/ItemTools/ItemEditDraft.h"
#include "Data/GameData/MonsterData/MonsterDefinition.h"

#include <optional>
#include <string>

namespace MuEditor::Monsters
{
// The Monster Equipment tool (monsters design, section 9, Q-MN33): what the
// selected monster holds and wears, as the game's character window shows a
// player's (the owner's choice, a paper doll): weapon 1 and 2, helm, armor,
// pants, gloves, boots, wings and helper. A click on a slot picks it; below,
// its item with a search (the items of the slot's groups), level and
// excellent flags, or a body that is no item (a part model), or nothing; a
// Dark Horse helper's horse look. A slot naming an item the item list does
// not have is red (D60). The Map, State and Look rows pick whose equipment is
// shown: the monster's, a map or state variant's, or a random look's (Q-MN83);
// one that has none shows the monster's grey, Set takes it to change, x
// gives the monster's back. Each change is an undo step of the monster
// tools, and living monsters wear it at once (D55).
class MuMonsterEquipmentUI
{
public:
    static MuMonsterEquipmentUI& GetInstance();

    void Render(bool* open);

    // The slots in the order MonsterEquipment holds them.
    enum Slot
    {
        Weapon1,
        Weapon2,
        Helm,
        Armor,
        Pants,
        Gloves,
        Boots,
        Wings,
        Helper,
        SlotCount,
    };

    static constexpr int NoVariant = -1;

private:
    using MonsterDraft = Items::ItemEditDraft<ShownValues::ShownMonster>;

    MuMonsterEquipmentUI();

    void RenderEquipment(Data::Monsters::MonsterDefinition& monster);
    void RenderRows(Data::Monsters::MonsterDefinition& monster);
    // The equipment the rows show (nullptr while a variant has none) and the
    // monster's it falls back to.
    Data::Monsters::MonsterEquipment* EditedEquipment(Data::Monsters::MonsterDefinition& monster) const;
    void RenderDoll(const Data::Monsters::MonsterEquipment& equipment, bool inherited);
    // Whether what is shown is player-shaped (the "player" model), which
    // takes an item in every slot; a monster model takes items in its weapon
    // slots only, and bodies in the others.
    bool IsPlayerShaped(const Data::Monsters::MonsterDefinition& monster) const;
    void RenderSlotEditor(std::optional<Data::Monsters::MonsterItem>& item, Slot slot);
    OwnValues::State EditState() const;
    // The equipment the picked variant wears while it has none: a map
    // variant's state variant the map variant's (if any), else the monster's.
    const Data::Monsters::MonsterEquipment& ParentEquipment(const Data::Monsters::MonsterDefinition& monster) const;
    std::string EquipmentLabel() const;

    MonsterDraft m_draft;
    const ShownValues::ShownMonster* m_shown = nullptr;
    int m_scopeKey = -1;
    int m_mapVariant = NoVariant;
    int m_stateVariant = NoVariant;
    int m_look = NoVariant;
    Slot m_slot = Weapon1;
    // What is shown is a monster model (not player-shaped), this frame.
    bool m_monsterModel = false;
    char m_search[64] = {};
};
} // namespace MuEditor::Monsters

#define g_MuMonsterEquipmentUI MuEditor::Monsters::MuMonsterEquipmentUI::GetInstance()

#endif // _EDITOR
