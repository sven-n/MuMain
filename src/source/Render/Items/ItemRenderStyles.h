#pragma once

#include <array>
#include <optional>
#include <span>
#include <string_view>

#include "Render/Models/DrawLook.h"

class BMD;
class OBJECT;

namespace Data::Looks
{
struct LookDefinition;
}

// The looks of item models that are more than a plain textured model: chrome
// layers, glowing meshes, animated textures, ... Each look is code, a render
// style with a name; the model entry of an item names its style
// (Data/Items/Models, "renderStyle"). Items with the same look share a style.
namespace Render::Items::Styles
{
// Whether there is a render style of this name. The model loader checks the
// names of the model entries with it.
bool Exists(std::string_view name);
// Whether a render style of this name is still in code (ItemRenderStyles.cpp), whether or not the data has one; a name
// moved into data (Data/Effects/RenderStyles.json) is deleted from the code (design L14).
bool ExistsInCode(std::string_view name);

// The names of all render styles, sorted, for the item editor's lists.
std::span<const std::string_view> GetNames();

// Draws the model with the render style of its item (the styles are taken
// from the item model database after each build of it). False when the item
// has no style, or its style is not for this drawing (e.g. a doppelganger);
// the drawing code then draws the model itself.
bool Render(BMD* b, const OBJECT* o, DrawLook& look, int modelType, float alpha, int renderType);

// Whether the model's render style draws alone (design L47): the drawing then draws it right after the item effect,
// and nothing of its own after it.
bool DrawsAlone(int modelType);

#ifdef _EDITOR
// While it lives, the model `modelType` is drawn with `look` instead of its own render style, without the glow pass or
// the shine below +3 of a style in code: the Style Editor's preview of a look before it is applied. The look must
// outlive it; one at a time. Editor builds only: the game's drawing has no preview.
class PreviewLook
{
public:
    PreviewLook(int modelType, const Data::Looks::LookDefinition& look);
    // The model drawn with no render style at all, as the model of an item that names none: the Style Editor's
    // "Without a look".
    explicit PreviewLook(int modelType);
    ~PreviewLook();
    PreviewLook(const PreviewLook&) = delete;
    PreviewLook& operator=(const PreviewLook&) = delete;
};
#endif

// Whether the item's style spawns lights or animates where it is drawn.
bool Spawns(int modelType);
// Spawns them from the model's pose, in the update; b is placed where the model is drawn.
void Spawn(BMD* b, OBJECT* o, int modelType, float (*bones)[3][4]);

// The lights and tint the helper NPCs' plate armor keeps on them, for the update to spawn while it is drawn.
void SpawnHelperNpcPlateLights(OBJECT* o);
} // namespace Render::Items::Styles
