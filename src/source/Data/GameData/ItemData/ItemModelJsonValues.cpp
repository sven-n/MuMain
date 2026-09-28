#include "stdafx.h"

#include "ItemModelJsonValues.h"

#include <cmath>

namespace Data::Items::ModelJson
{
namespace
{
using Json::OrderedJson;

// Whole numbers up to this size are written without a decimal point.
constexpr double LargestWholeNumber = 1e9;
} // namespace

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

const OrderedJson* FindObject(const OrderedJson& json, const char* key, const ReportIssue& report)
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
} // namespace Data::Items::ModelJson
