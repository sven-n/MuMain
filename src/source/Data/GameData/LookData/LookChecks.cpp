#include "stdafx.h"

#include "LookChecks.h"

#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/LookData/BoneThingTypes.h"
#include "Data/GameData/LookData/LookJson.h"

#include <algorithm>
#include <cctype>
#include <map>
#include <optional>
#include <set>
#include <utility>

namespace Data::Looks
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;

// A mesh a look names and the field it is in.
struct NamedMesh
{
    std::string field;
    int mesh = 0;
};

std::string LookField(std::size_t look)
{
    return "looks[" + std::to_string(look) + "]";
}

std::string LayerField(std::size_t look, std::size_t layer)
{
    return LookField(look) + ".layers[" + std::to_string(layer) + "]";
}

// A bone a thing on bones names and the field it is in.
struct NamedBone
{
    std::string field;
    int bone = 0;
};

std::string OnBonesField(std::size_t look, std::size_t entry)
{
    return LookField(look) + ".onBones[" + std::to_string(entry) + "]";
}

// The field of an entry's places: its own, or its group's.
std::string PlacesField(const OnBones& entry, std::size_t look, std::size_t index)
{
    return OnBonesField(look, index) + (entry.together ? ".together" : "");
}

std::vector<NamedBone> NamedBones(const LookDefinition& look, std::size_t index)
{
    std::vector<NamedBone> named;
    for (std::size_t i = 0; i < look.onBones.size(); ++i)
    {
        const OnBones& entry = look.onBones[i];
        const std::string field = PlacesField(entry, index, i);
        for (const int bone : entry.places.bones)
        {
            named.push_back({field + ".bones", bone});
        }
        for (const auto& [from, to] : entry.places.between)
        {
            named.push_back({field + ".between", from});
            named.push_back({field + ".between", to});
        }
    }
    return named;
}

// A type the effect catalogue doesn't have is an error: the game creates nothing of that thing.
void CheckBoneThingTypes(const LookDefinition& look, std::size_t index, const std::string& source,
                         const RenderStyleCheckInput& input, std::vector<ItemDataIssue>& issues)
{
    if (!input.typeExists)
    {
        return;
    }
    for (std::size_t i = 0; i < look.onBones.size(); ++i)
    {
        const OnBones& entry = look.onBones[i];
        for (std::size_t j = 0; j < entry.things.size(); ++j)
        {
            const BoneThing& thing = entry.things[j];
            if (input.typeExists(thing.kind, thing.type))
            {
                continue;
            }
            const std::string field =
                OnBonesField(index, i) + (entry.together ? ".things[" + std::to_string(j) + "]" : "");
            issues.push_back({ItemDataIssueSeverity::Error, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                              field + "." + BoneThingKey(thing.kind),
                              "\"" + thing.type + "\" is no " + BoneThingKey(thing.kind) +
                                  " type of the effect catalogue; it is not created"});
        }
    }
}

// The meshes of a look that an item's model must have; "none" and "every mesh" are none of them.
std::vector<NamedMesh> NamedMeshes(const LookDefinition& look, std::size_t index)
{
    std::vector<NamedMesh> named;
    const auto add = [&named](std::string field, int mesh)
    {
        if (mesh >= 0)
        {
            named.push_back({std::move(field), mesh});
        }
    };
    for (std::size_t i = 0; i < look.layers.size(); ++i)
    {
        const Layer& layer = look.layers[i];
        const std::string field = LayerField(index, i);
        for (const int mesh : layer.meshes.meshes)
        {
            add(field + ".meshes", mesh);
        }
        if (const int* glowMesh = std::get_if<int>(&layer.glowMesh))
        {
            add(field + ".glowMesh", *glowMesh);
        }
        if (layer.scrollingMesh)
        {
            add(field + ".scrollingMesh", *layer.scrollingMesh);
        }
        if (layer.onlyWhen.hiddenMesh)
        {
            add(field + ".onlyWhen.hiddenMesh", *layer.onlyWhen.hiddenMesh);
        }
    }
    const auto addValues = [&add](const std::string& field, const ItemValues& values)
    {
        if (values.glowMesh)
        {
            add(field + ".glowMesh", *values.glowMesh);
        }
        if (values.hiddenMesh)
        {
            add(field + ".hiddenMesh", *values.hiddenMesh);
        }
        if (values.scrollingMesh)
        {
            add(field + ".scrollingMesh", *values.scrollingMesh);
        }
    };
    addValues(LookField(index) + ".itemValuesAfter", look.itemValuesAfter);
    // An effect's entries.
    for (std::size_t i = 0; i < look.valuesBefore.size(); ++i)
    {
        const std::string field = LookField(index) + ".valuesBefore[" + std::to_string(i) + "]";
        addValues(field, look.valuesBefore[i].values);
        if (look.valuesBefore[i].onlyWhen.hiddenMesh)
        {
            add(field + ".onlyWhen.hiddenMesh", *look.valuesBefore[i].onlyWhen.hiddenMesh);
        }
    }
    return named;
}

