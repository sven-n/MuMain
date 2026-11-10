#include "stdafx.h"

#include "LookChecks.h"

#include "Data/GameData/ItemData/ItemType.h"

#include <algorithm>
#include <cctype>
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
    }
    const std::string after = LookField(index) + ".itemValuesAfter";
    const ItemValues& values = look.itemValuesAfter;
    if (values.glowMesh)
    {
        add(after + ".glowMesh", *values.glowMesh);
    }
    if (values.hiddenMesh)
    {
        add(after + ".hiddenMesh", *values.hiddenMesh);
    }
    if (values.scrollingMesh)
    {
        add(after + ".scrollingMesh", *values.scrollingMesh);
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

void CheckUsers(const LookDefinition& look, std::size_t index, const std::string& source,
                const RenderStyleCheckInput& input, std::vector<ItemDataIssue>& issues)
{
    const std::vector<NamedMesh> named = NamedMeshes(look, index);
    bool used = false;
    for (const Items::ItemModelDefinition& model : input.models)
    {
        if (!model.Exists() || model.renderStyle != look.name)
        {
            continue;
        }
        used = true;
        const int meshCount = input.meshCount ? input.meshCount(Items::MakeItemType(model.group, model.number)) : 0;
        if (meshCount <= 0)
        {
            continue;
        }
        for (const NamedMesh& mesh : named)
        {
            if (mesh.mesh >= meshCount)
            {
                issues.push_back({ItemDataIssueSeverity::Warning, source, model.group, model.number, mesh.field,
                                  "mesh " + std::to_string(mesh.mesh) + " is not in this item's model, which has " +
                                      std::to_string(meshCount) + (meshCount == 1 ? " mesh" : " meshes")});
            }
        }
    }
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
        CheckUsers(looks[i], i, source, input, issues);
    }
}
} // namespace Data::Looks
