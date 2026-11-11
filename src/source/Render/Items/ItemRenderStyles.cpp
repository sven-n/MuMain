#include "stdafx.h"

#include "Core/Utilities/Random.h"
#include "ItemRenderStyles.h"
#include "ItemModelTable.h"
#include "LookDrawing.h"

#include "Core/Globals/_enum.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/ItemData/PartModelSlots.h"
#include "Data/GameData/LookData/LookJson.h"
#include "Data/GameData/LookData/LookStore.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/w_ObjectInfo.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <iterator>
#include <optional>
#include <string_view>
#include <utility>
#include <vector>

namespace Render::Items::Styles
{
namespace
{
// A recipe draws the model and returns true, or returns false when the style
// is not for this drawing (the drawing code then draws the model plainly).
using StyleFunction = bool (*)(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType);

struct RenderStyle
{
    constexpr RenderStyle(const char* styleName, StyleFunction recipe) : name(styleName), render(recipe) {}

    // A render style that is data (Data/Effects/RenderStyles.json); the look outlives the entry.
    explicit RenderStyle(const Data::Looks::LookDefinition& look) : name(look.name.c_str()), data(&look) {}
    bool Render(BMD* b, const OBJECT* o, DrawLook& look, int modelType, float alpha, int renderType) const
    {
        if (data != nullptr)
        {
            return Looks::DrawRenderStyle(*data, b, o, look, alpha, renderType);
        }
        return render(b, o, look, modelType, alpha, renderType);
    }

    const char* name;
    StyleFunction render = nullptr;
    const Data::Looks::LookDefinition* data = nullptr;
};

// ------------------------------------------------ the recipes

bool RenderPumpkinOfLuck(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  look.hiddenMesh);
    b->RenderMesh(1, RENDER_DARK | RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight, -WorldTime * 0.0002f,
                  look.blendMeshV, BITMAP_CHROME);
    return true;
}

void SpawnPumpkinOfLuck(BMD* b, OBJECT* o, vec34_t* bones)
{
    vec3_t vPos, vRelativePos;
    Vector(0.f, 0.f, 0.f, vRelativePos);
    b->TransformPosition(bones[8], vRelativePos, vPos, true);
    float fLumi = (sinf(WorldTime * 0.004f) + 1.0f) * 0.05f;
    Vector(0.8f + fLumi, 0.8f + fLumi, 0.3f + fLumi, o->Light);
    CreateSprite(BITMAP_LIGHT, vPos, 1.5f, o->Light, o, 0.5f);
    b->TransformPosition(bones[10], vRelativePos, vPos, true);
    CreateSprite(BITMAP_LIGHT, vPos, 0.5f, o->Light, o, 0.5f);
    b->TransformPosition(bones[11], vRelativePos, vPos, true);
    CreateSprite(BITMAP_LIGHT, vPos, 0.5f, o->Light, o, 0.5f);
}

bool RenderCursedCastleWater(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    b->RenderBody(RenderType, Alpha, look.blendMesh, look.blendMeshLight * 1.5f, look.blendMeshU, look.blendMeshV);
    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, Alpha, look.blendMesh, look.blendMeshLight / 4.0f, WorldTime * 0.005f,
                  -WorldTime * 0.005f, 0);
    return true;
}

void SpawnCursedCastleWater(BMD* b, OBJECT* o, vec34_t* bones)
{
    vec3_t Light;
    vec3_t vRelativePos, vWorldPos;
    Vector(0.f, 0.f, 0.f, vRelativePos);
    Vector(1.f, 0.f, 0.0f, Light);

    b->TransformPosition(bones[1], vRelativePos, vWorldPos, true);
    CreateSprite(BITMAP_LIGHT, vWorldPos, 3.f, Light, o, 0.f);
    b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, 2.f / 7.f, o->Position, o->Angle);
}

bool RenderHelperNpcPlate(BMD* b, const OBJECT* o, DrawLook& look, int Type, float Alpha, int RenderType)
{
    // Only on the helper NPCs (Luke and Leo the Helper, Helper Ellen), the only
    // characters with the flag of the PC room look; otherwise drawn plainly.
    if (o->m_bpcroom != TRUE)
    {
        return false;
    }
    vec3_t Light;
    VectorCopy(b->BodyLight, Light);
    Vector(0.9f, 0.7f, 1.0f, b->BodyLight);

    b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, look.alpha, 1, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV, BITMAP_FENRIR_THUNDER);
    b->RenderMesh(0, RENDER_BRIGHT | RENDER_METAL, look.alpha, 1, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  BITMAP_FENRIR_THUNDER);
    VectorCopy(Light, b->BodyLight);
    return true;
}

// ------------------------------------------------ the names

