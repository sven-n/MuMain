#include "stdafx.h"

#include "ItemModelDisplayJson.h"

#include <cmath>
#include <set>
#include <span>

namespace Data::Items::DisplayJson
{
namespace
{
using Json::OrderedJson;

constexpr const char* ScaleKey = "scale";
constexpr const char* BodyHeightKey = "bodyHeight";

constexpr size_t AnchorCount = 2;
constexpr size_t PlaneOffsetCount = 2;
constexpr size_t OffsetCount = 3;
constexpr size_t RotationCount = 3;
// Whole numbers up to this size are written without a decimal point.
constexpr double LargestWholeNumber = 1e9;

// ---------------------------------------------------------------- writing

// 270 instead of 270.0, so hand-written and written files look the same.
OrderedJson WriteNumber(double value)
{
    if (value == std::floor(value) && std::abs(value) < LargestWholeNumber)
    {
        return static_cast<long long>(value);
    }
    return value;
}

OrderedJson WriteNumbers(std::span<const double> values)
{
    OrderedJson json = OrderedJson::array();
    for (const double value : values)
    {
        json.push_back(WriteNumber(value));
    }
    return json;
}

OrderedJson WriteInventory(const ItemInventoryDisplay& display)
{
    OrderedJson json = OrderedJson::object();
    if (display.anchor != ItemInventoryDisplay::DefaultAnchor)
    {
        json[AnchorKey] = WriteNumbers(display.anchor);
    }
    if (display.offset != std::array<double, OffsetCount>{})
    {
        // [x, y], or [x, y, z] when the model is also moved in depth.
        const size_t count = display.offset[2] == 0.0 ? PlaneOffsetCount : OffsetCount;
        json[OffsetKey] = WriteNumbers(std::span<const double>(display.offset).first(count));
    }
    if (display.rotation != ItemInventoryDisplay::DefaultRotation)
    {
        json[RotationKey] = WriteNumbers(display.rotation);
    }
    if (display.scale != ItemInventoryDisplay::DefaultScale)
    {
        json[ScaleKey] = WriteNumber(display.scale);
    }
    if (display.bodyHeight != 0.0)
    {
        json[BodyHeightKey] = WriteNumber(display.bodyHeight);
    }
    return json;
}

OrderedJson WriteGround(const ItemGroundDisplay& display)
{
    OrderedJson json = OrderedJson::object();
    if (display.rotation != ItemGroundDisplay::DefaultRotation)
    {
        json[RotationKey] = WriteNumbers(display.rotation);
    }
    if (display.scale)
    {
        json[ScaleKey] = WriteNumber(*display.scale);
    }
    if (display.bodyHeight != 0.0)
    {
        json[BodyHeightKey] = WriteNumber(display.bodyHeight);
    }
    return json;
}

// ---------------------------------------------------------------- reading

// Reads the values of one display object ("inventory" or "ground").
class DisplayReader
{
public:
    DisplayReader(const OrderedJson& json, std::string objectKey, const ReportIssue& report)
        : m_json(json), m_objectKey(std::move(objectKey)), m_report(report)
    {
    }

    bool ReadNumber(const char* key, double& value, bool mustBePositive)
    {
        m_knownKeys.insert(key);
        const auto field = m_json.find(key);
        if (field == m_json.end())
        {
            return false;
        }
        if (!field->is_number() || !std::isfinite(field->get<double>()))
        {
            Error(key, "must be a number");
            return false;
        }
        if (mustBePositive && field->get<double>() <= 0.0)
        {
            Error(key, "must be greater than 0");
            return false;
        }
        value = field->get<double>();
        return true;
    }

