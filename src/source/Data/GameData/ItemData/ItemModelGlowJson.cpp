#include "stdafx.h"

#include "ItemModelGlowJson.h"

#include <array>
#include <limits>
#include <string>

namespace Data::Items::GlowJson
{
namespace
{
using Json::OrderedJson;
using ModelJson::WriteNumber;
using ModelJson::WriteNumbers;

constexpr const char* LevelKey = "level";
constexpr const char* HiddenMeshKey = "hiddenMesh";
constexpr const char* ShineWhiteKey = "shineWhite";
constexpr const char* ShineHiddenMeshKey = "shineHiddenMesh";
constexpr const char* ExcellentKey = "excellent";
constexpr const char* ExcellentMeshKey = "excellentMesh";
constexpr const char* ExcellentMeshWithoutSkinKey = "excellentMeshWithoutSkin";

constexpr int MaxMesh = std::numeric_limits<int>::max();
constexpr double MaxColorValue = 1.0;

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
    if (glow.level)
    {
        json[LevelKey] = *glow.level;
    }
    if (glow.color != ItemGlow::DefaultColor)
    {
        json[ColorKey] = WriteNumbers(glow.color);
    }
    WriteMeshes(glow.meshes, MeshesKey, HiddenMeshKey, json);
    if (glow.shineColor != ItemGlow::DefaultShineColor)
    {
        json[ShineColorKey] = WriteNumbers(glow.shineColor);
    }
    if (glow.shineWhite)
    {
        json[ShineWhiteKey] = true;
    }
    WriteMeshes(glow.shineMeshes, ShineMeshesKey, ShineHiddenMeshKey, json);
    if (glow.ancientColor != ItemGlow::DefaultAncientColor)
    {
        json[AncientColorKey] = WriteNumbers(glow.ancientColor);
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

void ReadColor(ModelJson::ValueReader& reader, const char* key, std::array<double, 3>& color)
{
    std::array<double, 3> value{};
    if (!reader.ReadNumbers(key, value, value.size()))
    {
        return;
    }
    for (const double part : value)
    {
        if (part < 0.0 || part > MaxColorValue)
        {
            reader.Error(key, "must be a list of red, green and blue from 0 to 1");
            return;
        }
    }
    color = value;
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
    ReadOptionalIndex(reader, LevelKey, glow.level, MaxLevel);
    ReadColor(reader, ColorKey, glow.color);
    ReadMeshes(reader, MeshesKey, HiddenMeshKey, glow.meshes);
    ReadColor(reader, ShineColorKey, glow.shineColor);
    reader.ReadBool(ShineWhiteKey, glow.shineWhite);
    ReadMeshes(reader, ShineMeshesKey, ShineHiddenMeshKey, glow.shineMeshes);
    ReadColor(reader, AncientColorKey, glow.ancientColor);
    reader.ReadBool(ExcellentKey, glow.excellent);
    ReadOptionalIndex(reader, ExcellentMeshKey, glow.excellentMesh, MaxMesh);
    ReadOptionalIndex(reader, ExcellentMeshWithoutSkinKey, glow.excellentMeshWithoutSkin, MaxMesh);
    reader.WarnAboutUnknownKeys();
}
} // namespace Data::Items::GlowJson