const RenderStyle RenderStyles[] = {
    {"pumpkinOfLuck", RenderPumpkinOfLuck},
    {"cursedCastleWater", RenderCursedCastleWater},
    {"helperNpcPlate", RenderHelperNpcPlate},
};

using SpawnFunction = void (*)(BMD* b, OBJECT* o, vec34_t* bones);

// The styles whose items spawn lights or animate where they are drawn.
const struct
{
    const char* name;
    SpawnFunction spawn;
} StyleSpawns[] = {
    {"pumpkinOfLuck", SpawnPumpkinOfLuck},
    {"cursedCastleWater", SpawnCursedCastleWater},
};

ItemModelTable<SpawnFunction> g_itemSpawns{[](const Data::Items::ItemModelDefinition& model)
                                           {
                                               for (const auto& style : StyleSpawns)
                                                   if (model.renderStyle == style.name)
                                                       return style.spawn;
                                               return SpawnFunction{};
                                           }};

// The render styles that are data, one entry per look of g_LookStore; built again when the looks change.
const std::vector<RenderStyle>& DataStyles()
{
    static std::vector<RenderStyle> styles;
    static int version = -1;
    if (version != Data::Looks::g_LookStore.GetVersion())
    {
        version = Data::Looks::g_LookStore.GetVersion();
        styles.clear();
        for (const Data::Looks::LookDefinition& look : Data::Looks::g_LookStore.GetRenderStyles())
        {
            styles.emplace_back(look);
        }
    }
    return styles;
}

// A style that is data first, then one still in code (design L14).
const RenderStyle* FindStyle(std::string_view name)
{
    if (const Data::Looks::LookDefinition* look = Data::Looks::g_LookStore.FindRenderStyle(name))
    {
        return &DataStyles()[static_cast<size_t>(look - Data::Looks::g_LookStore.GetRenderStyles().data())];
    }
    const auto found = std::find_if(std::begin(RenderStyles), std::end(RenderStyles),
                                    [name](const RenderStyle& style) { return style.name == name; });
    return found != std::end(RenderStyles) ? found : nullptr;
}

// A look as the item that sets values of it draws it (design L12), and its style.
struct ItemStyle
{
    explicit ItemStyle(Data::Looks::LookDefinition resolved) : look(std::move(resolved)), style(look) {}

    Data::Looks::LookDefinition look;
    RenderStyle style;
};

// The style an item is drawn with: its look's, or for an item that sets values of a look in data the look with them.
// Those are made again with the style table, after a change of the item models or of the looks.
const RenderStyle* StyleOfItem(const Data::Items::ItemModelDefinition& model)
{
    const RenderStyle* style = FindStyle(model.renderStyle);
    if (style == nullptr || style->data == nullptr || model.renderStyleValues.empty() ||
        !Data::Looks::SetsEachItem(*style->data))
    {
        return style;
    }
    static ItemLooks<ItemStyle> itemStyles;
    // A value that can't be one draws as the look has it; Data Checks names the problem.
    std::vector<Data::Items::ItemDataIssue> issues;
    const ItemStyle* itemStyle =
        itemStyles.Keep(Data::Items::MakeItemType(model.group, model.number),
                        Data::Looks::ResolveForItem(*style->data, model.renderStyleValues, "", issues));
    // The textures the item's values name.
    Looks::PrepareTextures(itemStyle->look);
    return &itemStyle->style;
}

// The render style of a part model's entry (design L28), for its slot; nullptr for other slots and for a part model
// drawn plainly. The names are looked up when the part models or the looks change, not per draw.
const RenderStyle* PartModelStyle(int modelType)
{
    constexpr std::size_t SlotCount = std::size(Data::Items::PartModelSlots);
    static std::array<const RenderStyle*, SlotCount> styles{};
    static std::pair<int, int> builtFor{-1, -1};
    const std::pair<int, int> versions{g_ItemModelDatabase.GetVersion(), Data::Looks::g_LookStore.GetVersion()};
    if (builtFor != versions)
    {
        builtFor = versions;
        for (std::size_t slot = 0; slot < SlotCount; ++slot)
        {
            const Data::Items::PartModel* part =
                g_ItemModelDatabase.FindPartModel(Data::Items::PartModelSlots[slot].name);
            styles[slot] = part != nullptr && !part->renderStyle.empty() ? FindStyle(part->renderStyle) : nullptr;
        }
    }
    for (std::size_t slot = 0; slot < SlotCount; ++slot)
    {
        if (Data::Items::PartModelSlots[slot].modelType == modelType)
        {
            return styles[slot];
        }
    }
    return nullptr;
}

