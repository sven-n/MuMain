#pragma once

#include "Core/Globals/_define.h"
#include "Data/GameData/ItemData/ItemDataIssue.h"

#include "json.hpp"

#include <string>
#include <string_view>
#include <vector>

// Helpers shared by the item data files (ItemJsonFormat) and the item model
// files (ItemModelJsonFormat): both have the same file header,
//   { "formatVersion": 1, "group": 0, ... }
// and report their problems as ItemDataIssue.
namespace Data::Items::Json
{
using OrderedJson = nlohmann::ordered_json;

constexpr int Indent = 2;

namespace Keys
{
constexpr const char* FormatVersion = "formatVersion";
constexpr const char* Group = "group";
constexpr const char* Number = "number";
} // namespace Keys

// Reads a whole number without narrowing it, so a huge value cannot wrap
// around to a valid one. False when it is not a whole number or does not
// fit in a long long.
bool ReadWholeNumber(const OrderedJson& json, long long& number);

void AddFileIssue(std::vector<ItemDataIssue>& issues, const std::string& source, int group, const std::string& field,
                  const std::string& message);

// Parses the text and checks that it is an object with a supported
// "formatVersion" (1 to maxFormatVersion).
bool ReadFileVersion(std::string_view text, const std::string& source, int maxFormatVersion, OrderedJson& root,
                     std::vector<ItemDataIssue>& issues);

// ReadFileVersion, and checks that the file has a valid "group".
bool ReadFileHeader(std::string_view text, const std::string& source, int maxFormatVersion, OrderedJson& root,
                    int& group, std::vector<ItemDataIssue>& issues);

// Reads "number" of a list entry: a whole number from 0 to MAX_ITEM_INDEX - 1.
// On errors an issue is added (with `addIssue`) and false is returned.
template <typename TAddIssue> bool ReadNumber(const OrderedJson& json, int& number, TAddIssue&& addIssue)
{
    const auto field = json.find(Keys::Number);
    long long value = 0;
    if (field == json.end() || !field->is_number_integer())
    {
        addIssue(Keys::Number, "missing or not a whole number");
        return false;
    }
    if (!ReadWholeNumber(*field, value) || value < 0 || value >= MAX_ITEM_INDEX)
    {
        addIssue(Keys::Number, "must be between 0 and " + std::to_string(MAX_ITEM_INDEX - 1));
        return false;
    }
    number = static_cast<int>(value);
    return true;
}

// Names in the data (e.g. glow colors) have only letters and digits.
bool IsName(std::string_view text);

// The JSON writer puts every list entry on its own line. Short lists of
// words or numbers are easier to read on one line: "tags": ["jewel", "valuable"].
// Puts every list of `key` on one line; text in quotes (e.g. a folder name
// with spaces) is kept as it is.
std::string PutListsOnOneLine(const std::string& text, std::string_view key);

// Puts every object of `key`, with the objects in it, on one line:
// "angle": {"y": 0}. A space stays after each comma and colon.
std::string PutObjectsOnOneLine(const std::string& text, std::string_view key);
} // namespace Data::Items::Json
