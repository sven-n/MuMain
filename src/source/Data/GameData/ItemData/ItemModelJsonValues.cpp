#include "stdafx.h"

#include "ItemModelJsonValues.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <optional>

namespace Data::Items::ModelJson
{
namespace
{
using Json::OrderedJson;

// Whole numbers up to this size are written without a decimal point.
constexpr double LargestWholeNumber = 1e9;

std::optional<int> ToIndex(const OrderedJson& value, int maxValue)
{
    long long number = 0;
    if (!Json::ReadWholeNumber(value, number) || number < 0 || number > maxValue)
    {
        return std::nullopt;
    }
    return static_cast<int>(number);
}
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

ValueReader::ValueReader(const OrderedJson& json, std::string objectKey, const ReportIssue& report)
    : m_json(json), m_objectKey(std::move(objectKey)), m_report(report)
{
}

const OrderedJson* ValueReader::Find(const char* key)
{
    m_knownKeys.insert(key);
    const auto field = m_json.find(key);
    return field != m_json.end() ? &*field : nullptr;
}

bool ValueReader::Has(const char* key) const
{
    return m_json.contains(key);
}

bool ValueReader::ReadNumber(const char* key, double& value, bool mustBePositive)
{
    const OrderedJson* field = Find(key);
    if (field == nullptr)
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

bool ValueReader::ReadNumbers(const char* key, std::span<double> values, size_t minCount)
{
    const OrderedJson* field = Find(key);
    if (field == nullptr)
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

bool ValueReader::ReadIndex(const char* key, int& value, int maxValue)
{
    const OrderedJson* field = Find(key);
    if (field == nullptr)
    {
        return false;
    }
    const std::optional<int> index = ToIndex(*field, maxValue);
    if (!index)
    {
        Error(key, "must be a whole number from 0 to " + std::to_string(maxValue));
        return false;
    }
    value = *index;
    return true;
}

bool ValueReader::ReadIndexes(const char* key, std::vector<int>& values, int maxValue)
{
    const OrderedJson* field = Find(key);
    if (field == nullptr)
    {
        return false;
    }

    const std::string message = "must be a list of whole numbers from 0 to " + std::to_string(maxValue);
    if (!field->is_array() || field->empty())
    {
        Error(key, message);
        return false;
    }
    std::vector<int> indexes;
    for (const OrderedJson& entry : *field)
    {
        const std::optional<int> index = ToIndex(entry, maxValue);
        if (!index)
        {
            Error(key, message);
            return false;
        }
        indexes.push_back(*index);
    }
    values = std::move(indexes);
    return true;
}

bool ValueReader::ReadBool(const char* key, bool& value)
{
    const OrderedJson* field = Find(key);
    if (field == nullptr)
    {
        return false;
    }
    if (!field->is_boolean())
    {
        Error(key, "must be true or false");
        return false;
    }
    value = field->get<bool>();
    return true;
}

bool ValueReader::ReadName(const char* key, std::string& value)
{
    const OrderedJson* field = Find(key);
    if (field == nullptr)
    {
        return false;
    }
    if (!field->is_string() || field->get_ref<const std::string&>().empty() ||
        !std::all_of(field->get_ref<const std::string&>().begin(), field->get_ref<const std::string&>().end(),
                     [](unsigned char character) { return std::isalnum(character) != 0; }))
    {
        Error(key, "must be a name of letters and digits");
        return false;
    }
    value = field->get<std::string>();
    return true;
}

bool ValueReader::IsNumber(const char* key) const
{
    const auto field = m_json.find(key);
    return field != m_json.end() && field->is_number();
}

void ValueReader::Error(const char* key, const std::string& message)
{
    m_report(ItemDataIssueSeverity::Error, m_objectKey + "." + key, message);
}

void ValueReader::WarnAboutUnknownKeys()
{
    for (const auto& [key, value] : m_json.items())
    {
        if (!m_knownKeys.contains(key))
        {
            m_report(ItemDataIssueSeverity::Warning, m_objectKey + "." + key, "unknown field, ignored");
        }
    }
}
} // namespace Data::Items::ModelJson