    // Reads a list of minCount to values.size() numbers.
    bool ReadNumbers(const char* key, std::span<double> values, size_t minCount)
    {
        m_knownKeys.insert(key);
        const auto field = m_json.find(key);
        if (field == m_json.end())
        {
            return false;
        }

        const std::string sizes = minCount == values.size()
                                      ? std::to_string(values.size())
                                      : std::to_string(minCount) + " or " + std::to_string(values.size());
        if (!field->is_array() || field->size() < minCount || field->size() > values.size())
        {
            Error(key, "must be a list of " + sizes + " numbers");
            return false;
        }
        for (const OrderedJson& entry : *field)
        {
            if (!entry.is_number() || !std::isfinite(entry.get<double>()))
            {
                Error(key, "must be a list of " + sizes + " numbers");
                return false;
            }
        }
        for (size_t i = 0; i < field->size(); ++i)
        {
            values[i] = (*field)[i].get<double>();
        }
        return true;
    }

    void WarnAboutUnknownKeys()
    {
        for (const auto& [key, value] : m_json.items())
        {
            if (!m_knownKeys.contains(key))
            {
                m_report(ItemDataIssueSeverity::Warning, m_objectKey + "." + key, "unknown field, ignored");
            }
        }
    }

private:
    void Error(const char* key, const std::string& message)
    {
        m_report(ItemDataIssueSeverity::Error, m_objectKey + "." + key, message);
    }

    const OrderedJson& m_json;
    std::string m_objectKey;
    const ReportIssue& m_report;
    std::set<std::string, std::less<>> m_knownKeys;
};

// The display object `key` of the model, or nullptr when there is none (or
// it is not an object, which is reported).
const OrderedJson* FindDisplayObject(const OrderedJson& json, const char* key, const ReportIssue& report)
{
    const auto field = json.find(key);
    if (field == json.end())
    {
        return nullptr;
    }
    if (!field->is_object())
    {
        report(ItemDataIssueSeverity::Error, key, "must be an object");
        return nullptr;
    }
    return &*field;
}

void ReadInventory(const OrderedJson& json, ItemInventoryDisplay& display, const ReportIssue& report)
{
    const OrderedJson* object = FindDisplayObject(json, InventoryKey, report);
    if (object == nullptr)
    {
        return;
    }

    DisplayReader reader(*object, InventoryKey, report);
    reader.ReadNumbers(AnchorKey, display.anchor, AnchorCount);
    reader.ReadNumbers(OffsetKey, display.offset, PlaneOffsetCount);
    reader.ReadNumbers(RotationKey, display.rotation, RotationCount);
    reader.ReadNumber(ScaleKey, display.scale, true);
    reader.ReadNumber(BodyHeightKey, display.bodyHeight, false);
    reader.WarnAboutUnknownKeys();
}

void ReadGround(const OrderedJson& json, ItemGroundDisplay& display, const ReportIssue& report)
{
    const OrderedJson* object = FindDisplayObject(json, GroundKey, report);
    if (object == nullptr)
    {
        return;
    }

    DisplayReader reader(*object, GroundKey, report);
    reader.ReadNumbers(RotationKey, display.rotation, RotationCount);
    double scale = 0.0;
    if (reader.ReadNumber(ScaleKey, scale, true))
    {
        display.scale = scale;
    }
    reader.ReadNumber(BodyHeightKey, display.bodyHeight, false);
    reader.WarnAboutUnknownKeys();
}

void ReadCloth(const OrderedJson& json, bool& cloth, const ReportIssue& report)
{
    const auto field = json.find(ClothKey);
    if (field == json.end())
    {
        return;
    }
    if (!field->is_boolean())
    {
        report(ItemDataIssueSeverity::Error, ClothKey, "must be true or false");
        return;
    }
    cloth = field->get<bool>();
}
} // namespace

void Write(const ItemModelDefinition& model, OrderedJson& json)
{
    OrderedJson inventory = WriteInventory(model.inventory);
    if (!inventory.empty())
    {
        json[InventoryKey] = std::move(inventory);
    }
    OrderedJson ground = WriteGround(model.ground);
    if (!ground.empty())
    {
        json[GroundKey] = std::move(ground);
    }
    if (model.cloth)
    {
        json[ClothKey] = true;
    }
}

void Read(const OrderedJson& json, ItemModelDefinition& model, const ReportIssue& report)
{
    ReadInventory(json, model.inventory, report);
    ReadGround(json, model.ground, report);
    ReadCloth(json, model.cloth, report);
}
} // namespace Data::Items::DisplayJson
