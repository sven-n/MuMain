#include "stdafx.h"

#include "EffectTypesJson.h"

#include "Data/GameData/EffectData/EffectCreateParamsJson.h"
#include "Data/GameData/EffectData/EffectTypeSymbols.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <map>
#include <set>
#include <tuple>

namespace Data::Effects
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

namespace Keys
{
constexpr const char* Kind = "kind";
constexpr const char* Types = "types";
constexpr const char* Name = "name";
constexpr const char* Code = "code";
} // namespace Keys

constexpr const char* UnknownField = "unknown field, ignored";

void AddIssue(std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& source,
              const std::string& field, const std::string& message)
{
    issues.push_back({severity, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
}

void AddError(std::vector<ItemDataIssue>& issues, const std::string& source, const std::string& field,
              const std::string& message)
{
    AddIssue(issues, ItemDataIssueSeverity::Error, source, field, message);
}

bool IsTypeName(std::string_view name)
{
    return Items::Json::IsName(name) && std::isalpha(static_cast<unsigned char>(name.front())) != 0;
}

bool ReadText(const OrderedJson& json, const char* key, std::string& text)
{
    const auto field = json.find(key);
    if (field == json.end() || !field->is_string())
    {
        return false;
    }
    text = field->get<std::string>();
    return true;
}

// Reads the creation values of an effect entry, if it has them.
void ReadCreateParams(const OrderedJson& json, const std::string& field, const std::string& source,
                      EffectTypeEntry& entry, std::vector<ItemDataIssue>& issues)
{
    const auto create = json.find(CreateKey);
    if (create == json.end())
    {
        return;
    }
    const std::string createField = field + "." + CreateKey;
    if (!create->is_object())
    {
        AddError(issues, source, createField, "must be an object with the creation values");
        return;
    }
    const Items::ModelJson::ReportIssue report =
        [&](ItemDataIssueSeverity severity, const std::string& issueField, const std::string& message)
    { AddIssue(issues, severity, source, issueField, message); };
    entry.create = ReadEffectCreateParams(*create, createField, report);
    if (*entry.create == EffectCreateParams{})
    {
        AddIssue(issues, ItemDataIssueSeverity::Warning, source, createField,
                 "sets no value; the creation code of the type is skipped all the same");
    }
}

// Reads entry `index` of "types"; false when it has no name or code. Only
// effects have creation values.
bool ReadEntry(const OrderedJson& json, size_t index, const std::string& source, EffectKind kind,
               EffectTypeEntry& entry, std::vector<ItemDataIssue>& issues)
{
    const std::string field = std::string(Keys::Types) + "[" + std::to_string(index) + "]";
    if (!json.is_object())
    {
        AddError(issues, source, field, "must be an object with a name and a code");
        return false;
    }
    const auto readText = [&](const char* key, std::string& text)
    {
        if (ReadText(json, key, text))
        {
            return true;
        }
        AddError(issues, source, field + "." + key, "missing or not text");
        return false;
    };
    const bool hasName = readText(Keys::Name, entry.name);
    const bool hasCode = readText(Keys::Code, entry.code);
    const bool kindHasCreateParams = kind == EffectKind::Effect;
    if (kindHasCreateParams)
    {
        ReadCreateParams(json, field, source, entry, issues);
    }
    for (const auto& [key, value] : json.items())
    {
        if (key != Keys::Name && key != Keys::Code && !(kindHasCreateParams && key == CreateKey))
        {
            AddIssue(issues, ItemDataIssueSeverity::Warning, source, field + "." + key, UnknownField);
        }
    }
    return hasName && hasCode;
}

bool HasKind(const OrderedJson& root, EffectKind kind)
{
    const auto field = root.find(Keys::Kind);
    return field != root.end() && field->is_string() && field->get<std::string>() == GetEffectKindName(kind);
}

void WarnUnknownRootFields(const OrderedJson& root, const std::string& source, std::vector<ItemDataIssue>& issues)
{
    for (const auto& [key, value] : root.items())
    {
        if (key != Items::Json::Keys::FormatVersion && key != Keys::Kind && key != Keys::Types)
        {
            AddIssue(issues, ItemDataIssueSeverity::Warning, source, key, UnknownField);
        }
    }
}
} // namespace

std::string_view GetEffectTypesFileName(EffectKind kind)
{
    constexpr std::array<std::string_view, EffectKindCount> FileNames = {"EffectTypes.json", "ParticleTypes.json",
                                                                         "JointTypes.json", "SpriteTypes.json"};
    return FileNames[ToIndex(kind)];
}

void ReadEffectTypesJson(std::string_view text, const std::string& source, EffectKind kind,
                         std::vector<EffectTypeEntry>& types, std::vector<ItemDataIssue>& issues)
{
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, EffectTypesFormatVersion, root, issues))
    {
        return;
    }
    if (!HasKind(root, kind))
    {
        AddError(issues, source, Keys::Kind, "missing or not \"" + std::string(GetEffectKindName(kind)) + "\"");
        return;
    }

    const auto list = root.find(Keys::Types);
    if (list == root.end() || !list->is_array())
    {
        AddError(issues, source, Keys::Types, "missing or not a list of types");
        return;
    }
    for (size_t index = 0; index < list->size(); ++index)
    {
        EffectTypeEntry entry;
        if (ReadEntry((*list)[index], index, source, kind, entry, issues))
        {
            types.push_back(std::move(entry));
        }
    }
    WarnUnknownRootFields(root, source, issues);
}

