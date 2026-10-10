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

bool AnyColumnCopies(const EffectCreateParams& row, const Data::Effects::EffectCopyField& field)
{
    return row.*field.copy ||
           std::any_of(row.variants.begin(), row.variants.end(),
                       [&](const Data::Effects::EffectCreateVariant& variant) { return variant.params.*field.copy; });
}

// One line per field the row or a variant sets, in the order of the data file.
// The order comes from writing one set of values with every value and offset
// of the row and the variants and every copy one of them makes; the copies
// are added last, so the copy of one variant does not clear a value another
// sets.
std::vector<EffectCreateTable::Line> LinesOf(const EffectCreateParams& row, size_t columnCount)
{
    EffectCreateParams all = row;
    all.variants.clear();
    for (const Data::Effects::EffectCreateVariant& variant : row.variants)
    {
        EffectCreateParams values = variant.params;
        for (const Data::Effects::EffectCopyField& field : Data::Effects::EffectCopyFields)
            values.*field.copy = false;
        all = Data::Effects::ResolveVariant(all, values);
    }
    for (const Data::Effects::EffectCopyField& field : Data::Effects::EffectCopyFields)
        all.*field.copy = all.*field.copy || AnyColumnCopies(row, field);

    std::vector<EffectCreateTable::Line> lines;
    for (const auto& field : FlattenFields(all))
        lines.push_back({field.first, std::vector<std::string>(columnCount)});
    return lines;
}

void FillColumn(EffectCreateTable& table, size_t column, const FieldValues& fields)
{
    for (const auto& [field, value] : fields)
    {
        const auto line =
            std::find_if(table.lines.begin(), table.lines.end(),
                         [&](const EffectCreateTable::Line& existing) { return existing.field == field; });
        if (line != table.lines.end())
            line->values[column] = value;
    }
}
} // namespace

EffectCreateTable BuildEffectCreateTable(const EffectCreateParams& row)
{
    EffectCreateTable table;
    table.lines = LinesOf(row, row.variants.size() + 1);
    EffectCreateParams common = row;
    common.variants.clear();
    FillColumn(table, 0, FlattenFields(common));
    for (size_t i = 0; i < row.variants.size(); ++i)
    {
        table.variantSubTypes.push_back(row.variants[i].subTypes);
        FillColumn(table, i + 1, FlattenFields(Data::Effects::ResolveVariant(row, row.variants[i].params)));
    }
    return table;
}
} // namespace MuEditor::Effects

#endif // _EDITOR
