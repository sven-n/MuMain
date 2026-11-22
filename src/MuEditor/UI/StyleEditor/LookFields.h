#pragma once

#ifdef _EDITOR

#include "Data/GameData/LookData/LookDefinition.h"
#include "Data/GameData/LookData/LookEffects.h"
#include "LookParts.h"

#include <algorithm>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// The fields of a look in the Style Editor (design L19), in designer terms (L8, L9, L10): a layer's kind, motion,
// blend, meshes, texture, light and values, and the item values a look leaves after drawing. A value that changes over
// time names a shape and draws its curve below its fields. Each returns true in the frame the user changed something.
// The numbers are drag fields, every field says in a tooltip what it does, the meshes are the picked item's `parts` by
// name (checkboxes, a dropdown) and a light's color has a picker (design L42); without parts a mesh is a number.
namespace MuEditor::Looks::Fields
{
bool Layer(Data::Looks::Layer& layer, const std::vector<LookPart>& parts = {});
// The look's own options: Not for (doppelgangers, outside a monster's hands) and Draws alone (design L47).
bool LookOptions(Data::Looks::LookDefinition& look);
// `light`: whether the values can set the light; the values an effect sets before the drawing can't.
bool ItemValues(Data::Looks::ItemValues& values, const std::vector<LookPart>& parts = {}, bool light = true);
// An effect's entry before the drawing (design L13): when it holds, the item values it sets, the drawing's
// opacity and the level the item glows like from then on.
bool ValuesBeforeEntry(Data::Looks::ValuesBefore& entry, const std::vector<LookPart>& parts = {});
// An effect's light of the object (design L45): none, or red, green and blue, each a number or a shape.
bool ObjectLight(std::optional<std::array<Data::Looks::Number, 3>>& light);
// One line for an entry of the effect's list: "Only when: glow mesh, glows like level +7".
std::string DescribeValuesBefore(const Data::Looks::ValuesBefore& entry);

// Where an entry's things go, as its fields choose it.
enum class PlaceMode
{
    Bones,
    Between,
    Origin,
};
// The entry's places made `mode`'s: no bones yet, or the item's place, which is one place, so Once for all goes: it is
// hidden there, and the reader refuses it at the item's place, so the look would not load.
void ChangePlaceMode(Data::Looks::OnBones& entry, PlaceMode mode);

// A button of an effect's entry list (the entries before the drawing, the things on bones).
enum class EntryButton
{
    Copy,
    MoveUp,
    MoveDown,
    Remove,
};
// `button` applied to `list`, whose entries are the lines from `first` on, `line` the picked one: it follows a moved
// entry, goes to a copy, and after a removal to the next entry, else the one before, else the line before the list.
// Whether the picked line still shows the entry it showed (a move, or a button that changed nothing): only then does a
// choice kept for the picked entry (Between bones before its first pair) stay with it.
template <typename T>
bool ApplyEntryButton(std::vector<T>& list, std::size_t first, std::size_t& line, EntryButton button)
{
    const std::size_t index = line - first;
    const auto at = [&](std::size_t i) { return list.begin() + static_cast<std::ptrdiff_t>(i); };
    switch (button)
    {
    case EntryButton::Copy:
        list.insert(at(index + 1), list[index]);
        ++line;
        return false;
    case EntryButton::MoveUp:
        if (index > 0)
        {
            std::swap(list[index], list[index - 1]);
            --line;
        }
        return true;
    case EntryButton::MoveDown:
        if (index + 1 < list.size())
        {
            std::swap(list[index], list[index + 1]);
            ++line;
        }
        return true;
    case EntryButton::Remove:
        list.erase(at(index));
        line = list.empty() ? (first == 0 ? 0 : first - 1) : std::min(line, first + list.size() - 1);
        return false;
    }
    return true;
}

// Whether the list's line `line` shows Between bones chosen before its entry's first pair: only the line whose fields
// showed in the last frame (`fieldsLine`), not one clicked in this frame, whose fields haven't shown yet.
inline bool LineShowsBetweenChosen(std::size_t line, std::size_t fieldsLine, bool chosen)
{
    return chosen && line == fieldsLine;
}

// What the fields of an entry of things on bones showed in this frame, for the preview's marks (design L58).
struct OnBonesShown
{
    // Whether its places are pairs of bones (lightning).
    bool between = false;
    // The bone a number field points at (the mouse on it, or typed into).
    std::optional<int> pointedBone;
};
// An entry of an effect's things on bones (design L55, L56, section 7b): where (at bones, between pairs of bones,
// at the item's place) with its bones as number fields, its chance, Together, and its things, each with its kind, its
// type by name from the effect catalogue, SubType, scale, color, a sprite's rotation, lightning's direction and, in a
// group, its own chance. `chosenBetween`: Between bones chosen before the entry's first pair, which the caller keeps
// with the picked entry.
bool OnBonesEntry(Data::Looks::OnBones& entry, OnBonesShown& shown, bool& chosenBetween);
// A thing of things on bones made a thing of `kind`: no type yet (a name is a type of one kind), and without a
// sprite's rotation and lightning's direction, which belong to their kinds and would be written and refused.
void ChangeKind(Data::Looks::BoneThing& thing, Data::Looks::BoneThingKind kind);
// The speed the update plays the item's animation at (design L29); none plays none.
bool AnimationSpeed(std::optional<float>& speed);
// An entry of things on bones on one line: its things, where and its chance.
// `betweenChosen`: Between bones is chosen for the entry while it has no bones yet, as its fields show it.
std::string DescribeOnBones(const Data::Looks::OnBones& entry, bool betweenChosen = false);
// A layer as its effect (design L35, L40): its meshes, then only the fields of its main effect and of what else it
// does, as `effect` describes it (a pulse's strength with its curve, a shine's motion, an overlay's texture).
bool Effect(Data::Looks::Layer& layer, const Data::Looks::LayerEffect& effect, const std::vector<LookPart>& parts = {});

// A mesh the mesh fields offer: its number and its label.
struct MeshOption
{
    int mesh = 0;
    std::string label;

