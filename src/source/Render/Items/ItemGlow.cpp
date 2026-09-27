#include "stdafx.h"

#include "ItemGlow.h"
#include "ItemDisplay.h"

#include "Core/Globals/_enum.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
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
} // namespace

const ItemGlow& Get(int modelType)
{
    const ItemModelDefinition* model = FindItemModel(modelType);
    return model != nullptr ? model->glow : DefaultGlow;
}

const ItemGlow& GetOfDrawnItem(int modelType)
{
    if (const ItemModelDefinition* model = FindItemModel(modelType))
    {
        return model->glow;
    }
    if (const std::optional<int> item = Display::GetItemOfInventoryModel(modelType))
    {
        return Get(MODEL_ITEM + *item);
    }
    return Get(g_CMonkSystem.EqualItemModelType(modelType));
}

int GetLevel(int modelType, int level)
{
    // The glow of these depends on their level.
    switch (modelType)
    {
    case MODEL_DEVILS_EYE:
    case MODEL_DEVILS_KEY:
    case MODEL_DEVILS_INVITATION:
        return level <= 6 ? level / 2 : 13;
    case MODEL_BOLT:
    case MODEL_ARROWS:
        return level >= 1 ? level * 2 + 1 : 0;
    // The event models of level variants.
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
    return Get(modelType).level.value_or(level);
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
