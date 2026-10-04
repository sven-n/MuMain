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

// Appends the entries of the list or object that starts at `position` (after
// its `open` bracket), the lists and objects in it and its `close` bracket to
// `result`, without the newlines and indentation between the entries (the
// space after each comma and colon stays). Text in quotes is kept as it is.
// Returns the position after the closing bracket, or the end of the text when
// there is none.
size_t AppendOnOneLine(const std::string& text, size_t position, char open, char close, std::string& result)
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

        if (character == close && depth == 0)
        {
            break;
        }
        depth += character == open ? 1 : character == close ? -1 : 0;
        inText = character == '"';
        const bool afterSeparator = !result.empty() && (result.back() == ',' || result.back() == ':');
        if (character != '\n' && (character != ' ' || afterSeparator))
        {
            result += character;
        }
    }
    if (position == text.size())
    {
        // No closing bracket (the JSON writer always closes them).
        return position;
    }
    result += close;
    return position + 1;
}

// Puts every value of `key` that starts with `open` on one line.
std::string PutValuesOnOneLine(const std::string& text, std::string_view key, char open, char close)
{
    const std::string valueStart = "\"" + std::string(key) + "\": " + open;
    std::string result;
    size_t position = 0;
    while (true)
    {
        const size_t start = text.find(valueStart, position);
        if (start == std::string::npos)
        {
            result.append(text, position, std::string::npos);
            return result;
        }

        result.append(text, position, start + valueStart.size() - position);
        position = AppendOnOneLine(text, start + valueStart.size(), open, close, result);
    }
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
    return PutValuesOnOneLine(text, key, '[', ']');
}

std::string PutObjectsOnOneLine(const std::string& text, std::string_view key)
{
    return PutValuesOnOneLine(text, key, '{', '}');
}
} // namespace Data::Items::Json
