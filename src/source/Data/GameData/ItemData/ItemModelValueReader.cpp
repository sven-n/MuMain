#include "stdafx.h"

#include "ItemModelValueReader.h"

#include <cmath>
#include <optional>

namespace Data::Items::ModelJson
{
namespace
{
using Json::OrderedJson;

std::optional<int> ToInteger(const OrderedJson& value, int minValue, int maxValue)
{
    long long number = 0;
    if (!Json::ReadWholeNumber(value, number) || number < minValue || number > maxValue)
    {
        return std::nullopt;
    }
    return static_cast<int>(number);
}

std::optional<int> ToIndex(const OrderedJson& value, int maxValue)
{
    return ToInteger(value, 0, maxValue);
}
} // namespace

ItemModelValueReader::ItemModelValueReader(const OrderedJson& json, std::string objectKey, const ReportIssue& report)
    : m_json(json), m_objectKey(std::move(objectKey)), m_report(report)
{
}

const OrderedJson* ItemModelValueReader::Find(const char* key)
{
    m_knownKeys.insert(key);
    const auto field = m_json.find(key);
    return field != m_json.end() ? &*field : nullptr;
}

const OrderedJson* ItemModelValueReader::ReadJson(const char* key)
{
    return Find(key);
}

bool ItemModelValueReader::Has(const char* key) const
{
    return m_json.contains(key);
}

bool ItemModelValueReader::ReadNumber(const char* key, double& value, bool mustBePositive)
{
    const OrderedJson* field = Find(key);
    if (field == nullptr)
    {
        return false;
    }
    if (!field->is_number() || !std::isfinite(field->get<double>()))
    {
        Fatal(key, "must be a number");
        return false;
    }
    if (mustBePositive && field->get<double>() <= 0.0)
    {
        Fatal(key, "must be greater than 0");
        return false;
    }
    value = field->get<double>();
    return true;
}

bool ItemModelValueReader::ReadNumbers(const char* key, std::span<double> values, size_t minCount)
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
        Fatal(key, "must be a list of " + sizes + " numbers");
        return false;
    }
    for (const OrderedJson& entry : *field)
    {
        if (!entry.is_number() || !std::isfinite(entry.get<double>()))
        {
            Fatal(key, "must be a list of " + sizes + " numbers");
            return false;
        }
    }
    for (size_t i = 0; i < field->size(); ++i)
    {
        values[i] = (*field)[i].get<double>();
    }
    return true;
}

bool ItemModelValueReader::ReadIndex(const char* key, int& value, int maxValue)
{
    return ReadInteger(key, value, 0, maxValue);
}

bool ItemModelValueReader::ReadInteger(const char* key, int& value, int minValue, int maxValue)
{
    const OrderedJson* field = Find(key);
    if (field == nullptr)
    {
        return false;
    }
    const std::optional<int> number = ToInteger(*field, minValue, maxValue);
    if (!number)
    {
        Fatal(key, "must be a whole number from " + std::to_string(minValue) + " to " + std::to_string(maxValue));
        return false;
    }
    value = *number;
    return true;
}

bool ItemModelValueReader::ReadIndexes(const char* key, std::vector<int>& values, int maxValue)
{
    const OrderedJson* field = Find(key);
    if (field == nullptr)
    {
        return false;
    }

    const std::string message = "must be a list of whole numbers from 0 to " + std::to_string(maxValue);
    if (!field->is_array() || field->empty())
    {
        Fatal(key, message);
        return false;
    }
    std::vector<int> indexes;
    for (const OrderedJson& entry : *field)
    {
        const std::optional<int> index = ToIndex(entry, maxValue);
        if (!index)
        {
            Fatal(key, message);
            return false;
        }
        indexes.push_back(*index);
    }
    values = std::move(indexes);
    return true;
}

bool ItemModelValueReader::ReadPerLevelIndexes(const char* key, std::array<int, ItemLevelCount>& values, int maxValue)
{
    const OrderedJson* field = Find(key);
    if (field == nullptr)
    {
        return false;
    }
    bool valid = true;
    const auto readIndex = [&](const OrderedJson& entry, const std::string& entryField, int& value)
    {
        const std::optional<int> index = ToIndex(entry, maxValue);
        if (!index)
        {
            Fatal(entryField.c_str(), "must be a whole number from 0 to " + std::to_string(maxValue));
            valid = false;
            return;
        }
        value = *index;
    };
    const auto addError = [this](const std::string& errorField, const std::string& message)
    { Fatal(errorField.c_str(), message); };
    return Json::ReadPerLevelValues(*field, key, values, readIndex, addError) && valid;
}

bool ItemModelValueReader::ReadBool(const char* key, bool& value)
{
    const OrderedJson* field = Find(key);
    if (field == nullptr)
    {
        return false;
    }
    if (!field->is_boolean())
    {
        Fatal(key, "must be true or false");
        return false;
    }
    value = field->get<bool>();
    return true;
}

bool ItemModelValueReader::ReadName(const char* key, std::string& value)
{
    const OrderedJson* field = Find(key);
    if (field == nullptr)
    {
        return false;
    }
    if (!field->is_string() || !Json::IsName(field->get_ref<const std::string&>()))
    {
        Fatal(key, "must be a name of letters and digits");
        return false;
    }
    value = field->get<std::string>();
    return true;
}

void ItemModelValueReader::Fatal(const char* key, const std::string& message)
{
    m_report(ItemDataIssueSeverity::Fatal, m_objectKey + "." + key, message);
}

void ItemModelValueReader::WarnAboutUnknownKeys()
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
