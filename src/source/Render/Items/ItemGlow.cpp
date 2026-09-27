#include "stdafx.h"

#include "ItemGlow.h"
#include "ItemDisplay.h"

#include "Core/Globals/_enum.h"
#include "Data/GameData/EffectData/GlowColors.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Engine/Object/w_ObjectInfo.h"
#include "GameLogic/Social/MonkSystem.h"
#include "Render/Models/ZzzBMD.h"

namespace Render::Items::Glow
{
namespace
{
using Data::Items::ItemGlow;
using Data::Items::ItemGlowMeshes;
using Data::Items::ItemModelDefinition;

const ItemGlow DefaultGlow;

constexpr int ExcellentGlowRenderType = RENDER_TEXTURE | RENDER_BRIGHT;

const ItemModelDefinition* FindItemModel(int modelType)
{
    return g_ItemModelDatabase.Find(modelType - MODEL_ITEM);
}

// The glow colors of every item type, looked up in the glow color list again
// when the models or the list change.
struct ColorCache
{
    int modelsVersion = -1;
    int colorListVersion = -1;
    Colors defaults;
    std::vector<Colors> items;
};

// Names that are not in the list (loading reports them) have no color.
Color FindColor(const std::string& name)
{
    const Data::Effects::GlowColorValue* value = g_GlowColors.Find(name);
    if (value == nullptr)
    {
        return {};
    }
    return {static_cast<float>((*value)[0]), static_cast<float>((*value)[1]), static_cast<float>((*value)[2])};
}

Colors FindColors(const ItemGlow& glow)
{
    return {FindColor(glow.color), FindColor(glow.shineColor), FindColor(glow.ancientColor)};
}

const ColorCache& GetColorCache()
{
    static ColorCache cache;
    if (cache.modelsVersion != g_ItemModelDatabase.GetVersion() || cache.colorListVersion != g_GlowColors.GetVersion())
    {
        cache.defaults = FindColors(DefaultGlow);
        cache.items.assign(MAX_ITEM, cache.defaults);
        const std::span<const ItemModelDefinition> models = g_ItemModelDatabase.GetAllSlots();
        for (size_t itemType = 0; itemType < models.size(); ++itemType)
        {
            if (models[itemType].Exists())
            {
                cache.items[itemType] = FindColors(models[itemType].glow);
            }
        }
        cache.modelsVersion = g_ItemModelDatabase.GetVersion();
        cache.colorListVersion = g_GlowColors.GetVersion();
    }
    return cache;
}
} // namespace

const ItemGlow& Get(int modelType)
{
    const ItemModelDefinition* model = FindItemModel(modelType);
    return model != nullptr ? model->glow : DefaultGlow;
}

const Colors& GetColors(int modelType)
{
    const ColorCache& cache = GetColorCache();
    const int itemType = modelType - MODEL_ITEM;
    return Data::Items::IsValidItemType(itemType) ? cache.items[itemType] : cache.defaults;
}

const Colors& GetColorsOfDrawnItem(int modelType)
{
    if (const std::optional<int> item = Display::GetItemOfInventoryModel(modelType))
    {
        return GetColors(MODEL_ITEM + *item);
    }
    if (FindItemModel(modelType) == nullptr)
    {
        return GetColors(g_CMonkSystem.EqualItemModelType(modelType));
    }
    return GetColors(modelType);
}

int GetLevel(int modelType, int level)
{
    // The event models of level variants; they get model entries of their own
    // with phase 4d and 12.
    switch (modelType)
    {
    case MODEL_EVENT:
    case MODEL_EVENT + 1:
    case MODEL_EVENT + 9:
    case MODEL_EVENT + 15:
        return 8;
    case MODEL_EVENT + 4:
    case MODEL_EVENT + 5:
    case MODEL_EVENT + 7:
    case MODEL_EVENT + 8:
    case MODEL_EVENT + 12:
    case MODEL_EVENT + 13:
    case MODEL_EVENT + 16:
        return 0;
    case MODEL_EVENT + 6:
        return level == 13 ? 13 : 9;
    case MODEL_EVENT + 10:
        return (level - 8) * 2 + 1;
    case MODEL_EVENT + 11:
        return level - 1;
    case MODEL_EVENT + 14:
        return level + 7;
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

void RenderExcellentGlow(BMD* b, OBJECT* o, int modelType, float alpha)
{
    std::optional<int> mesh;
    if (b->HideSkin)
    {
        mesh = Get(o->Type).excellentMeshWithoutSkin;
    }
    if (!mesh)
    {
        // The inventory model of the Phoenix Soul Armor glows on its first mesh.
        mesh = modelType == MODEL_ARMORINVEN_74 ? std::optional<int>(0) : Get(modelType).excellentMesh;
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
