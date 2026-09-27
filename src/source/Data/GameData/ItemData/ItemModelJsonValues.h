#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <functional>
#include <span>
#include <string>

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
} // namespace Data::Items::ModelJson
