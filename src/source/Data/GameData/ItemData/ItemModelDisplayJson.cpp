#include "stdafx.h"

#include "ItemModelDisplayJson.h"
#include "ItemModelValueReader.h"

#include <span>

namespace Data::Items::DisplayJson
{
namespace
{
using Json::OrderedJson;
using ModelJson::WriteNumber;
using ModelJson::WriteNumbers;

constexpr const char* ScaleKey = "scale";
constexpr const char* BodyHeightKey = "bodyHeight";

constexpr size_t AnchorCount = 2;
constexpr size_t PlaneOffsetCount = 2;
constexpr size_t OffsetCount = 3;
constexpr size_t RotationCount = 3;

// ---------------------------------------------------------------- writing

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

void ReadInventory(const OrderedJson& json, ItemInventoryDisplay& display, const ReportIssue& report)
{
    const OrderedJson* object = ModelJson::FindObject(json, InventoryKey, report);
    if (object == nullptr)
    {
        return;
    }

    ModelJson::ItemModelValueReader reader(*object, InventoryKey, report);
    reader.ReadNumbers(AnchorKey, display.anchor, AnchorCount);
    reader.ReadNumbers(OffsetKey, display.offset, PlaneOffsetCount);
    reader.ReadNumbers(RotationKey, display.rotation, RotationCount);
    reader.ReadNumber(ScaleKey, display.scale, true);
    reader.ReadNumber(BodyHeightKey, display.bodyHeight, false);
    reader.WarnAboutUnknownKeys();
}

void ReadGround(const OrderedJson& json, ItemGroundDisplay& display, const ReportIssue& report)
{
    const OrderedJson* object = ModelJson::FindObject(json, GroundKey, report);
    if (object == nullptr)
    {
        return;
    }

    ModelJson::ItemModelValueReader reader(*object, GroundKey, report);
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
