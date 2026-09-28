#include "stdafx.h"

#include "ItemModelGlowJson.h"
#include "ItemModelValueReader.h"

#include <algorithm>
#include <array>
#include <limits>
#include <string>

namespace Data::Items::GlowJson
{
namespace
{
using Json::OrderedJson;

constexpr const char* ColorKey = "color";
constexpr const char* HiddenMeshKey = "hiddenMesh";
constexpr const char* ShineColorKey = "shineColor";
constexpr const char* ShineWhiteKey = "shineWhite";
constexpr const char* ShineHiddenMeshKey = "shineHiddenMesh";
constexpr const char* AncientColorKey = "ancientColor";
constexpr const char* ExcellentKey = "excellent";
constexpr const char* ExcellentMeshKey = "excellentMesh";
constexpr const char* ExcellentMeshWithoutSkinKey = "excellentMeshWithoutSkin";

constexpr int MaxMesh = std::numeric_limits<int>::max();

// ---------------------------------------------------------------- writing

void WriteMeshes(const ItemGlowMeshes& meshes, const char* meshesKey, const char* hiddenMeshKey, OrderedJson& json)
{
    if (!meshes.only.empty())
    {
        json[meshesKey] = meshes.only;
    }
    if (meshes.hidden)
    {
        json[hiddenMeshKey] = *meshes.hidden;
    }
}

OrderedJson WriteGlow(const ItemGlow& glow)
{
    OrderedJson json = OrderedJson::object();
    if (glow.levels)
    {
        // One number when the level is the same at every item level.
        const auto& levels = *glow.levels;
        const bool sameAtEveryLevel =
            std::all_of(levels.begin(), levels.end(), [&](int level) { return level == levels.front(); });
        if (sameAtEveryLevel)
        {
            json[LevelKey] = levels.front();
        }
        else
        {
            json[LevelKey] = levels;
        }
    }
    if (glow.color != ItemGlow::DefaultColor)
    {
        json[ColorKey] = glow.color;
    }
    WriteMeshes(glow.meshes, MeshesKey, HiddenMeshKey, json);
    if (glow.shineColor != ItemGlow::DefaultShineColor)
    {
        json[ShineColorKey] = glow.shineColor;
    }
    if (glow.shineWhite)
    {
        json[ShineWhiteKey] = true;
    }
    WriteMeshes(glow.shineMeshes, ShineMeshesKey, ShineHiddenMeshKey, json);
    if (glow.ancientColor != ItemGlow::DefaultAncientColor)
    {
        json[AncientColorKey] = glow.ancientColor;
    }
    if (!glow.excellent)
    {
        json[ExcellentKey] = false;
    }
    if (glow.excellentMesh)
    {
        json[ExcellentMeshKey] = *glow.excellentMesh;
    }
    if (glow.excellentMeshWithoutSkin)
    {
        json[ExcellentMeshWithoutSkinKey] = *glow.excellentMeshWithoutSkin;
    }
    return json;
}

// ---------------------------------------------------------------- reading

// One level for all item levels, or one per item level.
void ReadLevels(ModelJson::ItemModelValueReader& reader,
                std::optional<std::array<int, ItemGlow::ItemLevelCount>>& levels)
{
    if (!reader.Has(LevelKey))
    {
        return;
    }
    int level = 0;
    std::vector<int> perItemLevel;
    if (reader.IsNumber(LevelKey))
    {
        if (reader.ReadIndex(LevelKey, level, MaxLevel))
        {
            levels.emplace();
            levels->fill(level);
        }
    }
    else if (reader.ReadIndexes(LevelKey, perItemLevel, MaxLevel))
    {
        if (perItemLevel.size() != ItemGlow::ItemLevelCount)
        {
            reader.Error(LevelKey, "must be one level, or a list of one level for each item level 0 to 15");
            return;
        }
        levels.emplace();
        std::copy(perItemLevel.begin(), perItemLevel.end(), levels->begin());
    }
}

void ReadOptionalIndex(ModelJson::ItemModelValueReader& reader, const char* key, std::optional<int>& value,
                       int maxValue)
{
    int index = 0;
    if (reader.ReadIndex(key, index, maxValue))
    {
        value = index;
    }
}

void ReadMeshes(ModelJson::ItemModelValueReader& reader, const char* meshesKey, const char* hiddenMeshKey,
                ItemGlowMeshes& meshes)
{
    if (reader.Has(meshesKey) && reader.Has(hiddenMeshKey))
    {
        reader.Error(meshesKey, std::string("cannot be used together with ") + hiddenMeshKey);
    }
    reader.ReadIndexes(meshesKey, meshes.only, MaxMesh);
    ReadOptionalIndex(reader, hiddenMeshKey, meshes.hidden, MaxMesh);
}
} // namespace

void Write(const ItemModelDefinition& model, OrderedJson& json)
{
    OrderedJson glow = WriteGlow(model.glow);
    if (!glow.empty())
    {
        json[GlowKey] = std::move(glow);
    }
}

void ForEachMesh(const ItemGlow& glow, const std::function<void(const std::string& field, int mesh)>& visit)
{
    const auto visitIndex = [&](const char* key, const std::optional<int>& mesh)
    {
        if (mesh)
        {
            visit(std::string(GlowKey) + "." + key, *mesh);
        }
    };
    const auto visitMeshes = [&](const char* meshesKey, const char* hiddenMeshKey, const ItemGlowMeshes& meshes)
    {
        for (const int mesh : meshes.only)
        {
            visit(std::string(GlowKey) + "." + meshesKey, mesh);
        }
        visitIndex(hiddenMeshKey, meshes.hidden);
    };

    visitMeshes(MeshesKey, HiddenMeshKey, glow.meshes);
    visitMeshes(ShineMeshesKey, ShineHiddenMeshKey, glow.shineMeshes);
    visitIndex(ExcellentMeshKey, glow.excellentMesh);
    visitIndex(ExcellentMeshWithoutSkinKey, glow.excellentMeshWithoutSkin);
}

void Read(const OrderedJson& json, ItemModelDefinition& model, const ModelJson::ReportIssue& report)
{
    const OrderedJson* object = ModelJson::FindObject(json, GlowKey, report);
    if (object == nullptr)
    {
        return;
    }

    ItemGlow& glow = model.glow;
    ModelJson::ItemModelValueReader reader(*object, GlowKey, report);
    ReadLevels(reader, glow.levels);
    reader.ReadName(ColorKey, glow.color);
    ReadMeshes(reader, MeshesKey, HiddenMeshKey, glow.meshes);
    reader.ReadName(ShineColorKey, glow.shineColor);
    reader.ReadBool(ShineWhiteKey, glow.shineWhite);
    ReadMeshes(reader, ShineMeshesKey, ShineHiddenMeshKey, glow.shineMeshes);
    reader.ReadName(AncientColorKey, glow.ancientColor);
    reader.ReadBool(ExcellentKey, glow.excellent);
    ReadOptionalIndex(reader, ExcellentMeshKey, glow.excellentMesh, MaxMesh);
    ReadOptionalIndex(reader, ExcellentMeshWithoutSkinKey, glow.excellentMeshWithoutSkin, MaxMesh);
    reader.WarnAboutUnknownKeys();
}
} // namespace Data::Items::GlowJson
