#include "stdafx.h"

#include "LookChecks.h"

#include "Data/GameData/ItemData/ItemType.h"
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
    // An item effect's entries.
    for (std::size_t i = 0; i < look.itemValuesBefore.size(); ++i)
    {
        const std::string field = LookField(index) + ".itemValuesBefore[" + std::to_string(i) + "]";
        addValues(field, look.itemValuesBefore[i].values);
        if (look.itemValuesBefore[i].onlyWhen.hiddenMesh)
        {
            add(field + ".onlyWhen.hiddenMesh", *look.itemValuesBefore[i].onlyWhen.hiddenMesh);
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
    const bool effect = kind == LookKind::ItemEffect;
    const std::map<std::string, std::string>& values = effect ? model.itemEffectValues : model.renderStyleValues;
    const std::string valuesField = effect ? "itemEffectValues." : "renderStyleValues.";
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
    const bool effect = kind == LookKind::ItemEffect;
    const std::vector<NamedMesh> named = NamedMeshes(look, index);
    bool used = false;
    for (const Items::ItemModelDefinition& model : input.models)
    {
        if (!model.Exists() || (effect ? model.itemEffect : model.renderStyle) != look.name)
        {
            continue;
        }
        used = true;
        // The meshes of the look as this item draws it: its values can name others.
        std::optional<std::vector<NamedMesh>> itemNamed;
        if (!(effect ? model.itemEffectValues : model.renderStyleValues).empty())
        {
            itemNamed = NamedMeshes(CheckItemValues(look, index, source, input, model, kind, issues), index);
        }
        const int meshCount = input.meshCount ? input.meshCount(Items::MakeItemType(model.group, model.number)) : 0;
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
    if (!used)
    {
        issues.push_back({ItemDataIssueSeverity::Warning, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem,
                          LookField(index) + ".name", "no item uses \"" + look.name + "\""});
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
        CheckUsers(looks[i], i, source, input, LookKind::ItemEffect, issues);
    }
}

void CheckLooks(LookKind kind, std::span<const LookDefinition> looks, const std::string& source,
                const RenderStyleCheckInput& input, std::vector<ItemDataIssue>& issues)
{
    if (kind == LookKind::ItemEffect)
    {
        CheckItemEffects(looks, source, input, issues);
    }
    else
    {
        CheckRenderStyles(looks, source, input, issues);
    }
}
} // namespace Data::Looks
