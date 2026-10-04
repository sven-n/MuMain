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
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;
using Items::ModelJson::ItemModelValueReader;
using Items::ModelJson::ReportIssue;

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
constexpr const char* LightEnable = "lightEnable";
constexpr const char* AlphaEnable = "alphaEnable";
constexpr const char* Kind = "kind";
constexpr const char* Skill = "skill";
constexpr const char* PkKey = "pkKey";
constexpr const char* Timer = "timer";
constexpr const char* Distance = "distance";
constexpr const char* CollisionRange = "collisionRange";
constexpr const char* Position = "position";
constexpr const char* Angle = "angle";
constexpr const char* Direction = "direction";
constexpr const char* StartPosition = "startPosition";
constexpr const char* HeadTargetAngle = "headTargetAngle";
constexpr const char* Offset = "offset";
constexpr const char* Copy = "copy";
constexpr const char* Value = "value";
constexpr const char* TimesFrameFactor = "timesFrameFactor";
constexpr std::array<const char*, 3> Components = {"x", "y", "z"};
} // namespace Keys

// The offsets and the copies, in the order they are written.
struct OffsetField
{
    const char* key;
    EffectCreateVector EffectCreateParams::* offset;
};
constexpr std::array<OffsetField, 3> OffsetFields = {{
    {Keys::Position, &EffectCreateParams::positionOffset},
    {Keys::Angle, &EffectCreateParams::angleOffset},
    {Keys::StartPosition, &EffectCreateParams::startPositionOffset},
}};

// "copy": { "<target>": "<source>" }; the sources starting with "call" are
// arguments of the CreateEffect call.
struct CopyField
{
    const char* target;
    const char* source;
    bool EffectCreateParams::* copy;
};
constexpr std::array<CopyField, 4> CopyFields = {{
    {Keys::Direction, CreateLightKey, &EffectCreateParams::copyLightToDirection},
    {Keys::StartPosition, Keys::Position, &EffectCreateParams::copyPositionToStartPosition},
    {Keys::HeadTargetAngle, "callLight", &EffectCreateParams::copyCallLightToHeadTargetAngle},
    {Keys::Scale, "callScale", &EffectCreateParams::copyCallScaleToScale},
}};

// hiddenMesh is a mesh number and blendMesh the texture number of meshes,
// both shorts in the model; -1 is none, a hiddenMesh of -2 hides the whole
// model and a blendMesh of -2 means every mesh.
constexpr int SmallestMeshNumber = -2;
constexpr int LargestMeshNumber = std::numeric_limits<short>::max();

// Kind is a byte and Skill a 16-bit number in the effect.
constexpr int LargestKind = std::numeric_limits<unsigned char>::max();
constexpr int LargestSkill = std::numeric_limits<unsigned short>::max();

// The game keeps the values as float.
constexpr double LargestValue = std::numeric_limits<float>::max();
constexpr const char* TooLarge = "is too large for the game";
constexpr const char* SetsNoValue = "sets no value";

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

bool ReadThreeValues(ItemModelValueReader& reader, const char* key, std::array<double, 3>& values)
{
    if (!reader.ReadNumbers(key, values, values.size()))
    {
        return false;
    }
    if (!std::all_of(values.begin(), values.end(), FitsFloat))
    {
        reader.Error(key, TooLarge);
        return false;
    }
    return true;
}

void ReadLight(ItemModelValueReader& reader, std::optional<std::array<double, 3>>& value)
{
    std::array<double, 3> light{};
    if (ReadThreeValues(reader, CreateLightKey, light))
    {
        value = light;
    }
}

void ReadInteger(ItemModelValueReader& reader, const char* key, int smallest, int largest, std::optional<int>& value)
{
    int number = 0;
    if (reader.ReadInteger(key, number, smallest, largest))
    {
        value = number;
    }
}

void ReadFlag(ItemModelValueReader& reader, const char* key, std::optional<bool>& value)
{
    bool flag = false;
    if (reader.ReadBool(key, flag))
    {
        value = flag;
    }
}

// The issues of a nested object name its fields "<objectKey>.<key>.<field>".
struct NestedObject
{
    const OrderedJson* json = nullptr;
    std::string objectKey;
};

// Reads `key` as an object; null when it is missing or no object.
NestedObject ReadObject(ItemModelValueReader& reader, const std::string& objectKey, const char* key,
                        const char* expected)
{
    const OrderedJson* field = reader.ReadJson(key);
    if (field == nullptr)
    {
        return {};
    }
    if (!field->is_object())
    {
        reader.Error(key, expected);
        return {};
    }
    return {field, objectKey + "." + key};
}

