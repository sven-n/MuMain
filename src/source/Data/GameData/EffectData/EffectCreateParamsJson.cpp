#include "stdafx.h"

#include "EffectCreateParamsJson.h"

#include "Data/GameData/ItemData/ItemModelValueReader.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <set>

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
constexpr const char* AlphaTarget = "alphaTarget";
constexpr const char* RenderType = "renderType";
constexpr const char* Animation = "animation";
constexpr const char* Position = "position";
constexpr const char* Angle = "angle";
constexpr const char* Direction = "direction";
constexpr const char* StartPosition = "startPosition";
constexpr const char* HeadTargetAngle = "headTargetAngle";
constexpr const char* Offset = "offset";
constexpr const char* Copy = "copy";
constexpr const char* CopyLightToDirection = "copyLightToDirection";
constexpr const char* Variants = "variants";
constexpr const char* SubType = "subType";
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

// The names of the render types.
struct RenderTypeName
{
    const char* name;
    EffectRenderType type;
};
constexpr std::array<RenderTypeName, 2> RenderTypeNames = {{
    {"dark", EffectRenderType::Dark},
    {"alphaBlendMinus", EffectRenderType::AlphaBlendMinus},
}};

// hiddenMesh is a mesh number and blendMesh the texture number of meshes,
// both shorts in the model; -1 is none, a hiddenMesh of -2 hides the whole
// model and a blendMesh of -2 means every mesh.
constexpr int SmallestMeshNumber = -2;
constexpr int LargestMeshNumber = std::numeric_limits<short>::max();

// The animation is an int in the effect.
constexpr int LargestAnimation = std::numeric_limits<int>::max();

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
        reader.Fatal(key, TooLarge);
        return;
    }
    value = number;
}

void ReadAlpha(ItemModelValueReader& reader, const char* key, std::optional<double>& value)
{
    double alpha = 0.0;
    if (!reader.ReadNumber(key, alpha, false))
    {
        return;
    }
    if (alpha < 0.0 || alpha > 1.0)
    {
        reader.Fatal(key, "must be from 0 to 1");
        return;
    }
    value = alpha;
}

void ReadRenderType(ItemModelValueReader& reader, std::optional<EffectRenderType>& value)
{
    const OrderedJson* field = reader.ReadJson(Keys::RenderType);
    if (field == nullptr)
    {
        return;
    }
    for (const RenderTypeName& name : RenderTypeNames)
    {
        if (field->is_string() && field->get_ref<const std::string&>() == name.name)
        {
            value = name.type;
            return;
        }
    }
    reader.Fatal(Keys::RenderType, "must be \"dark\" or \"alphaBlendMinus\"");
}

