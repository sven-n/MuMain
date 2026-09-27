#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <functional>
#include <set>
#include <span>
#include <string>
#include <vector>

// Reading and writing the values of the objects in the item model files
// ("inventory", "ground", "glow").
namespace Data::Items::ModelJson
{
using ReportIssue = std::function<void(ItemDataIssueSeverity, const std::string& field, const std::string& message)>;

// 270 instead of 270.0, so hand-written and written files look the same.
Json::OrderedJson WriteNumber(double value);
Json::OrderedJson WriteNumbers(std::span<const double> values);

// The object `key` of the model, or nullptr when there is none (or it is not
// an object, which is reported).
const Json::OrderedJson* FindObject(const Json::OrderedJson& json, const char* key, const ReportIssue& report);

// Reads the values of one object of a model; the issues name the field as
// "object.key".
class ValueReader
{
public:
    ValueReader(const Json::OrderedJson& json, std::string objectKey, const ReportIssue& report);

    bool ReadNumber(const char* key, double& value, bool mustBePositive);
    // Reads a list of minCount to values.size() numbers.
    bool ReadNumbers(const char* key, std::span<double> values, size_t minCount);
    // Reads a whole number from 0 to maxValue.
    bool ReadIndex(const char* key, int& value, int maxValue);
    // Reads a list of at least one whole number from 0 to maxValue.
    bool ReadIndexes(const char* key, std::vector<int>& values, int maxValue);
    bool ReadBool(const char* key, bool& value);
    // Reads a name of letters and digits.
    bool ReadName(const char* key, std::string& value);

    bool IsNumber(const char* key) const;

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
