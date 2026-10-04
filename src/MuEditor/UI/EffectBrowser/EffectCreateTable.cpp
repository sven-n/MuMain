#include "stdafx.h"

#ifdef _EDITOR

#include "EffectCreateTable.h"

#include "Data/GameData/EffectData/EffectCreateParamsJson.h"

#include <algorithm>
#include <array>
#include <string_view>
#include <utility>

namespace MuEditor::Effects
{
namespace
{
using Data::Effects::EffectCreateParams;
using Data::Items::Json::OrderedJson;

// The objects of the written "create" whose fields get a line each
// (EffectCreateParamsJson.cpp).
constexpr std::array<std::string_view, 2> ObjectsOfFields = {"offset", "copy"};

using FieldValues = std::vector<std::pair<std::string, std::string>>;

// A space after each ':' and ',' outside strings, as the data file writes the
// lists and objects it puts on one line ({"value": -60, "timesFrameFactor":
// true}); it also lets a narrow column wrap the text.
std::string WithSpaces(const std::string& compact)
{
    std::string text;
    bool inString = false;
    bool escaped = false;
    for (const char c : compact)
    {
        text += c;
        if (inString)
        {
            inString = escaped || c != '"';
            escaped = !escaped && c == '\\';
        }
        else if (c == '"')
            inString = true;
        else if (c == ':' || c == ',')
            text += ' ';
    }
    return text;
}

std::string DescribeValue(const OrderedJson& value)
{
    return value.is_string() ? value.get<std::string>() : WithSpaces(value.dump());
}

bool IsObjectOfFields(const std::string& key)
{
    return std::find(ObjectsOfFields.begin(), ObjectsOfFields.end(), key) != ObjectsOfFields.end();
}

// The fields `params` sets, in the order of the data file. `params` has no
// variants.
FieldValues FlattenFields(const EffectCreateParams& params)
{
    FieldValues fields;
    const OrderedJson written = Data::Effects::WriteEffectCreateParams(params);
    for (const auto& [key, value] : written.items())
    {
        if (!IsObjectOfFields(key))
        {
            fields.emplace_back(key, DescribeValue(value));
            continue;
        }
        for (const auto& [field, fieldValue] : value.items())
        {
            fields.emplace_back(key + "." + field, DescribeValue(fieldValue));
        }
    }
    return fields;
}

// Adds the fields of one column. A field no column before had goes after the
// field before it in this column, so the lines keep the order of the file.
void AddColumn(EffectCreateTable& table, size_t column, size_t columnCount, const FieldValues& fields)
{
    size_t insertAt = 0;
    for (const auto& [field, value] : fields)
    {
        auto line = std::find_if(table.lines.begin(), table.lines.end(),
                                 [&](const EffectCreateTable::Line& existing) { return existing.field == field; });
        if (line == table.lines.end())
        {
            line = table.lines.insert(table.lines.begin() + static_cast<std::ptrdiff_t>(insertAt),
                                      EffectCreateTable::Line{field, std::vector<std::string>(columnCount)});
        }
        line->values[column] = value;
        insertAt = static_cast<size_t>(line - table.lines.begin()) + 1;
    }
}
} // namespace

EffectCreateTable BuildEffectCreateTable(const EffectCreateParams& row)
{
    EffectCreateTable table;
    const size_t columnCount = row.variants.size() + 1;
    EffectCreateParams common = row;
    common.variants.clear();
    AddColumn(table, 0, columnCount, FlattenFields(common));
    for (size_t i = 0; i < row.variants.size(); ++i)
    {
        table.variantSubTypes.push_back(row.variants[i].subTypes);
        AddColumn(table, i + 1, columnCount, FlattenFields(Data::Effects::ResolveVariant(row, row.variants[i].params)));
    }
    return table;
}
} // namespace MuEditor::Effects

#endif // _EDITOR