std::string LowerExtension(const std::string& file)
{
    const std::size_t dot = file.rfind('.');
    if (dot == std::string::npos || file.find('/', dot) != std::string::npos)
    {
        return "";
    }
    std::string extension = file.substr(dot + 1);
    std::ranges::transform(extension, extension.begin(),
                           [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    return extension;
}

void CheckTextures(const LookDefinition& look, std::size_t index, const std::string& source,
                   const RenderStyleCheckInput& input, std::vector<ItemDataIssue>& issues)
{
    for (std::size_t i = 0; i < look.layers.size(); ++i)
    {
        const std::string& texture = look.layers[i].texture;
        if (texture.empty())
        {
            continue;
        }
        const std::string field = LayerField(index, i) + ".texture";
        const std::string file = GetTextureFilePath(texture);
        if (file.empty())
        {
            issues.push_back({ItemDataIssueSeverity::Error, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field,
                              "\"" + texture +
                                  "\" is no .jpg or .tga file, the textures the game loads; the layer is "
                                  "skipped"});
        }
        else if (input.textureExists && !input.textureExists(texture))
        {
            issues.push_back({ItemDataIssueSeverity::Error, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field,
                              "\"" + texture + "\" is not found (" + file + "); the layer is skipped"});
        }
    }
}

// The values an item sets for the look (design L12): each must be one the look lets each item set, and fit its
// field, and a texture it names must be there; a problem is about the item. The look as the item draws it.
LookDefinition CheckItemValues(const LookDefinition& look, std::size_t index, const std::string& source,
                               const RenderStyleCheckInput& input, const Items::ItemModelDefinition& model,
                               LookKind kind, std::vector<ItemDataIssue>& issues)
{
    const bool effect = kind == LookKind::Effect;
    const std::map<std::string, std::string>& values = effect ? model.effectValues : model.renderStyleValues;
    const std::string valuesField = effect ? "effectValues." : "renderStyleValues.";
    const std::set<std::string> names = EachItemNames(look);
    for (const auto& [name, value] : values)
    {
        if (!names.contains(name))
        {
            issues.push_back({ItemDataIssueSeverity::Warning, source, model.group, model.number, valuesField + name,
                              "\"" + look.name + "\" has no value \"" + name + "\" that each item sets; left out"});
        }
    }
    std::vector<ItemDataIssue> resolved;
    LookDefinition itemLook = ResolveForItem(look, values, source, resolved, kind);
    // The textures the look names itself are checked with the look.
    for (std::size_t i = 0; i < itemLook.layers.size() && i < look.layers.size(); ++i)
    {
        if (itemLook.layers[i].texture == look.layers[i].texture)
        {
            itemLook.layers[i].texture.clear();
        }
    }
    CheckTextures(itemLook, 0, source, input, resolved);
    constexpr std::string_view OnlyLook = "looks[0]";
    for (ItemDataIssue& issue : resolved)
    {
        issue.group = model.group;
        issue.number = model.number;
        if (issue.field.starts_with(OnlyLook))
        {
            issue.field = LookField(index) + issue.field.substr(OnlyLook.size());
        }
        issue.message += " (a value of this item)";
        issues.push_back(std::move(issue));
    }
    return itemLook;
}

void CheckUsers(const LookDefinition& look, std::size_t index, const std::string& source,
                const RenderStyleCheckInput& input, LookKind kind, std::vector<ItemDataIssue>& issues)
{
    const bool effect = kind == LookKind::Effect;
    const std::vector<NamedMesh> named = NamedMeshes(look, index);
    const std::vector<NamedBone> bones = NamedBones(look, index);
    bool used = false;
    for (const Items::ItemModelDefinition& model : input.models)
    {
        if (!model.Exists() || (effect ? model.effect : model.renderStyle) != look.name)
        {
            continue;
        }
        used = true;
        // The meshes of the look as this item draws it: its values can name others.
        std::optional<std::vector<NamedMesh>> itemNamed;
        if (!(effect ? model.effectValues : model.renderStyleValues).empty())
        {
            itemNamed = NamedMeshes(CheckItemValues(look, index, source, input, model, kind, issues), index);
        }
        const int itemType = Items::MakeItemType(model.group, model.number);
        if (const int boneCount = input.boneCount ? input.boneCount(itemType) : 0; boneCount > 0)
        {
            for (const NamedBone& bone : bones)
            {
                if (bone.bone >= boneCount)
                {
                    issues.push_back({ItemDataIssueSeverity::Warning, source, model.group, model.number, bone.field,
                                      "bone " + std::to_string(bone.bone) + " is not in this item's model, which has " +
                                          std::to_string(boneCount) + (boneCount == 1 ? " bone" : " bones")});
                }
            }
        }
        const int meshCount = input.meshCount ? input.meshCount(itemType) : 0;
        if (meshCount <= 0)
        {
            continue;
        }
        for (const NamedMesh& mesh : itemNamed ? *itemNamed : named)
        {
            if (mesh.mesh >= meshCount)
            {
                const bool fromItem =
                    std::ranges::none_of(named, [&mesh](const NamedMesh& other)
                                         { return other.field == mesh.field && other.mesh == mesh.mesh; });
                issues.push_back({ItemDataIssueSeverity::Warning, source, model.group, model.number, mesh.field,
                                  "mesh " + std::to_string(mesh.mesh) + " is not in this item's model, which has " +
                                      std::to_string(meshCount) + (meshCount == 1 ? " mesh" : " meshes") +
                                      (fromItem ? " (a value of this item)" : "")});
            }
        }
    }
    used = used || (!effect && std::ranges::any_of(input.partModels, [&](const Items::PartModel& part)
                                                   { return part.renderStyle == look.name; }));
    used =
        used || (input.monsterUses && input.monsterUses(look.name, effect ? LookKind::Effect : LookKind::RenderStyle));
    if (!used)
    {
        issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                          LookField(index) + ".name", "no item or monster uses \"" + look.name + "\""});
    }
}
} // namespace