// The render style of every item type, taken from the item model database
// after each build of it and after each change of the looks that are data.
ItemModelTable<const RenderStyle*> g_itemStyles{[](const Data::Items::ItemModelDefinition& model)
                                                {
                                                    // Names that are not styles are drawn plainly; the model
                                                    // loader reports them.
                                                    return model.renderStyle.empty() ? nullptr : StyleOfItem(model);
                                                },
                                                [] { return Data::Looks::g_LookStore.GetVersion(); }};
} // namespace

bool Exists(std::string_view name)
{
    return FindStyle(name) != nullptr;
}

bool ExistsInCode(std::string_view name)
{
    return std::ranges::any_of(RenderStyles, [name](const RenderStyle& style) { return style.name == name; });
}

std::span<const std::string_view> GetNames()
{
    static std::vector<std::string_view> names;
    static int version = -1;
    if (version != Data::Looks::g_LookStore.GetVersion())
    {
        version = Data::Looks::g_LookStore.GetVersion();
        names.clear();
        for (const RenderStyle& style : RenderStyles)
        {
            names.emplace_back(style.name);
        }
        for (const Data::Looks::LookDefinition& look : Data::Looks::g_LookStore.GetRenderStyles())
        {
            names.emplace_back(look.name);
        }
        std::ranges::sort(names);
        names.erase(std::unique(names.begin(), names.end()), names.end());
    }
    return names;
}

void SpawnHelperNpcPlateLights(OBJECT* o)
{
    BMD* b = &Models[MODEL_PLATE_ARMOR];
    b->BodyScale = o->Scale;
    VectorCopy(o->Position, b->BodyOrigin);

    vec3_t EndRelative, EndPos;
    Vector(0.f, 0.f, 0.f, EndRelative);

    b->TransformPosition(o->BoneTransform[0], EndRelative, EndPos, true);

    Vector(0.4f, 0.6f, 0.8f, o->Light);
    CreateSprite(BITMAP_LIGHT, EndPos, 6.0f, o->Light, o, 0.5f);

    float Luminosity;
    Luminosity = sinf(WorldTime * 0.05f) * 0.4f + 0.9f;
    Vector(Luminosity * 0.3f, Luminosity * 0.5f, Luminosity * 0.8f, o->Light);
    CreateSprite(BITMAP_LIGHT, EndPos, 2.0f, o->Light, o);
}

namespace
{
#ifdef _EDITOR
// PreviewLook's model and look, while one lives; the look is nullptr for the model drawn with no render style.
struct Preview
{
    bool active = false;
    int modelType = -1;
    const Data::Looks::LookDefinition* look = nullptr;
};
Preview s_preview;
#endif

// The look the Style Editor's preview draws the model with (PreviewLook): nullopt while it shows no preview of the
// model, nullptr for the model drawn with no render style. Only the editor previews looks.
std::optional<const Data::Looks::LookDefinition*> PreviewOf(int modelType)
{
#ifdef _EDITOR
    if (s_preview.active && s_preview.modelType == modelType)
    {
        return s_preview.look;
    }
#else
    static_cast<void>(modelType);
#endif
    return std::nullopt;
}
} // namespace

#ifdef _EDITOR
PreviewLook::PreviewLook(int modelType, const Data::Looks::LookDefinition& look)
{
    s_preview = {true, modelType, &look};
}

PreviewLook::PreviewLook(int modelType)
{
    s_preview = {true, modelType, nullptr};
}

PreviewLook::~PreviewLook()
{
    s_preview = {};
}
#endif

bool Render(BMD* b, const OBJECT* o, DrawLook& look, int modelType, float alpha, int renderType)
{
    if (const std::optional<const Data::Looks::LookDefinition*> preview = PreviewOf(modelType))
    {
        if (*preview == nullptr)
        {
            // No render style: the drawing code draws the model itself.
            return false;
        }
        return Looks::DrawRenderStyle(**preview, b, o, look, alpha, renderType);
    }
    const RenderStyle* style = g_itemStyles.Find(modelType);
    if (style == nullptr)
    {
        style = PartModelStyle(modelType);
    }
    return style != nullptr && style->Render(b, o, look, modelType, alpha, renderType);
}

bool DrawsAlone(int modelType)
{
    if (const std::optional<const Data::Looks::LookDefinition*> preview = PreviewOf(modelType))
    {
        return *preview != nullptr && (*preview)->drawsAlone;
    }
    const RenderStyle* style = g_itemStyles.Find(modelType);
    if (style == nullptr)
    {
        style = PartModelStyle(modelType);
    }
    return style != nullptr && style->data != nullptr && style->data->drawsAlone;
}

bool Spawns(int modelType)
{
    return g_itemSpawns.Find(modelType) != nullptr;
}

void Spawn(BMD* b, OBJECT* o, int modelType, float (*bones)[3][4])
{
    if (const SpawnFunction spawn = g_itemSpawns.Find(modelType))
        spawn(b, o, bones);
}
} // namespace Render::Items::Styles
