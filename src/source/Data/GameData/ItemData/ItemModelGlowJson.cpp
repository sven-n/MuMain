#include "stdafx.h"

#include "ItemModelGlowJson.h"

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
    if (glow.levels.size() == 1)
    {
        json[LevelKey] = glow.levels.front();
    }
    else if (!glow.levels.empty())
    {
        json[LevelKey] = glow.levels;
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
void ReadLevels(ModelJson::ValueReader& reader, std::vector<int>& levels)
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
            levels = {level};
        }
    }
    else if (reader.ReadIndexes(LevelKey, perItemLevel, MaxLevel))
    {
        if (perItemLevel.size() != ItemLevelCount)
        {
            reader.Error(LevelKey, "must be one level, or a list of one level for each item level 0 to 15");
            return;
        }
        levels = std::move(perItemLevel);
    }
}

void ReadOptionalIndex(ModelJson::ValueReader& reader, const char* key, std::optional<int>& value, int maxValue)
{
    int index = 0;
    if (reader.ReadIndex(key, index, maxValue))
    {
        value = index;
    }
}

void ReadMeshes(ModelJson::ValueReader& reader, const char* meshesKey, const char* hiddenMeshKey,
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

void Read(const OrderedJson& json, ItemModelDefinition& model, const ModelJson::ReportIssue& report)
{
    const OrderedJson* object = ModelJson::FindObject(json, GlowKey, report);
    if (object == nullptr)
    {
        return;
    }

    ItemGlow& glow = model.glow;
    ModelJson::ValueReader reader(*object, GlowKey, report);
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