void ValidateEffectTypes(EffectKind kind, std::span<const EffectTypeEntry> types, const std::string& source,
                         std::vector<ItemDataIssue>& issues)
{
    std::set<std::string_view> codes;
    for (const EffectTypeSymbol& symbol : GetEffectTypeSymbols(kind))
    {
        codes.insert(symbol.code);
    }

    std::set<std::string_view> names;
    std::map<std::string_view, std::string_view> nameOfCode;
    for (const EffectTypeEntry& entry : types)
    {
        if (!IsTypeName(entry.name))
        {
            AddError(issues, source, entry.name, "is not a name of letters and digits that starts with a letter");
        }
        else if (!names.insert(entry.name).second)
        {
            AddError(issues, source, entry.name, "is the name of more than one type");
        }

        if (!codes.contains(entry.code))
        {
            AddError(issues, source, entry.name, "the code " + entry.code + " is no type of this kind");
            continue;
        }
        const auto [named, inserted] = nameOfCode.emplace(entry.code, entry.name);
        if (!inserted)
        {
            AddError(issues, source, entry.name, entry.code + " is already named " + std::string(named->second));
        }
    }

    for (const std::string_view code : codes)
    {
        if (!nameOfCode.contains(code))
        {
            AddError(issues, source, std::string(code), "has no name; every type the code uses needs one");
        }
    }
}

std::string WriteEffectTypesJson(EffectKind kind, std::span<const EffectTypeEntry> types)
{
    std::vector<const EffectTypeEntry*> sorted;
    for (const EffectTypeEntry& entry : types)
    {
        sorted.push_back(&entry);
    }
    std::sort(sorted.begin(), sorted.end(), [](const EffectTypeEntry* left, const EffectTypeEntry* right)
              { return std::tie(left->name, left->code) < std::tie(right->name, right->code); });

    OrderedJson root;
    root[Items::Json::Keys::FormatVersion] = EffectTypesFormatVersion;
    root[Keys::Kind] = std::string(GetEffectKindName(kind));
    root[Keys::Types] = OrderedJson::array();
    for (const EffectTypeEntry* entry : sorted)
    {
        OrderedJson json;
        json[Keys::Name] = entry->name;
        json[Keys::Code] = entry->code;
        if (entry->create && kind == EffectKind::Effect)
        {
            json[CreateKey] = WriteEffectCreateParams(*entry->create);
        }
        root[Keys::Types].push_back(std::move(json));
    }
    std::string text = root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    text = Items::Json::PutListsOnOneLine(text, CreateLightKey);
    for (const char* key : CreateVectorKeys)
    {
        text = Items::Json::PutObjectsOnOneLine(Items::Json::PutListsOnOneLine(text, key), key);
    }
    return text + "\n";
}
} // namespace Data::Effects
