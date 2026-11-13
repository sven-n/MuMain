#pragma once

#ifdef _EDITOR

#include "Data/GameData/MonsterData/MonsterSetup.h"

#include <algorithm>
#include <cstddef>

#include <functional>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Monster Setup's editors of the setup fields that are lists (monsters
// design, section 9): the spawn effects, one row each (D63, Q-MN36a), the
// spawn buffs, the random skins and the facing by tile (Q-MN38b). Each
// returns true when a control that changes at once changed the list; a text
// or number field calls `afterField` after it, so the window stores the edit
// when the field is left.
namespace MuEditor::Monsters::SetupLists
{
// The setup whose state variants a window's State row shows: the picked map
// variant's (its own list), else the monster's (a negative mapVariant: none
// picked). Monster Setup, Monster Equipment and Monster Preview share it.
template <typename TMonster> auto& SetupOfMap(TMonster& monster, int mapVariant)
{
    return mapVariant >= 0 ? monster.mapVariants[static_cast<std::size_t>(mapVariant)].setup : monster.setup;
}

// The state variants that apply on the picked map variant's maps, as
// GetSetupOnMap takes them: its own when it has some, else the monster's.
// Monster Preview shows these.
template <typename TMonster> const auto& StatesOnMap(const TMonster& monster, int mapVariant)
{
    const auto& own = SetupOfMap(monster, mapVariant).stateVariants;
    return own.empty() ? monster.setup.stateVariants : own;
}

// The picked state (empty: Always) while the shown state variants still have
// it, else Always: a list that changes under the pick (a map variant gets its
// maps or its own state variants) does not turn it into another state.
inline std::string PickedState(const std::vector<Data::Monsters::MonsterStateVariant>& states,
                               const std::string& picked)
{
    const bool listed =
        std::any_of(states.begin(), states.end(),
                    [&](const Data::Monsters::MonsterStateVariant& variant) { return variant.state == picked; });
    return listed ? picked : std::string();
}

// A map variant's own state variants replace the monster's on its maps
// (GetSetupOnMap): while the picked map variant has none, the monster's apply
// there. Their states, comma separated; empty when no map variant is picked,
// it has its own, or the monster has none.
template <typename TMonster> std::string MonsterStatesOnMap(const TMonster& monster, int mapVariant)
{
    std::string names;
    if (mapVariant < 0 || !SetupOfMap(monster, mapVariant).stateVariants.empty())
        return names;
    for (const Data::Monsters::MonsterStateVariant& variant : monster.setup.stateVariants)
    {
        if (!names.empty())
            names += ", ";
        names += variant.state;
    }
    return names;
}

using AfterField = std::function<void()>;

// Which kinds an effect row may have: a spawn's all, an appear's or a
// teleport's a joint, an effect or a particle (the reader refuses the others).
enum class EffectRows
{
    Spawn,
    Appear,
};

// A row per effect, opened to its values (kind, type, sub type, scale,
// count, colour, key, bone, offsets, the eyes' bones, an object's angle),
// with up, down and remove; Add an effect.
bool SpawnEffects(std::vector<Data::Monsters::MonsterSpawnEffect>& effects, const AfterField& afterField,
                  EffectRows rows);
// A row switched to the kind: a new row of it with its first type, keeping
// the count and offset where the kind has them. The old kind's other fields
// go, as the writer would drop them.
Data::Monsters::MonsterSpawnEffect SwitchedKind(const Data::Monsters::MonsterSpawnEffect& effect,
                                                Data::Monsters::MonsterSpawnEffect::Kind kind);
// Whether a row of the rows may have the kind.
bool IsEffectKindAllowed(EffectRows rows, Data::Monsters::MonsterSpawnEffect::Kind kind);
// A box per buff the game knows.
bool SpawnBuffs(std::vector<std::string>& buffs);
// The skins, comma separated ("0, 1, 2").
bool RandomLook(std::vector<int>& skins, const AfterField& afterField);
// A row per tile: X or Y, the tile and the step; Add a tile.
bool FacingByTile(std::vector<Data::Monsters::MonsterTileFacing>& tiles, const AfterField& afterField);
// A row's tile moved to the axis (0: X, 1: Y) with its value: the row then has
// that axis only, as the reader takes it. And the tile on the row's axis.
void SetTileAxis(Data::Monsters::MonsterTileFacing& tile, int axis);
int& TileOnAxis(Data::Monsters::MonsterTileFacing& tile);
// How it comes in (Q-MN38c): the animation, blend, start frame, fade in,
// appear time, sound and the effects where it comes in.
bool Appear(Data::Monsters::MonsterAppear& appear, const AfterField& afterField);
// An NPC's talk steps in order (D66): a row per step, opened to its values
// (a dialog's title, lines and OK, a message, the classes of a quest window),
// with up, down and remove; Add a step. Texts are edited in English.
bool TalkBlocks(std::vector<Data::Monsters::MonsterTalkBlock>& blocks, const AfterField& afterField);

// A castle gate's animations closed, opening and open, and what an open gate
// coming into view plays.
bool Gate(Data::Monsters::MonsterGate& gate);
// A row per hook: the map code by name and its value; Show code says what it
// runs; Add a hook.
bool Hooks(std::vector<Data::Monsters::MonsterHook>& hooks);
// What a hook runs, as the code writes it, for Show code; empty for a name
// that is none.
std::string_view GetHookCode(std::string_view name);

// "0, 1, 2" from the numbers, and the numbers of such a text; nullopt for a
// text that is no list of numbers.
std::string FormatNumbers(const std::vector<int>& numbers);
std::optional<std::vector<int>> ParseNumbers(const std::string& text);
// An eyes row's bones of such a text, as the reader takes them: numbers from
// 0, two, four or six of them; nullopt for another text.
std::optional<std::vector<int>> ParseEyeBones(const std::string& text);
} // namespace MuEditor::Monsters::SetupLists

#endif // _EDITOR