std::string GetTextureFilePath(const std::string& texture)
{
    const std::string extension = LowerExtension(texture);
    const char* packed = extension == "jpg" ? "OZJ" : extension == "tga" ? "OZT" : nullptr;
    if (packed == nullptr)
    {
        return "";
    }
    return "Data/" + texture.substr(0, texture.size() - extension.size()) + packed;
}

void CheckRenderStyles(std::span<const LookDefinition> looks, const std::string& source,
                       const RenderStyleCheckInput& input, std::vector<ItemDataIssue>& issues)
{
    for (std::size_t i = 0; i < looks.size(); ++i)
    {
        CheckTextures(looks[i], i, source, input, issues);
        CheckUsers(looks[i], i, source, input, LookKind::RenderStyle, issues);
    }
}

void CheckItemEffects(std::span<const LookDefinition> looks, const std::string& source,
                      const RenderStyleCheckInput& input, std::vector<ItemDataIssue>& issues)
{
    for (std::size_t i = 0; i < looks.size(); ++i)
    {
        CheckBoneThingTypes(looks[i], i, source, input, issues);
        CheckUsers(looks[i], i, source, input, LookKind::Effect, issues);
    }
}

void CheckLooks(LookKind kind, std::span<const LookDefinition> looks, const std::string& source,
                const RenderStyleCheckInput& input, std::vector<ItemDataIssue>& issues)
{
    if (kind == LookKind::Effect)
    {
        CheckItemEffects(looks, source, input, issues);
    }
    else
    {
        CheckRenderStyles(looks, source, input, issues);
    }
}
} // namespace Data::Looks
