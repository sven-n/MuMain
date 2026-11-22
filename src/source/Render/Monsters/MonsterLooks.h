#pragma once

#include <cstdint>
#include <string>
#include <string_view>

class CHARACTER;
class OBJECT;

namespace Data::Looks
{
struct LookDefinition;
}

namespace Data::Monsters
{
struct MonsterSetup;
}

// The looks monsters and NPCs are drawn with (MN8, docs/monster-data.md "Looks"): a render style and an effect of
// Data/Effects by name. A model entry of Models.json names the looks of its monsters; a monster entry, one of its
// numbers, a map variant or a state variant names others in their place (ML1, ML3). The names are resolved at spawn:
// the character's object keeps a small number for each, and the drawing finds the look of that number in g_LookStore
// without looking a name up (a change of the looks finds them again).
namespace Render::Monsters::Looks
{
// The number of a look name the object keeps; NoLook for none.
using LookId = std::uint16_t;
constexpr LookId NoLook = 0;

// The name that draws a monster without the look of its model.
constexpr std::string_view NoneName = "none";

// The number of a name, the same for the game's run; NoLook for an empty name or "none".
LookId IdOf(std::string_view name);
// The name of a number; empty for NoLook.
const std::string& NameOf(LookId id);

// The render style or effect of the number as the looks are now; nullptr for NoLook or a name no look file has.
const Data::Looks::LookDefinition* RenderStyleOf(LookId id);
const Data::Looks::LookDefinition* EffectOf(LookId id);

// The names a setup and the model of `modelSlot` give: the setup's over the model's (ML1).
std::string_view RenderStyleName(const Data::Monsters::MonsterSetup& setup, int modelSlot);
std::string_view EffectName(const Data::Monsters::MonsterSetup& setup, int modelSlot);

// Sets the looks of the character's object from its setup and the model it is drawn with.
void Resolve(CHARACTER& character, const Data::Monsters::MonsterSetup& setup);

// The object's render style and effect as the looks are now; nullptr for none.
const Data::Looks::LookDefinition* RenderStyleOf(const OBJECT& object);
const Data::Looks::LookDefinition* EffectOf(const OBJECT& object);
} // namespace Render::Monsters::Looks