bool ReadThreeValues(ItemModelValueReader& reader, const char* key, std::array<double, 3>& values)
{
    if (!reader.ReadNumbers(key, values, values.size()))
    {
        return false;
    }
    if (!std::all_of(values.begin(), values.end(), FitsFloat))
    {
        reader.Fatal(key, TooLarge);
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
        reader.Fatal(key, expected);
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

// A vector: a list of 3 numbers, or an object with the components it sets.
void ReadVector(ItemModelValueReader& reader, const std::string& objectKey, const char* key,
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
        ReadValue(components, Keys::Components[i], vector.components[i]);
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
    ReadValue(offsetReader, Keys::LifeTime, params.lifeTimeOffset);
    for (const OffsetField& field : OffsetFields)
    {
        ReadVector(offsetReader, offsets.objectKey, field.key, report, params.*field.offset);
    }
    offsetReader.WarnAboutUnknownKeys();
}

// The sources a copy into `target` can have: "a", "a" or "b", "a", "b" or "c".
std::string SourcesOf(std::string_view target)
{
    std::vector<std::string> sources;
    for (const EffectCopyField& field : EffectCopyFields)
    {
        if (field.target == target)
        {
            sources.push_back(std::string("\"") + field.source + "\"");
        }
    }
    std::string text;
    for (size_t i = 0; i < sources.size(); ++i)
    {
        text += (i == 0 ? "" : i + 1 == sources.size() ? " or " : ", ") + sources[i];
    }
    return text;
}

// Sets the copy into `target` from `source`; false when there is no such copy.
bool ReadCopySource(const OrderedJson& source, std::string_view target, EffectCreateParams& params)
{
    if (!source.is_string())
    {
        return false;
    }
    for (const EffectCopyField& field : EffectCopyFields)
    {
        if (field.target == target && source.get_ref<const std::string&>() == field.source)
        {
            params.*field.copy = true;
            return true;
        }
    }
    return false;
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
    for (size_t i = 0; i < EffectCopyFields.size(); ++i)
    {
        const std::string_view target = EffectCopyFields[i].target;
        if (i > 0 && EffectCopyFields[i - 1].target == target)
        {
            continue; // the copies of a target are listed together
        }
        const OrderedJson* source = copyReader.ReadJson(EffectCopyFields[i].target);
        if (source == nullptr)
        {
            continue;
        }
        if (!ReadCopySource(*source, target, params))
        {
            copyReader.Fatal(EffectCopyFields[i].target, "must be " + SourcesOf(target));
        }
    }
    copyReader.WarnAboutUnknownKeys();

    // A field gets one value or one copy.
    for (size_t i = 0; i < EffectCopyFields.size(); ++i)
    {
        const char* target = EffectCopyFields[i].target;
        if (SetsValueOf(params, target) && CopiesInto(params, target))
        {
            copyReader.Fatal(target, "is also set as a value");
            for (const EffectCopyField& field : EffectCopyFields)
            {
                if (field.target == std::string_view(target))
                {
                    params.*field.copy = false;
                }
            }
        }
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

// A list when all three components are set, else an object with the
// components that are set.
void WriteVector(OrderedJson& json, const char* key, const EffectCreateVector& vector)
{
    if (!vector.IsSet())
    {
        return;
    }
    const bool whole = std::all_of(vector.components.begin(), vector.components.end(),
                                   [](const std::optional<double>& component) { return component.has_value(); });
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
        components[Keys::Components[i]] = Items::ModelJson::WriteNumber(*vector.components[i]);
    }
    json[key] = std::move(components);
}
} // namespace

namespace
{
// The fields of a "create" object or of one of its variants.
void ReadFields(ItemModelValueReader& reader, const std::string& objectKey, const ReportIssue& report,
                EffectCreateParams& params)
{
    ReadValue(reader, Keys::LifeTime, params.lifeTime);
    ReadValue(reader, Keys::Scale, params.scale);
    ReadValue(reader, Keys::Velocity, params.velocity);
    ReadValue(reader, Keys::Gravity, params.gravity);
    ReadMeshNumber(reader, Keys::HiddenMesh, params.hiddenMesh);
    ReadMeshNumber(reader, Keys::BlendMesh, params.blendMesh);
    ReadValue(reader, Keys::BlendMeshLight, params.blendMeshLight);
    ReadAlpha(reader, Keys::Alpha, params.alpha);
    ReadLight(reader, params.light);
    ReadFlag(reader, Keys::LightEnable, params.lightEnable);
    ReadFlag(reader, Keys::AlphaEnable, params.alphaEnable);
    ReadInteger(reader, Keys::Kind, 0, LargestKind, params.kind);
    ReadInteger(reader, Keys::Skill, 0, LargestSkill, params.skill);
    ReadValue(reader, Keys::PkKey, params.pkKey);
    ReadValue(reader, Keys::Timer, params.timer);
    ReadValue(reader, Keys::Distance, params.distance);
    ReadValue(reader, Keys::CollisionRange, params.collisionRange);
    ReadAlpha(reader, Keys::AlphaTarget, params.alphaTarget);
    ReadRenderType(reader, params.renderType);
    ReadInteger(reader, Keys::Animation, 0, LargestAnimation, params.animation);
    ReadVector(reader, objectKey, Keys::Position, report, params.position);
    ReadVector(reader, objectKey, Keys::Angle, report, params.angle);
    ReadVector(reader, objectKey, Keys::Direction, report, params.direction);
    ReadVector(reader, objectKey, Keys::StartPosition, report, params.startPosition);
    ReadOffsets(reader, objectKey, report, params);
    ReadCopies(reader, objectKey, report, params);

    // Until FX1.4 the copy of the light was a field of its own; a file that
    // still has it would lose the copy with only a warning.
    if (reader.ReadJson(Keys::CopyLightToDirection) != nullptr)
    {
        reader.Fatal(Keys::CopyLightToDirection, "was replaced by \"copy\": {\"direction\": \"light\"}");
    }
}

// CreateEffect sets neither the start position nor the lifeTime, so an offset
// of a component or of the lifeTime that nothing sets or copies before adds to
// what the slot's previous effect left there.
void WarnAboutOffsetsOfOldValues(const EffectCreateParams& params, const std::string& objectKey,
                                 const ReportIssue& report)
{
    const EffectCreateVector& offset = params.startPositionOffset;
    const bool copied = CopiesInto(params, Keys::StartPosition);
    for (size_t i = 0; i < offset.components.size(); ++i)
    {
        if (offset.components[i] && !copied && !params.startPosition.components[i])
        {
            report(ItemDataIssueSeverity::Warning, objectKey + "." + Keys::Offset + "." + Keys::StartPosition,
                   "adds to the start position the slot's previous effect left; set or copy it first");
            break;
        }
    }
    if (params.lifeTimeOffset && !params.lifeTime)
    {
        report(ItemDataIssueSeverity::Warning, objectKey + "." + Keys::Offset + "." + Keys::LifeTime,
               "adds to the lifeTime the slot's previous effect left; set it first");
    }
}

// "subType": n, or "subTypes": [n, ...] for two or more.
bool ReadSubTypes(ItemModelValueReader& reader, std::vector<int>& subTypes)
{
    constexpr int LargestSubType = std::numeric_limits<int>::max();
    const bool hasOne = reader.Has(Keys::SubType);
    const bool hasList = reader.Has(CreateSubTypesKey);
    if (hasOne == hasList)
    {
        reader.Fatal(Keys::SubType, hasOne ? "and subTypes are both set; set one of them"
                                           : "missing; a variant needs subType or subTypes");
        reader.ReadJson(Keys::SubType);
        reader.ReadJson(CreateSubTypesKey);
        return false;
    }
    if (hasOne)
    {
        int subType = 0;
        if (!reader.ReadInteger(Keys::SubType, subType, 0, LargestSubType))
        {
            return false;
        }
        subTypes = {subType};
        return true;
    }
    std::vector<int> list;
    if (!reader.ReadIndexes(CreateSubTypesKey, list, LargestSubType))
    {
        return false;
    }
    std::sort(list.begin(), list.end());
    if (std::adjacent_find(list.begin(), list.end()) != list.end())
    {
        reader.Fatal(CreateSubTypesKey, "lists a SubType twice");
        return false;
    }
    subTypes = std::move(list);
    return true;
}

// A variant that sets part of a vector the row copies into would mean "copy,
// then change one component", which the order of the steps cannot express.
// The name of such a vector, or null.
const char* PartOfCopiedVector(const EffectCreateParams& row, const EffectCreateParams& variant)
{
    const auto setsPart = [](const EffectCreateVector& vector)
    { return vector.IsSet() && !(vector.components[0] && vector.components[1] && vector.components[2]); };
    if (CopiesInto(row, Keys::Direction) && setsPart(variant.direction))
    {
        return Keys::Direction;
    }
    if (CopiesInto(row, Keys::StartPosition) && setsPart(variant.startPosition))
    {
        return Keys::StartPosition;
    }
    return nullptr;
}

// Reads variant `entry` of `row`; false when it has fatal problems that drop it. Its
// SubTypes go into `taken` only when it is kept, so a dropped variant does not
// report the later ones as duplicates.
bool ReadVariant(const OrderedJson& entry, const std::string& variantKey, const ReportIssue& report,
                 const EffectCreateParams& row, std::set<int>& taken, EffectCreateVariant& variant)
{
    if (!entry.is_object())
    {
        report(ItemDataIssueSeverity::Fatal, variantKey, "must be an object with subType or subTypes and values");
        return false;
    }
    ItemModelValueReader reader(entry, variantKey, report);
    bool valid = ReadSubTypes(reader, variant.subTypes);
    ReadFields(reader, variantKey, report, variant.params);
    if (reader.ReadJson(Keys::Variants) != nullptr)
    {
        reader.Fatal(Keys::Variants, "a variant cannot have variants");
        valid = false;
    }
    reader.WarnAboutUnknownKeys();
    for (const int subType : variant.subTypes)
    {
        if (taken.contains(subType))
        {
            report(ItemDataIssueSeverity::Fatal, variantKey,
                   "SubType " + std::to_string(subType) + " is in another variant too");
            valid = false;
        }
    }
    if (const char* vector = PartOfCopiedVector(row, variant.params))
    {
        reader.Fatal(vector, std::string("sets part of ") + vector +
                                 ", which the row copies into; set all three "
                                 "components");
        valid = false;
    }
    if (variant.params == EffectCreateParams{})
    {
        report(ItemDataIssueSeverity::Warning, variantKey, SetsNoValue);
    }
    if (variant.params.startPositionOffset.IsSet() || variant.params.lifeTimeOffset)
    {
        WarnAboutOffsetsOfOldValues(ResolveVariant(row, variant.params), variantKey, report);
    }
    if (valid)
    {
        taken.insert(variant.subTypes.begin(), variant.subTypes.end());
    }
    return valid;
}

void ReadVariants(ItemModelValueReader& reader, const std::string& objectKey, const ReportIssue& report,
                  EffectCreateParams& params)
{
    const OrderedJson* list = reader.ReadJson(Keys::Variants);
    if (list == nullptr)
    {
        return;
    }
    const std::string listKey = objectKey + "." + Keys::Variants;
    if (!list->is_array())
    {
        reader.Fatal(Keys::Variants, "must be a list of variants");
        return;
    }
    if (list->empty())
    {
        report(ItemDataIssueSeverity::Warning, listKey, "has no variants");
    }
    std::set<int> taken;
    for (size_t i = 0; i < list->size(); ++i)
    {
        EffectCreateVariant variant;
        if (ReadVariant((*list)[i], listKey + "[" + std::to_string(i) + "]", report, params, taken, variant))
        {
            params.variants.push_back(std::move(variant));
        }
    }
}

// The variants, sorted by their SubTypes; one SubType is written as "subType".
OrderedJson WriteVariants(const std::vector<EffectCreateVariant>& variants)
{
    std::vector<const EffectCreateVariant*> sorted;
    for (const EffectCreateVariant& variant : variants)
    {
        sorted.push_back(&variant);
    }
    std::sort(sorted.begin(), sorted.end(), [](const EffectCreateVariant* left, const EffectCreateVariant* right)
              { return left->subTypes < right->subTypes; });
    OrderedJson json = OrderedJson::array();
    for (const EffectCreateVariant* variant : sorted)
    {
        OrderedJson entry = OrderedJson::object();
        if (variant->subTypes.size() == 1)
        {
            entry[Keys::SubType] = variant->subTypes.front();
        }
        else
        {
            entry[CreateSubTypesKey] = variant->subTypes;
        }
        const OrderedJson fields = WriteEffectCreateParams(variant->params);
        for (const auto& [key, value] : fields.items())
        {
            entry[key] = value;
        }
        json.push_back(std::move(entry));
    }
    return json;
}
} // namespace

EffectCreateParams ReadEffectCreateParams(const OrderedJson& json, const std::string& objectKey,
                                          const ReportIssue& report)
{
    ItemModelValueReader reader(json, objectKey, report);
    EffectCreateParams params;
    ReadFields(reader, objectKey, report, params);
    WarnAboutOffsetsOfOldValues(params, objectKey, report);
    ReadVariants(reader, objectKey, report, params);
    reader.WarnAboutUnknownKeys();
    return params;
}

void WriteOffsets(OrderedJson& json, const EffectCreateParams& params)
{
    OrderedJson offsets = OrderedJson::object();
    if (params.lifeTimeOffset)
    {
        offsets[Keys::LifeTime] = Items::ModelJson::WriteNumber(*params.lifeTimeOffset);
    }
    for (const OffsetField& field : OffsetFields)
    {
        WriteVector(offsets, field.key, params.*field.offset);
    }
    if (!offsets.empty())
    {
        json[Keys::Offset] = std::move(offsets);
    }
}

void WriteCopies(OrderedJson& json, const EffectCreateParams& params)
{
    OrderedJson copies = OrderedJson::object();
    for (const EffectCopyField& field : EffectCopyFields)
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
    WriteValue(json, Keys::AlphaTarget, params.alphaTarget);
    if (params.renderType)
    {
        for (const RenderTypeName& name : RenderTypeNames)
        {
            if (name.type == *params.renderType)
            {
                json[Keys::RenderType] = name.name;
            }
        }
    }
    WriteExact(json, Keys::Animation, params.animation);
    WriteVector(json, Keys::Position, params.position);
    WriteVector(json, Keys::Angle, params.angle);
    WriteVector(json, Keys::Direction, params.direction);
    WriteVector(json, Keys::StartPosition, params.startPosition);
    WriteOffsets(json, params);
    WriteCopies(json, params);

    if (!params.variants.empty())
    {
        json[Keys::Variants] = WriteVariants(params.variants);
    }
    return json;
}
} // namespace Data::Effects
