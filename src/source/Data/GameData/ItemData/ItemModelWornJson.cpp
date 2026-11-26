#include "stdafx.h"

#include "ItemModelWornJson.h"
#include "ItemModelValueReader.h"

namespace Data::Items::WornJson
{
namespace
{
using Json::OrderedJson;

constexpr const char* BoneKey = "bone";
constexpr const char* LinkKey = "link";
constexpr const char* OffsetKey = "offset";
} // namespace

void Write(const ItemModelDefinition& model, OrderedJson& json)
{
    const ItemWorn defaults;
    if (model.worn == defaults)
    {
        return;
    }

    OrderedJson worn = OrderedJson::object();
    if (model.worn.bone != defaults.bone)
    {
        worn[BoneKey] = model.worn.bone;
    }
    if (model.worn.link != defaults.link)
    {
        worn[LinkKey] = model.worn.link;
    }
    if (model.worn.offset != defaults.offset)
    {
        worn[OffsetKey] = ModelJson::WriteNumbers(model.worn.offset);
    }
    json[WornKey] = std::move(worn);
}

void Read(const OrderedJson& json, ItemModelDefinition& model, const ModelJson::ReportIssue& report)
{
    const OrderedJson* object = ModelJson::FindObject(json, WornKey, report);
    if (object == nullptr)
    {
        return;
    }

    ModelJson::ItemModelValueReader reader(*object, WornKey, report);
    reader.ReadIndex(BoneKey, model.worn.bone, MaxBone);
    reader.ReadBool(LinkKey, model.worn.link);
    reader.ReadNumbers(OffsetKey, model.worn.offset, model.worn.offset.size());
    reader.WarnAboutUnknownKeys();
}
} // namespace Data::Items::WornJson
