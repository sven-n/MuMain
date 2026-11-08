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

// Draws the glow pass of items +7 and up for the few render styles that glow
// differently from their "glow" values. False for the others.
bool RenderGlow(BMD* b, const OBJECT* o, DrawLook& look, int modelType, float alpha, int renderType, int texture);

// How a few render styles shine below +3: the light of the item scaled, the
// model drawn, then two shine passes (RenderPartObjectBodyColor2).
struct ShineBelowPlus3
{
    struct Pass
    {
        float alpha;
        int renderType;
        float bright;
    };

    // Scales the light of the item before the model is drawn; none: the light
    // stays as it is.
    std::optional<float> light;
    std::array<Pass, 2> passes;
};

// The shine below +3 of the render style of the item; nullptr for the others,
// which are drawn plainly below +3.
const ShineBelowPlus3* FindShineBelowPlus3(int modelType);
} // namespace Render::Items::Styles
