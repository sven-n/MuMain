#include "stdafx.h"

#include "ItemGlow.h"
#include "ItemDisplay.h"
#include "ItemModelLookup.h"

#include "Core/Globals/_enum.h"
#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Engine/Object/w_ObjectInfo.h"
#include "GameLogic/Social/MonkSystem.h"
#include "Render/Models/ZzzBMD.h"

#include <vector>

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

// The glow colors of every item type (ResolveColors).
std::vector<Colors> g_itemColors;

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

void RenderPhoenixSoulInventoryMesh(BMD* b, OBJECT* o, int renderType, float alpha, int draws)
{
    for (int draw = 0; draw < draws; ++draw)
    {
        b->RenderMesh(PhoenixSoulInventoryGlowMesh, renderType, alpha, o->BlendMesh, o->BlendMeshLight,
                      o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
}

// ------------------------------------------------ colors

Color FindColor(const std::string& name)
{
    // Loading stops at names that are not in the list.
    const Data::Effects::GlowColorValue* value = g_GlowColors.Find(name);
    if (value == nullptr)
    {
        return {};
    }
    return {static_cast<float>((*value)[0]), static_cast<float>((*value)[1]), static_cast<float>((*value)[2])};
}

Colors FindColors(const ItemGlow& glow)
{
    return {FindColor(glow.color), FindColor(glow.shineColor), glow.shineWhite, FindColor(glow.ancientColor)};
}

const Colors& GetItemColors(int itemType)
{
    if (!Data::Items::IsValidItemType(itemType) || g_itemColors.empty())
    {
        return OtherModelColors;
    }
    return g_itemColors[itemType];
}

// The second models of the Rage Fighter gloves.
bool IsGloveSecondModel(int modelType)
{
    return modelType >= MODEL_SWORD_32_LEFT && modelType <= MODEL_SWORD_35_RIGHT;
}

void RenderMeshes(BMD* b, OBJECT* o, const ItemGlowMeshes& meshes, int renderType, float alpha, int texture)
{
    if (!meshes.only.empty())
    {
        // Single meshes are drawn with the alpha of the object.
        for (const int mesh : meshes.only)
        {
            b->RenderMesh(mesh, renderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                          o->BlendMeshTexCoordV);
        }
        return;
    }
    b->RenderBody(renderType, alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
                  meshes.hidden.value_or(-1), texture);
}
} // namespace

void ResolveColors()
{
    const Colors defaults = FindColors(DefaultGlow);
    g_itemColors.assign(MAX_ITEM, defaults);
    const std::span<const ItemModelDefinition> models = g_ItemModelDatabase.GetAllSlots();
    for (size_t itemType = 0; itemType < models.size(); ++itemType)
    {
        if (models[itemType].Exists())
        {
            g_itemColors[itemType] = FindColors(models[itemType].glow);
        }
    }
}

const ItemGlow& Get(int modelType)
{
    const ItemModelDefinition* model = FindItemModel(modelType);
    return model != nullptr ? model->glow : DefaultGlow;
}

const Colors& GetColors(int modelType)
{
    if (const std::optional<int> item = Display::GetItemOfInventoryModel(modelType))
    {
        return GetItemColors(*item);
    }
    if (IsGloveSecondModel(modelType))
    {
        return GetItemColors(g_CMonkSystem.EqualItemModelType(modelType) - MODEL_ITEM);
    }
    return GetItemColors(modelType - MODEL_ITEM);
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

    const std::vector<int>& levels = Get(modelType).levels;
    if (levels.size() == 1)
    {
        return levels.front();
    }
    if (level >= 0 && level < static_cast<int>(levels.size()))
    {
        return levels[level];
    }
    return level;
}

bool HasExcellentGlow(int modelType)
{
    return Get(modelType).excellent;
}

void RenderGlow(BMD* b, OBJECT* o, int modelType, int renderType, float alpha, int texture)
{
    if (IsPhoenixSoulInventoryModel(modelType))
    {
        RenderPhoenixSoulInventoryMesh(b, o, renderType, alpha,
                                       (renderType & RENDER_METAL) ? PhoenixSoulInventoryMetalDraws : 1);
        return;
    }
    RenderMeshes(b, o, Get(modelType).meshes, renderType, alpha, texture);
}

void RenderShine(BMD* b, OBJECT* o, int modelType, int renderType, float alpha, int texture)
{
    if (IsPhoenixSoulInventoryModel(modelType))
    {
        RenderPhoenixSoulInventoryMesh(b, o, renderType, alpha, 1);
        return;
    }
    RenderMeshes(b, o, Get(modelType).shineMeshes, renderType, alpha, texture);
}

void RenderExcellentGlow(BMD* b, OBJECT* o, int modelType, float alpha)
{
    if (IsPhoenixSoulInventoryModel(modelType))
    {
        RenderPhoenixSoulInventoryMesh(b, o, ExcellentGlowRenderType, alpha, 1);
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
        b->RenderMesh(*mesh, ExcellentGlowRenderType, alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                      o->BlendMeshTexCoordV);
        return;
    }
    b->RenderBody(ExcellentGlowRenderType, alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
                  o->BlendMeshTexCoordV);
}
} // namespace Render::Items::Glow
