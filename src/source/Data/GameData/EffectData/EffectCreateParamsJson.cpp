#include "stdafx.h"

#include "EffectCreateParamsJson.h"

#include "Data/GameData/ItemData/ItemModelValueReader.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace Data::Effects
{
namespace
{
using Items::Json::OrderedJson;
using Items::ModelJson::ItemModelValueReader;

namespace Keys
{
constexpr const char* LifeTime = "lifeTime";
constexpr const char* Scale = "scale";
constexpr const char* Velocity = "velocity";
constexpr const char* Gravity = "gravity";
constexpr const char* HiddenMesh = "hiddenMesh";
constexpr const char* BlendMesh = "blendMesh";
constexpr const char* BlendMeshLight = "blendMeshLight";
constexpr const char* Alpha = "alpha";
constexpr const char* CopyLightToDirection = "copyLightToDirection";
} // namespace Keys

// hiddenMesh is a mesh number and blendMesh the texture number of meshes,
// both shorts in the model; -1 is none, a hiddenMesh of -2 hides the whole
// model and a blendMesh of -2 means every mesh.
constexpr int SmallestMeshNumber = -2;
constexpr int LargestMeshNumber = std::numeric_limits<short>::max();

// The game keeps the values as float.
constexpr double LargestValue = std::numeric_limits<float>::max();
constexpr const char* TooLarge = "is too large for the game";

bool FitsFloat(double value)
{
    return std::abs(value) <= LargestValue;
}

void ReadValue(ItemModelValueReader& reader, const char* key, std::optional<double>& value)
{
    double number = 0.0;
    if (!reader.ReadNumber(key, number, false))
    {
        return;
    }
    if (!FitsFloat(number))
    {
        reader.Error(key, TooLarge);
        return;
    }
    value = number;
}

void ReadAlpha(ItemModelValueReader& reader, std::optional<double>& value)
{
    double alpha = 0.0;
    if (!reader.ReadNumber(Keys::Alpha, alpha, false))
    {
        return;
    }
    if (alpha < 0.0 || alpha > 1.0)
    {
        reader.Error(Keys::Alpha, "must be from 0 to 1");
        return;
    }
    value = alpha;
}

void ReadLight(ItemModelValueReader& reader, std::optional<std::array<double, 3>>& value)
{
    std::array<double, 3> light{};
    if (!reader.ReadNumbers(CreateLightKey, light, light.size()))
    {
        return;
    }
    if (!std::all_of(light.begin(), light.end(), FitsFloat))
    {
        reader.Error(CreateLightKey, TooLarge);
        return;
    }
    value = light;
}

void ReadMeshNumber(ItemModelValueReader& reader, const char* key, std::optional<int>& value)
{
    int number = 0;
    if (reader.ReadInteger(key, number, SmallestMeshNumber, LargestMeshNumber))
    {
        value = number;
    }
}

void WriteValue(OrderedJson& json, const char* key, const std::optional<double>& value)
{
    if (value)
    {
        json[key] = Items::ModelJson::WriteNumber(*value);
    }
}

void WriteMeshNumber(OrderedJson& json, const char* key, const std::optional<int>& value)
{
    if (value)
    {
        json[key] = *value;
    }
}
} // namespace

EffectCreateParams ReadEffectCreateParams(const OrderedJson& json, const std::string& objectKey,
                                          const Items::ModelJson::ReportIssue& report)
{
    ItemModelValueReader reader(json, objectKey, report);
    EffectCreateParams params;
    ReadValue(reader, Keys::LifeTime, params.lifeTime);
    ReadValue(reader, Keys::Scale, params.scale);
    ReadValue(reader, Keys::Velocity, params.velocity);
    ReadValue(reader, Keys::Gravity, params.gravity);
    ReadMeshNumber(reader, Keys::HiddenMesh, params.hiddenMesh);
    ReadMeshNumber(reader, Keys::BlendMesh, params.blendMesh);
    ReadValue(reader, Keys::BlendMeshLight, params.blendMeshLight);
    ReadAlpha(reader, params.alpha);
    ReadLight(reader, params.light);
    reader.ReadBool(Keys::CopyLightToDirection, params.copyLightToDirection);
    reader.WarnAboutUnknownKeys();
    return params;
}

OrderedJson WriteEffectCreateParams(const EffectCreateParams& params)
{
    OrderedJson json = OrderedJson::object();
    WriteValue(json, Keys::LifeTime, params.lifeTime);
    WriteValue(json, Keys::Scale, params.scale);
    WriteValue(json, Keys::Velocity, params.velocity);
    WriteValue(json, Keys::Gravity, params.gravity);
    WriteMeshNumber(json, Keys::HiddenMesh, params.hiddenMesh);
    WriteMeshNumber(json, Keys::BlendMesh, params.blendMesh);
    WriteValue(json, Keys::BlendMeshLight, params.blendMeshLight);
    WriteValue(json, Keys::Alpha, params.alpha);
    if (params.light)
    {
        json[CreateLightKey] = Items::ModelJson::WriteNumbers(*params.light);
    }
    if (params.copyLightToDirection)
    {
        json[Keys::CopyLightToDirection] = true;
    }
    return json;
}
} // namespace Data::Effects