    bool operator==(const MeshOption&) const = default;
};
// The parts in their order ("0 · Crystal", "1 · wand44_R.jpg" without a name), then the listed meshes the parts
// don't have, sorted ("5 (not in the model)").
std::vector<MeshOption> MeshOptions(const std::vector<LookPart>& parts, const std::vector<int>& listed);
// `meshes` with `mesh` added at its end or taken out; the others keep their order.
std::vector<int> WithMesh(std::vector<int> meshes, int mesh, bool in);

// "Pulse", and the gallery's line about it ("" for the base).
const char* EffectName(Data::Looks::EffectKind kind);
const char* EffectNote(Data::Looks::EffectKind kind);
// "Shine (glows, flows)": the main effect and what else the layer does.
std::string EffectLabel(const Data::Looks::LayerEffect& effect);
// "every mesh but 1", "meshes 0, 2".
std::string DescribeMeshes(const Data::Looks::MeshSet& meshes);

// One line written from a layer's fields, for the layer list: "Reflection: slow drift, glow, every mesh but 2; texture,
// glow mesh".
std::string DescribeLayer(const Data::Looks::Layer& layer);
// The item values a look leaves, for the last line of the layer list; empty when it leaves none.
std::string DescribeItemValues(const Data::Looks::ItemValues& values);

// A shape's value at `seconds` (the clock in the editor's terms), as the curve draws it; a random shape gives its
// middle.
double ShapeValueAt(const Data::Looks::Shape& shape, double seconds);

// The curve under a shape's fields (design L43) shows this many seconds of the preview's clock.
inline constexpr double CurveSpan = 10.0;
// The preview's clock in seconds, which the curves mark; the Style Editor sets it each frame before the fields.
void SetCurveTime(double seconds);
// What a curve shows at the clock `now`: the ten seconds from the last whole ten before it, its values from `low`
// (at most 0) to `high` (at least 1), and where `now` is in it.
struct CurveWindow
{
    double start = 0.0;
    double low = 0.0;
    double high = 1.0;
    double now = 0.0;
};
CurveWindow CurveWindowOf(const Data::Looks::Shape& shape, double now);
// The shape's value `seconds` into the window; a scroll from where the window starts, as a texture looks the same
// whatever whole steps it moved before.
double CurveValueAt(const Data::Looks::Shape& shape, const CurveWindow& window, double seconds);
} // namespace MuEditor::Looks::Fields

#endif // _EDITOR
