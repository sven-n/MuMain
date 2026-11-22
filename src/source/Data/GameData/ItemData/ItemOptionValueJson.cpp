#include "stdafx.h"

#include "Data/GameData/ItemData/ItemOptionValueJson.h"

namespace Data::Items::Json
{
namespace
{
namespace ValueKey
{
constexpr const char* Values = "values";
constexpr const char* PerLevel = "perLevel";
constexpr const char* Per = "per";
constexpr const char* Attribute = "attribute";
constexpr const char* Times = "times";
constexpr const char* Divisor = "divisor";
} // namespace ValueKey

// perLevel has an error of its own, so it is no unknown field.
const KeySet ValueFields = {ValueKey::Values, ValueKey::PerLevel, ValueKey::Per};
const KeySet TermFields = {ValueKey::Attribute, ValueKey::Times, ValueKey::Divisor};

void ReadTerms(const OrderedJson& json, const std::string& field, std::vector<ItemOptionTerm>& terms,
               const ReportProblem& fatal, const ReportProblem& warning)
{
    if (!json.is_array() || json.empty())
    {
        fatal(field, R"(must be a list of terms: [{"attribute": "level", "divisor": 20}])");
        return;
    }
    for (size_t index = 0; index < json.size(); ++index)
    {
        const std::string termField = field + "[" + std::to_string(index) + "]";
        const OrderedJson& entry = json[index];
        if (!entry.is_object())
        {
            fatal(termField, R"(must be an object: {"attribute": "level", "divisor": 20})");
            continue;
        }
        ItemOptionTerm term;
        if (!ReadNameField(entry, ValueKey::Attribute, termField, term.attribute, fatal))
        {
            continue;
        }
        const auto times = entry.find(ValueKey::Times);
        const auto divisor = entry.find(ValueKey::Divisor);
        if ((times == entry.end()) == (divisor == entry.end()))
        {
            fatal(termField, "needs exactly one of times and divisor");
        }
        else if (times != entry.end())
        {
            ReadWholeNumberInRange(*times, termField + "." + ValueKey::Times, 1, MaxOptionValue, term.times, fatal);
        }
        else
        {
            ReadWholeNumberInRange(*divisor, termField + "." + ValueKey::Divisor, 1, MaxOptionValue, term.divisor,
                                   fatal);
        }
        WarnAboutUnknownKeys(entry, TermFields, termField + ".", warning);
        terms.push_back(std::move(term));
    }
}
} // namespace

void ReadOptionValue(const OrderedJson& json, const std::string& field, ItemOptionValue& value,
                     const ReportProblem& fatal, const ReportProblem& warning, int maximum)
{
    if (!json.is_object())
    {
        fatal(field, R"(must be an object: {"values": 5}, or {"values": [4, 8, 12, 16]} for one value per level)");
        return;
    }
    if (json.contains(ValueKey::PerLevel))
    {
        fatal(field + "." + ValueKey::PerLevel,
              R"(is not a value; write one value per level as a list: {"values": [4, 8, 12, 16]})");
    }
    const auto values = json.find(ValueKey::Values);
    if (values == json.end() || (!values->is_number() && (!values->is_array() || values->empty())))
    {
        fatal(field + "." + ValueKey::Values, "missing, or neither a number nor a list of one value per level");
        return;
    }
    value.isList = values->is_array();
    if (!value.isList)
    {
        int constant = 0;
        ReadWholeNumberInRange(*values, field + "." + ValueKey::Values, 0, maximum, constant, fatal);
        value.values = {constant};
    }
    else
    {
        for (size_t index = 0; index < values->size(); ++index)
        {
            int entry = 0;
            ReadWholeNumberInRange((*values)[index], field + "." + ValueKey::Values + "[" + std::to_string(index) + "]",
                                   0, maximum, entry, fatal);
            value.values.push_back(entry);
        }
    }
    if (const auto per = json.find(ValueKey::Per); per != json.end())
    {
        ReadTerms(*per, field + "." + ValueKey::Per, value.per, fatal, warning);
    }
    WarnAboutUnknownKeys(json, ValueFields, field + ".", warning);
}

OrderedJson WriteOptionValue(const ItemOptionValue& value)
{
    OrderedJson json;
    if (value.isList)
    {
        json[ValueKey::Values] = value.values;
    }
    else
    {
        json[ValueKey::Values] = value.values.empty() ? 0 : value.values.front();
    }
    if (!value.per.empty())
    {
        OrderedJson terms = OrderedJson::array();
        for (const ItemOptionTerm& term : value.per)
        {
            OrderedJson entry;
            entry[ValueKey::Attribute] = term.attribute;
            if (term.divisor > 0)
            {
                entry[ValueKey::Divisor] = term.divisor;
            }
            else
            {
                entry[ValueKey::Times] = term.times;
            }
            terms.push_back(std::move(entry));
        }
        json[ValueKey::Per] = std::move(terms);
    }
    return json;
}
} // namespace Data::Items::Json
