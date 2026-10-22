#include "stdafx.h"

#include "ItemModelMaterialJson.h"
#include "ItemModelValueReader.h"

#include <string>

namespace Data::Items::MaterialJson
{
namespace
{
using Json::OrderedJson;

constexpr const char* SpecularKey = "specular";
} // namespace

void Write(const ItemModelDefinition& model, OrderedJson& json)
{
    if (model.material == ItemMaterial{})
    {
        return;
    }

    OrderedJson material = OrderedJson::object();
    material[SpecularKey] = ModelJson::WriteNumber(model.material.specular);
    json[MaterialKey] = std::move(material);
}

void Read(const OrderedJson& json, ItemModelDefinition& model, const ModelJson::ReportIssue& report)
{
    const OrderedJson* object = ModelJson::FindObject(json, MaterialKey, report);
    if (object == nullptr)
    {
        return;
    }

    ModelJson::ItemModelValueReader reader(*object, MaterialKey, report);
    double specular = 0.0;
    if (reader.ReadNumber(SpecularKey, specular, false))
    {
        if (specular < 0.0 || specular > MaxSpecular)
        {
            reader.Error(SpecularKey, "must be from 0 to " + std::to_string(static_cast<int>(MaxSpecular)));
        }
        else
        {
            model.material.specular = specular;
        }
    }
    reader.WarnAboutUnknownKeys();
}
} // namespace Data::Items::MaterialJson
