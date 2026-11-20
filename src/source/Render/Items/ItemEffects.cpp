#include "stdafx.h"

#include "ItemEffects.h"
#include "ItemModelTable.h"

#include "LookDrawing.h"

#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/LookData/LookJson.h"
#include "Data/GameData/LookData/LookStore.h"
#include "Engine/Object/w_ObjectInfo.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <string_view>
#include <utility>
#include <vector>

namespace Render
{
DrawLook DrawLook::Of(const OBJECT& o)
{
    return {o.Alpha, o.BlendMesh, o.BlendMeshLight, o.BlendMeshTexCoordU, o.BlendMeshTexCoordV, o.HiddenMesh};
}
} // namespace Render

namespace Render::Items::ItemEffects
{
namespace
{
// The item effect of an item, as the item has it (it can set values of it), with the catalogue types of its things on
// bones.
struct ItemEffectOf
{
    const Data::Looks::LookDefinition* data = nullptr;
    Looks::BoneThingTypes types;
};

// What drawing an item takes from its ItemEffectOf, without copying the types: `types` is null for none.
struct EffectView
{
    const Data::Looks::LookDefinition* data = nullptr;
    const Looks::BoneThingTypes* types = nullptr;
};

// An item effect that is data as an item that sets values of it has it (design L12); those are made again with the
// effect table, after a change of the item models or of the looks.
const Data::Looks::LookDefinition* DataEffectOfItem(const Data::Items::ItemModelDefinition& model,
                                                    const Data::Looks::LookDefinition& effect)
{
    if (model.itemEffectValues.empty() || !Data::Looks::SetsEachItem(effect))
    {
        return &effect;
    }
    static ItemLooks<Data::Looks::LookDefinition> itemEffects;
    // A value that can't be one keeps the look's; Data Checks names the problem.
    std::vector<Data::Items::ItemDataIssue> issues;
    return itemEffects.Keep(
        Data::Items::MakeItemType(model.group, model.number),
        Data::Looks::ResolveForItem(effect, model.itemEffectValues, "", issues, Data::Looks::LookKind::ItemEffect));
}

// The effect of every item type, taken from the item model database after
// each build of it and after each change of the looks that are data.
ItemModelTable<ItemEffectOf> g_itemEffects{
    [](const Data::Items::ItemModelDefinition& model) -> ItemEffectOf
    {
        if (model.itemEffect.empty())
        {
            return {};
        }
        if (const Data::Looks::LookDefinition* data = Data::Looks::g_LookStore.FindItemEffect(model.itemEffect))
        {
            const Data::Looks::LookDefinition* effect = DataEffectOfItem(model, *data);
            return {effect, Looks::ResolveBoneThingTypes(*effect)};
        }
        // Names that are not effects are drawn without one; the
        // model loader reports them.
        return {};
    },
    [] { return Data::Looks::g_LookStore.GetVersion(); }};

#ifdef _EDITOR
// PreviewEffect's model and effect, while one lives.
int s_previewModel = -1;
const Data::Looks::LookDefinition* s_previewEffect = nullptr;
#endif

// The model's effect, or the one the Style Editor previews on it.
EffectView EffectOfModel(int modelType)
{
#ifdef _EDITOR
    if (s_previewEffect != nullptr && modelType == s_previewModel)
    {
        // Its things on bones are the Style Editor's to create (MuEditor::Looks::BoneThingsPreview), not the game's.
        return {s_previewEffect, nullptr};
    }
#endif
    const ItemEffectOf& effect = g_itemEffects.Find(modelType);
    return {effect.data, &effect.types};
}
} // namespace

#ifdef _EDITOR
PreviewEffect::PreviewEffect(int modelType, const Data::Looks::LookDefinition& effect)
{
    s_previewModel = modelType;
    s_previewEffect = &effect;
}

PreviewEffect::~PreviewEffect()
{
    s_previewModel = -1;
    s_previewEffect = nullptr;
}
#endif

bool Exists(std::string_view name)
{
    return Data::Looks::g_LookStore.FindItemEffect(name) != nullptr;
}

std::span<const std::string_view> GetNames()
{
    static std::vector<std::string_view> names;
    static int version = -1;
    if (version != Data::Looks::g_LookStore.GetVersion())
    {
        version = Data::Looks::g_LookStore.GetVersion();
        names.clear();
        for (const Data::Looks::LookDefinition& look : Data::Looks::g_LookStore.GetItemEffects())
        {
            names.emplace_back(look.name);
        }
        std::ranges::sort(names);
    }
    return names;
}

Result Apply(BMD* b, const OBJECT* o, DrawLook& look, int modelType, float& alpha, int& level)
{
    const EffectView effect = EffectOfModel(modelType);
    if (effect.data == nullptr)
    {
        return Result::None;
    }
    Looks::ApplyItemEffect(*effect.data, b, o, look, alpha, level);
    return Result::Applied;
}

const Data::Looks::LookDefinition* EffectOf(int modelType)
{
    return EffectOfModel(modelType).data;
}

bool Spawns(int modelType)
{
    const EffectView effect = EffectOfModel(modelType);
    return effect.data != nullptr && (effect.data->objectLight.has_value() || !effect.data->onBones.empty() ||
                                      effect.data->animationSpeed.has_value());
}

void Spawn(BMD* b, OBJECT* o, int modelType, float (*bones)[3][4])
{
    const EffectView effect = EffectOfModel(modelType);
    if (effect.data != nullptr)
    {
        static const Looks::BoneThingTypes noTypes;
        Looks::SpawnItemEffect(*effect.data, effect.types ? *effect.types : noTypes, b, o, bones);
    }
}
} // namespace Render::Items::ItemEffects