void WarnIfEmpty(const NestedObject& object, const ReportIssue& report)
{
    if (object.json->empty())
    {
        report(ItemDataIssueSeverity::Warning, object.objectKey, SetsNoValue);
    }
}

// A component: a number, or, in an offset, { "value": n, "timesFrameFactor": true }.
void ReadComponent(ItemModelValueReader& reader, const std::string& objectKey, size_t index, bool isOffset,
                   const ReportIssue& report, EffectCreateVector& vector)
{
    const char* key = Keys::Components[index];
    const OrderedJson* field = reader.ReadJson(key);
    if (field == nullptr)
    {
        return;
    }
    if (!field->is_object())
    {
        ReadValue(reader, key, vector.components[index]);
        return;
    }
    if (!isOffset)
    {
        reader.Error(key, "must be a number; only offsets can be multiplied by the frame factor");
        return;
    }
    ItemModelValueReader number(*field, objectKey + "." + key, report);
    if (!number.Has(Keys::Value))
    {
        number.Error(Keys::Value, "missing");
    }
    std::optional<double> value;
    ReadValue(number, Keys::Value, value);
    bool timesFrameFactor = false;
    const bool flagRead =
        !number.Has(Keys::TimesFrameFactor) || number.ReadBool(Keys::TimesFrameFactor, timesFrameFactor);
    number.WarnAboutUnknownKeys();
    if (value && flagRead)
    {
        vector.components[index] = value;
        vector.timesFrameFactor[index] = timesFrameFactor;
    }
}

// A vector: a list of 3 numbers, or an object with the components it sets.
// Only the components of offsets can be multiplied by the frame factor.
void ReadVector(ItemModelValueReader& reader, const std::string& objectKey, const char* key, bool isOffset,
                const ReportIssue& report, EffectCreateVector& vector)
{
    if (!reader.Has(key))
    {
        return;
    }
    if (reader.ReadJson(key)->is_array())
    {
        std::array<double, 3> values{};
        if (ReadThreeValues(reader, key, values))
        {
            std::copy(values.begin(), values.end(), vector.components.begin());
        }
        return;
    }
    const NestedObject object =
        ReadObject(reader, objectKey, key, "must be a list of 3 numbers or an object with x, y or z");
    if (object.json == nullptr)
    {
        return;
    }
    WarnIfEmpty(object, report);
    ItemModelValueReader components(*object.json, object.objectKey, report);
    for (size_t i = 0; i < Keys::Components.size(); ++i)
    {
        ReadComponent(components, object.objectKey, i, isOffset, report, vector);
    }
    components.WarnAboutUnknownKeys();
}

void ReadOffsets(ItemModelValueReader& reader, const std::string& objectKey, const ReportIssue& report,
                 EffectCreateParams& params)
{
    const NestedObject offsets =
        ReadObject(reader, objectKey, Keys::Offset, "must be an object with the offsets of fields");
    if (offsets.json == nullptr)
    {
        return;
    }
    WarnIfEmpty(offsets, report);
    ItemModelValueReader offsetReader(*offsets.json, offsets.objectKey, report);
    for (const OffsetField& field : OffsetFields)
    {
        ReadVector(offsetReader, offsets.objectKey, field.key, true, report, params.*field.offset);
    }
    offsetReader.WarnAboutUnknownKeys();
}

void ReadCopies(ItemModelValueReader& reader, const std::string& objectKey, const ReportIssue& report,
                EffectCreateParams& params)
{
    const NestedObject copies =
        ReadObject(reader, objectKey, Keys::Copy, "must be an object with the fields to copy into");
    if (copies.json == nullptr)
    {
        return;
    }
    WarnIfEmpty(copies, report);
    ItemModelValueReader copyReader(*copies.json, copies.objectKey, report);
    for (const CopyField& field : CopyFields)
    {
        const OrderedJson* source = copyReader.ReadJson(field.target);
        if (source == nullptr)
        {
            continue;
        }
        if (!source->is_string() || source->get_ref<const std::string&>() != field.source)
        {
            copyReader.Error(field.target, std::string("must be \"") + field.source + "\"");
            continue;
        }
        params.*field.copy = true;
    }
    copyReader.WarnAboutUnknownKeys();

    // A field gets one value or one copy.
    if (params.scale && params.copyCallScaleToScale)
    {
        copyReader.Error(Keys::Scale, "is also set as a value");
        params.copyCallScaleToScale = false;
    }
    if (params.direction.IsSet() && params.copyLightToDirection)
    {
        copyReader.Error(Keys::Direction, "is also set as a value");
        params.copyLightToDirection = false;
    }
}

