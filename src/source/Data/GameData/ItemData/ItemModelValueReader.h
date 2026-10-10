#pragma once

#include "Data/GameData/ItemData/ItemModelJsonValues.h"

#include <set>
#include <span>
#include <string>
#include <vector>

namespace Data::Items::ModelJson
{
// Reads the values of one object of a model; the issues name the field as
// "object.key".
class ItemModelValueReader
{
public:
    ItemModelValueReader(const Json::OrderedJson& json, std::string objectKey, const ReportIssue& report);

    bool ReadNumber(const char* key, double& value, bool mustBePositive);
    // Reads a list of minCount to values.size() numbers.
    bool ReadNumbers(const char* key, std::span<double> values, size_t minCount);
    // Reads a whole number from 0 to maxValue.
    bool ReadIndex(const char* key, int& value, int maxValue);
    // Reads a whole number from minValue to maxValue.
    bool ReadInteger(const char* key, int& value, int minValue, int maxValue);
    // Reads a list of at least one whole number from 0 to maxValue.
    bool ReadIndexes(const char* key, std::vector<int>& values, int maxValue);
    bool ReadBool(const char* key, bool& value);
    // Reads a name of letters and digits.
    bool ReadName(const char* key, std::string& value);

    bool IsNumber(const char* key) const;
    // Marks the key as read and returns its value, or null when the object
    // does not have it; for values the Read functions above do not cover.
    const Json::OrderedJson* ReadJson(const char* key);

    bool Has(const char* key) const;
    void Error(const char* key, const std::string& message);
    void WarnAboutUnknownKeys();

private:
    const Json::OrderedJson* Find(const char* key);

    const Json::OrderedJson& m_json;
    std::string m_objectKey;
    const ReportIssue& m_report;
    std::set<std::string, std::less<>> m_knownKeys;
};
} // namespace Data::Items::ModelJson
