#include "stdafx.h"

#include "ItemJsonCommon.h"

#include <algorithm>
#include <cctype>
#include <limits>

namespace Data::Items::Json
{
namespace
{
bool TryParse(std::string_view text, const std::string& source, OrderedJson& root, std::vector<ItemDataIssue>& issues)
{
    try
    {
        root = OrderedJson::parse(text);
        return true;
    }
    catch (const OrderedJson::parse_error& error)
    {
        AddFileIssue(issues, source, ItemDataIssue::NoItem, "", std::string("invalid JSON: ") + error.what());
        return false;
    }
}

bool ReadFormatVersion(const OrderedJson& root, const std::string& source, int maxFormatVersion,
                       std::vector<ItemDataIssue>& issues)
{
    const auto version = root.find(Keys::FormatVersion);
    if (version == root.end() || !version->is_number_integer())
    {
        AddFileIssue(issues, source, ItemDataIssue::NoItem, Keys::FormatVersion, "missing or not a whole number");
        return false;
    }

    long long formatVersion = 0;
    if (!ReadWholeNumber(*version, formatVersion) || formatVersion < 1 || formatVersion > maxFormatVersion)
    {
        AddFileIssue(issues, source, ItemDataIssue::NoItem, Keys::FormatVersion,
                     "version " + version->dump() + " is not supported (this client reads up to " +
                         std::to_string(maxFormatVersion) + ")");
        return false;
    }
    return true;
}

bool ReadGroup(const OrderedJson& root, const std::string& source, int& group, std::vector<ItemDataIssue>& issues)
{
    const auto groupField = root.find(Keys::Group);
    if (groupField == root.end() || !groupField->is_number_integer())
    {
        AddFileIssue(issues, source, ItemDataIssue::NoItem, Keys::Group, "missing or not a whole number");
        return false;
    }

    long long groupNumber = 0;
    if (!ReadWholeNumber(*groupField, groupNumber) || groupNumber < 0 || groupNumber >= MAX_ITEM_TYPE)
    {
        AddFileIssue(issues, source, ItemDataIssue::NoItem, Keys::Group,
                     "must be between 0 and " + std::to_string(MAX_ITEM_TYPE - 1));
        return false;
    }
    group = static_cast<int>(groupNumber);
    return true;
}

// Appends the list entries that start at `position` and its closing ']' to
// `result`, without the newlines and indentation between the entries (the
// space after each comma stays). Text in quotes is kept as it is. Returns
// the position after the ']', or the end of the text when there is none.
size_t AppendListOnOneLine(const std::string& text, size_t position, std::string& result)
{
    bool inText = false;
    for (; position < text.size(); ++position)
    {
        const char character = text[position];
        if (inText)
        {
            result += character;
            if (character == '\\' && position + 1 < text.size())
            {
                result += text[++position];
            }
            else if (character == '"')
            {
                inText = false;
            }
            continue;
        }

        if (character == ']')
        {
            break;
        }
        inText = character == '"';
        const bool afterComma = !result.empty() && result.back() == ',';
        if (character != '\n' && (character != ' ' || afterComma))
        {
            result += character;
        }
    }
    if (position == text.size())
    {
        // No closing ']' (the JSON writer always closes its lists).
        return position;
    }
    result += ']';
    return position + 1;
}

// Appends the members of the object that starts at `position`, the objects in
// it and its closing '}' to `result`, without the newlines and indentation
// (the space after each comma and colon stays). Returns the position after
// the '}', or the end of the text when there is none.
size_t AppendObjectOnOneLine(const std::string& text, size_t position, std::string& result)
{
    bool inText = false;
    int depth = 0;
    for (; position < text.size(); ++position)
    {
        const char character = text[position];
        if (inText)
        {
            result += character;
            if (character == '\\' && position + 1 < text.size())
            {
                result += text[++position];
            }
            else if (character == '"')
            {
                inText = false;
            }
            continue;
        }

        if (character == '}' && depth == 0)
        {
            break;
        }
        depth += character == '{' ? 1 : character == '}' ? -1 : 0;
        inText = character == '"';
        const bool afterSeparator = !result.empty() && (result.back() == ',' || result.back() == ':');
        if (character != '\n' && (character != ' ' || afterSeparator))
        {
            result += character;
        }
    }
    if (position == text.size())
    {
        // No closing '}' (the JSON writer always closes its objects).
        return position;
    }
    result += '}';
    return position + 1;
}
} // namespace

bool ReadWholeNumber(const OrderedJson& json, long long& number)
{
    if (json.is_number_unsigned())
    {
        const auto value = json.get<unsigned long long>();
        if (value > static_cast<unsigned long long>(std::numeric_limits<long long>::max()))
        {
            return false;
        }
        number = static_cast<long long>(value);
        return true;
    }
    if (json.is_number_integer())
    {
        number = json.get<long long>();
        return true;
    }
    return false;
}

bool IsName(std::string_view text)
{
    return !text.empty() &&
           std::all_of(text.begin(), text.end(), [](unsigned char character) { return std::isalnum(character) != 0; });
}

void AddFileIssue(std::vector<ItemDataIssue>& issues, const std::string& source, int group, const std::string& field,
                  const std::string& message)
{
    issues.push_back({ItemDataIssueSeverity::Error, source, group, ItemDataIssue::NoItem, field, message});
}

bool ReadFileVersion(std::string_view text, const std::string& source, int maxFormatVersion, OrderedJson& root,
                     std::vector<ItemDataIssue>& issues)
{
    if (!TryParse(text, source, root, issues))
    {
        return false;
    }
    if (!root.is_object())
    {
        AddFileIssue(issues, source, ItemDataIssue::NoItem, "", "the file must contain a JSON object");
        return false;
    }
    return ReadFormatVersion(root, source, maxFormatVersion, issues);
}

bool ReadFileHeader(std::string_view text, const std::string& source, int maxFormatVersion, OrderedJson& root,
                    int& group, std::vector<ItemDataIssue>& issues)
{
    return ReadFileVersion(text, source, maxFormatVersion, root, issues) && ReadGroup(root, source, group, issues);
}

std::string PutListsOnOneLine(const std::string& text, std::string_view key)
{
    const std::string listStart = "\"" + std::string(key) + "\": [";
    std::string result;
    size_t position = 0;
    while (true)
    {
        const size_t start = text.find(listStart, position);
        if (start == std::string::npos)
        {
            result.append(text, position, std::string::npos);
            return result;
        }

        result.append(text, position, start + listStart.size() - position);
        position = AppendListOnOneLine(text, start + listStart.size(), result);
    }
}

std::string PutObjectsOnOneLine(const std::string& text, std::string_view key)
{
    const std::string objectStart = "\"" + std::string(key) + "\": {";
    std::string result;
    size_t position = 0;
    while (true)
    {
        const size_t start = text.find(objectStart, position);
        if (start == std::string::npos)
        {
            result.append(text, position, std::string::npos);
            return result;
        }

        result.append(text, position, start + objectStart.size() - position);
        position = AppendObjectOnOneLine(text, start + objectStart.size(), result);
    }
}
} // namespace Data::Items::Json