void ReadMeshNumber(ItemModelValueReader& reader, const char* key, std::optional<int>& value)
{
    ReadInteger(reader, key, SmallestMeshNumber, LargestMeshNumber, value);
}

void WriteValue(OrderedJson& json, const char* key, const std::optional<double>& value)
{
    if (value)
    {
        json[key] = Items::ModelJson::WriteNumber(*value);
    }
}

template <typename T> void WriteExact(OrderedJson& json, const char* key, const std::optional<T>& value)
{
    if (value)
    {
        json[key] = *value;
    }
}

// A list when all three components are set and none is multiplied by the
// frame factor, else an object with the components that are set.
void WriteVector(OrderedJson& json, const char* key, const EffectCreateVector& vector)
{
    if (!vector.IsSet())
    {
        return;
    }
    const bool whole = std::all_of(vector.components.begin(), vector.components.end(),
                                   [](const std::optional<double>& component) { return component.has_value(); }) &&
                       std::none_of(vector.timesFrameFactor.begin(), vector.timesFrameFactor.end(),
                                    [](bool timesFrameFactor) { return timesFrameFactor; });
    if (whole)
    {
        const std::array<double, 3> values = {*vector.components[0], *vector.components[1], *vector.components[2]};
        json[key] = Items::ModelJson::WriteNumbers(values);
        return;
    }
    OrderedJson components = OrderedJson::object();
    for (size_t i = 0; i < Keys::Components.size(); ++i)
    {
        if (!vector.components[i])
        {
            continue;
        }
        OrderedJson value = Items::ModelJson::WriteNumber(*vector.components[i]);
        if (vector.timesFrameFactor[i])
        {
            OrderedJson number = OrderedJson::object();
            number[Keys::Value] = std::move(value);
            number[Keys::TimesFrameFactor] = true;
            value = std::move(number);
        }
        components[Keys::Components[i]] = std::move(value);
    }
    json[key] = std::move(components);
}
} // namespace

EffectCreateParams ReadEffectCreateParams(const OrderedJson& json, const std::string& objectKey,
                                          const ReportIssue& report)
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
    ReadFlag(reader, Keys::LightEnable, params.lightEnable);
    ReadFlag(reader, Keys::AlphaEnable, params.alphaEnable);
    ReadInteger(reader, Keys::Kind, 0, LargestKind, params.kind);
    ReadInteger(reader, Keys::Skill, 0, LargestSkill, params.skill);
    ReadValue(reader, Keys::PkKey, params.pkKey);
    ReadValue(reader, Keys::Timer, params.timer);
    ReadValue(reader, Keys::Distance, params.distance);
    ReadValue(reader, Keys::CollisionRange, params.collisionRange);
    ReadVector(reader, objectKey, Keys::Position, false, report, params.position);
    ReadVector(reader, objectKey, Keys::Angle, false, report, params.angle);
    ReadVector(reader, objectKey, Keys::Direction, false, report, params.direction);
    ReadOffsets(reader, objectKey, report, params);
    ReadCopies(reader, objectKey, report, params);
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
    WriteExact(json, Keys::HiddenMesh, params.hiddenMesh);
    WriteExact(json, Keys::BlendMesh, params.blendMesh);
    WriteValue(json, Keys::BlendMeshLight, params.blendMeshLight);
    WriteValue(json, Keys::Alpha, params.alpha);
    if (params.light)
    {
        json[CreateLightKey] = Items::ModelJson::WriteNumbers(*params.light);
    }
    WriteExact(json, Keys::LightEnable, params.lightEnable);
    WriteExact(json, Keys::AlphaEnable, params.alphaEnable);
    WriteExact(json, Keys::Kind, params.kind);
    WriteExact(json, Keys::Skill, params.skill);
    WriteValue(json, Keys::PkKey, params.pkKey);
    WriteValue(json, Keys::Timer, params.timer);
    WriteValue(json, Keys::Distance, params.distance);
    WriteValue(json, Keys::CollisionRange, params.collisionRange);
    WriteVector(json, Keys::Position, params.position);
    WriteVector(json, Keys::Angle, params.angle);
    WriteVector(json, Keys::Direction, params.direction);

    OrderedJson offsets = OrderedJson::object();
    for (const OffsetField& field : OffsetFields)
    {
        WriteVector(offsets, field.key, params.*field.offset);
    }
    if (!offsets.empty())
    {
        json[Keys::Offset] = std::move(offsets);
    }

    OrderedJson copies = OrderedJson::object();
    for (const CopyField& field : CopyFields)
    {
        if (params.*field.copy)
        {
            copies[field.target] = field.source;
        }
    }
    if (!copies.empty())
    {
        json[Keys::Copy] = std::move(copies);
    }
    return json;
}
} // namespace Data::Effects
