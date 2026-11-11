#include "stdafx.h"

#include "ItemGlow.h"
#include "ItemDisplay.h"
#include "ItemModelLookup.h"
#include "ItemModelTable.h"
#include "LookDrawing.h"

#include "Core/Globals/_enum.h"
#include "Engine/Object/w_ObjectInfo.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>

namespace Render::Items::Glow
{
namespace
{
using Data::Items::ItemGlow;
using Data::Items::ItemGlowMeshes;
using Data::Items::ItemModelDefinition;

const ItemGlow DefaultGlow;

// The colors of models that are not items (monsters, ...); they stay in code
// until the monsters get data of their own.
const Colors OtherModelColors{{1.0f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, false, {0.1f, 0.6f, 1.0f}};

constexpr int ExcellentGlowRenderType = RENDER_TEXTURE | RENDER_BRIGHT;

// ------------------------------------------------ event models of level variants

// The level variants drawn with event models glow like these levels; they get
// model entries of their own with phase 4d and 12.
constexpr int PlainGlowLevel = 0;
constexpr int BrightGlowLevel = 8;
// Box of Luck +3 (Heart of Love) and +13 (Heart of Dark Lord).
constexpr int HeartOfDarkLordLevel = 13;
constexpr int HeartOfDarkLordGlowLevel = 13;
constexpr int HeartOfLoveGlowLevel = 9;
// Box of Kundun +1 to +5 is Box of Luck +8 to +12; it glows like +1, +3, ... +9.
constexpr int FirstBoxOfKundunLevel = 8;
// Wizard's Ring +1 to +3 glows like +8 to +10.
constexpr int WizardsRingGlowLevelOffset = 7;

// ------------------------------------------------ Phoenix Soul inventory model

// The inventory model of the Phoenix Soul Armor glows differently from its
// item (the last cleanup step of the plan moves it into data): only on its
// first mesh, and the metal pass draws that mesh three times.
constexpr int PhoenixSoulInventoryGlowMesh = 0;
constexpr int PhoenixSoulInventoryMetalDraws = 3;

bool IsPhoenixSoulInventoryModel(int modelType)
{
    return modelType == MODEL_ARMORINVEN_74;
}

void RenderPhoenixSoulInventoryMesh(BMD* b, const OBJECT* o, DrawLook& look, float alpha, int renderType, int draws)
{
    for (int draw = 0; draw < draws; ++draw)
    {
        b->RenderMesh(PhoenixSoulInventoryGlowMesh, renderType, alpha, look.blendMesh, look.blendMeshLight,
                      look.blendMeshU, look.blendMeshV);
    }
}

// ------------------------------------------------ colors

const Colors& GetItemColors(int itemType)
{
    const Colors* colors = g_ItemModelDatabase.FindGlowColors(itemType);
    return colors != nullptr ? *colors : OtherModelColors;
}

void RenderMeshes(BMD* b, const OBJECT* o, DrawLook& look, const ItemGlowMeshes& meshes, float alpha, int renderType,
                  int texture)
{
    if (!meshes.only.empty())
    {
        // Single meshes are drawn with the alpha of the object.
        for (const int mesh : meshes.only)
        {
            b->RenderMesh(mesh, renderType, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
        }
        return;
    }
    b->RenderBody(renderType, alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                  meshes.hidden.value_or(-1), texture);
}
} // namespace

const ItemGlow& Get(int modelType)
{
    const ItemModelDefinition* model = g_ItemModelDatabase.Find(GetItemTypeOfModel(modelType));
    return model != nullptr ? model->glow : DefaultGlow;
}

const Colors& GetColors(int modelType)
{
    if (const std::optional<int> item = Display::GetItemOfInventoryModel(modelType))
    {
        return GetItemColors(*item);
    }
    return GetItemColors(GetItemTypeOfModel(modelType));
}

int GetLevel(int modelType, int level)
{
    switch (modelType)
    {
    case MODEL_EVENT:
    case MODEL_EVENT + 1:
    case MODEL_EVENT + 9:
    case MODEL_EVENT + 15:
        return BrightGlowLevel;
    case MODEL_EVENT + 4:
    case MODEL_EVENT + 5:
    case MODEL_EVENT + 7:
    case MODEL_EVENT + 8:
    case MODEL_EVENT + 12:
    case MODEL_EVENT + 13:
    case MODEL_EVENT + 16:
        return PlainGlowLevel;
    case MODEL_EVENT + 6:
        return level == HeartOfDarkLordLevel ? HeartOfDarkLordGlowLevel : HeartOfLoveGlowLevel;
    case MODEL_EVENT + 10:
        return (level - FirstBoxOfKundunLevel) * 2 + 1;
    case MODEL_EVENT + 11:
        return level - 1;
    case MODEL_EVENT + 14:
        return level + WizardsRingGlowLevelOffset;
    }

    const auto& levels = Get(modelType).levels;
    if (!levels)
    {
        return level;
    }
    // Other levels than 0 to 15 (drawn for effects) take the nearest one.
    return (*levels)[std::clamp(level, 0, static_cast<int>(levels->size()) - 1)];
}

bool HasExcellentGlow(int modelType)
{
    return Get(modelType).excellent;
}

const Data::Effects::Shine* FindShineBelowPlus3(int modelType)
{
    // The name is looked up when the item models or the shines change, not per draw.
    static ItemModelTable<const Data::Effects::Shine*> shines(
        [](const ItemModelDefinition& model) -> const Data::Effects::Shine*
        {
            const std::string& name = model.glow.shineBelowPlus3;
            return name.empty() ? nullptr : g_Shines.Find(name);
        },
        [] { return g_Shines.GetVersion(); });
    return shines.Find(modelType);
}

int RenderTypeOf(const Data::Effects::ShinePass& pass)
{
    return (pass.reflection ? Looks::RenderFlagOf(pass.motion) : RENDER_TEXTURE) | RENDER_BRIGHT;
}

void RenderGlow(BMD* b, const OBJECT* o, DrawLook& look, int modelType, float alpha, int renderType, int texture)
{
    if (IsPhoenixSoulInventoryModel(modelType))
    {
        RenderPhoenixSoulInventoryMesh(b, o, look, alpha, renderType,
                                       (renderType & RENDER_METAL) ? PhoenixSoulInventoryMetalDraws : 1);
        return;
    }
    const ItemGlow& glow = Get(modelType);
    RenderMeshes(b, o, look, glow.meshes, alpha, renderType, texture);
    if (glow.glowAgainInWhite)
    {
        const int mesh = *glow.glowAgainInWhite;
        look.blendMesh = mesh;
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(mesh, renderType, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
    }
}

void RenderShine(BMD* b, const OBJECT* o, DrawLook& look, int modelType, float alpha, int renderType, int texture)
{
    if (IsPhoenixSoulInventoryModel(modelType))
    {
        RenderPhoenixSoulInventoryMesh(b, o, look, alpha, renderType, 1);
        return;
    }
    RenderMeshes(b, o, look, Get(modelType).shineMeshes, alpha, renderType, texture);
}

void RenderExcellentGlow(BMD* b, const OBJECT* o, DrawLook& look, int modelType, float alpha)
{
    if (IsPhoenixSoulInventoryModel(modelType))
    {
        RenderPhoenixSoulInventoryMesh(b, o, look, alpha, ExcellentGlowRenderType, 1);
        return;
    }

    std::optional<int> mesh;
    if (b->HideSkin)
    {
        mesh = Get(o->Type).excellentMeshWithoutSkin;
    }
    if (!mesh)
    {
        mesh = Get(modelType).excellentMesh;
    }

    if (mesh)
    {
        b->RenderMesh(*mesh, ExcellentGlowRenderType, alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
        return;
    }
    b->RenderBody(ExcellentGlowRenderType, alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                  look.blendMeshV);
}
} // namespace Render::Items::Glow
