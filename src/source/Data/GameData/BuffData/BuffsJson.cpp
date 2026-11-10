#include "stdafx.h"

#include "BuffsJson.h"

#include "Core/Globals/_define.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <map>
#include <optional>
#include <set>
#include <utility>

namespace Data::Buffs
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

namespace Keys
{
constexpr const char* Buffs = "buffs";
constexpr const char* Number = "number";
constexpr const char* Name = "name";
constexpr const char* Kind = "kind";
constexpr const char* Group = "group";
constexpr const char* DisplayName = "displayName";
constexpr const char* Description = "description";
constexpr const char* Item = "item";
constexpr const char* Family = "family";
constexpr const char* Tier = "tier";
constexpr const char* Hidden = "hidden";
constexpr const char* CancelSkill = "cancelSkill";
} // namespace Keys

constexpr const char* BuffKindName = "buff";
constexpr const char* DebuffKindName = "debuff";
// A buff's group is a byte, as in the old BuffEffect files.
constexpr int MaxBuffGroup = 255;

const Items::Json::KeySet EntryKeys{Keys::Number,      Keys::Name,        Keys::Kind,       Keys::Group,
                                    Keys::DisplayName, Keys::Description, Keys::Item,       Keys::Family,
                                    Keys::Tier,        Keys::Hidden,      Keys::CancelSkill};
const Items::Json::KeySet ItemKeys{Keys::Group, Keys::Number};

void AddIssue(std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& source,
              const std::string& field, const std::string& message)
{
    issues.push_back({severity, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
}

// The fields of one entry; false, with a fatal problem added, at the first value of the wrong form.
class EntryReader
{
public:
    EntryReader(size_t index, const std::string& source, std::vector<ItemDataIssue>& issues)
        : m_index(index), m_source(source), m_issues(issues)
    {
    }

    bool Read(const OrderedJson& json, BuffDefinition& buff)
    {
        if (!json.is_object())
        {
            return Fail("", "must be an object");
        }
        return ReadWhole(json, Keys::Number, 1, MaxBuffNumber, buff.number) && ReadName(json, buff.name) &&
               ReadKind(json, buff.kind) && ReadWhole(json, Keys::Group, 0, MaxBuffGroup, buff.group) &&
               ReadText(json, Keys::DisplayName, true, buff.displayName) &&
               ReadText(json, Keys::Description, false, buff.description) && ReadItem(json, buff.item) &&
               ReadFamily(json, buff) && ReadHidden(json, buff.hidden) && ReadCancelSkill(json, buff.cancelSkill) &&
               WarnUnknown(json, EntryKeys, "");
    }

private:
    bool Fail(const std::string& key, const std::string& message)
    {
        AddFatal(BuffField(m_index, key), message);
        return false;
    }

    void AddFatal(const std::string& field, const std::string& message)
    {
        AddIssue(m_issues, ItemDataIssueSeverity::Fatal, m_source, field, message);
    }

    // The whole number at `key` of `json`, which is the entry's field `prefix` ("" for the entry itself).
    bool ReadWhole(const OrderedJson& json, const std::string& prefix, const char* key, int low, int high, int& value)
    {
        const std::string field = BuffField(m_index, prefix.empty() ? std::string(key) : prefix + "." + key);
        // A missing field reads as null, which is no whole number.
        static const OrderedJson Missing;
        const auto found = json.find(key);
        return Items::Json::ReadWholeNumberInRange(found == json.end() ? Missing : *found, field, low, high, value,
                                                   [this](const std::string& name, const std::string& message)
                                                   { AddFatal(name, message); });
    }

    bool ReadWhole(const OrderedJson& json, const char* key, int low, int high, int& value)
    {
        return ReadWhole(json, "", key, low, high, value);
    }

    bool ReadName(const OrderedJson& json, std::string& name)
    {
        return Items::Json::ReadNameField(json, Keys::Name, BuffField(m_index, ""), name,
                                          [this](const std::string& field, const std::string& message)
                                          { AddFatal(field, message); });
    }

    bool ReadKind(const OrderedJson& json, BuffKind& kind)
    {
        const auto field = json.find(Keys::Kind);
        const std::string text = field != json.end() && field->is_string() ? field->get<std::string>() : "";
        if (text == BuffKindName)
        {
            kind = BuffKind::Buff;
            return true;
        }
        if (text == DebuffKindName)
        {
            kind = BuffKind::Debuff;
            return true;
        }
        return Fail(Keys::Kind, std::string("must be ") + BuffKindName + " or " + DebuffKindName);
    }

    bool ReadText(const OrderedJson& json, const char* key, bool required, LocalizedString& text)
    {
        const auto field = json.find(key);
        if (field == json.end())
        {
            return !required || Fail(key, "missing");
        }
        bool valid = Items::Json::ReadLocalizedString(
            *field, BuffField(m_index, key), text, [this](const std::string& name, const std::string& message)
            { AddIssue(m_issues, ItemDataIssueSeverity::Fatal, m_source, name, message); });
        if (valid && text.GetNeutral().empty())
        {
            valid = Fail(key, "must have an English text");
        }
        return valid;
    }

    bool ReadItem(const OrderedJson& json, std::optional<BuffItemLink>& item)
    {
        const auto field = json.find(Keys::Item);
        if (field == json.end())
        {
            return true;
        }
        if (!field->is_object())
        {
            return Fail(Keys::Item, "must be an object with group and number");
        }
        BuffItemLink link;
        if (!ReadWhole(*field, Keys::Item, Keys::Group, 0, MAX_ITEM_TYPE - 1, link.group) ||
            !ReadWhole(*field, Keys::Item, Keys::Number, 0, MAX_ITEM_INDEX - 1, link.number))
        {
            return false;
        }
        item = link;
        return WarnUnknown(*field, ItemKeys, Keys::Item);
    }

    bool ReadFamily(const OrderedJson& json, BuffDefinition& buff)
    {
        const auto family = json.find(Keys::Family);
        const auto tier = json.find(Keys::Tier);
        if (family == json.end())
        {
            return tier == json.end() || Fail(Keys::Tier, "needs a family");
        }
        if (!family->is_string() || !Items::Json::IsName(family->get_ref<const std::string&>()))
        {
            return Fail(Keys::Family, "must be a name of letters and digits");
        }
        buff.family = family->get<std::string>();
        return tier == json.end() || ReadWhole(json, Keys::Tier, 0, MaxBuffTier, buff.tier);
    }

    bool ReadHidden(const OrderedJson& json, bool& hidden)
    {
        const auto field = json.find(Keys::Hidden);
        if (field == json.end())
        {
            return true;
        }
        if (!field->is_boolean())
        {
            return Fail(Keys::Hidden, "must be true or false");
        }
        hidden = field->get<bool>();
        return true;
    }

    bool ReadCancelSkill(const OrderedJson& json, int& skill)
    {
        return !json.contains(Keys::CancelSkill) || ReadWhole(json, Keys::CancelSkill, 1, MAX_SKILLS - 1, skill);
    }

    bool WarnUnknown(const OrderedJson& json, const Items::Json::KeySet& keys, const std::string& key)
    {
        Items::Json::WarnAboutUnknownKeys(
            json, keys, BuffField(m_index, key) + ".", [&](const std::string& field, const std::string& message)
            { AddIssue(m_issues, ItemDataIssueSeverity::Warning, m_source, field, message); });
        return true;
    }

    size_t m_index;
    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
};

OrderedJson WriteEntry(const BuffDefinition& buff)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::Number] = buff.number;
    json[Keys::Name] = buff.name;
    json[Keys::Kind] = buff.kind == BuffKind::Debuff ? DebuffKindName : BuffKindName;
    json[Keys::Group] = buff.group;
    json[Keys::DisplayName] = Items::Json::WriteLocalizedString(buff.displayName);
    if (!buff.description.IsEmpty())
    {
        json[Keys::Description] = Items::Json::WriteLocalizedString(buff.description);
    }
    if (buff.item)
    {
        OrderedJson item = OrderedJson::object();
        item[Keys::Group] = buff.item->group;
        item[Keys::Number] = buff.item->number;
        json[Keys::Item] = std::move(item);
    }
    if (!buff.family.empty())
    {
        json[Keys::Family] = buff.family;
        if (buff.tier != 0)
        {
            json[Keys::Tier] = buff.tier;
        }
    }
    if (buff.hidden)
    {
        json[Keys::Hidden] = true;
    }
    if (buff.cancelSkill != 0)
    {
        json[Keys::CancelSkill] = buff.cancelSkill;
    }
    return json;
}
} // namespace

std::string BuffField(size_t entry, std::string_view key)
{
    std::string field = std::string(Keys::Buffs) + "[" + std::to_string(entry) + "]";
    return key.empty() ? field : field + "." + std::string(key);
}

void ReadBuffsJson(std::string_view text, const std::string& source, std::vector<BuffDefinition>& buffs,
                   std::vector<Items::ItemDataIssue>& issues, std::vector<size_t>* fileEntries)
{
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, BuffsFormatVersion, root, issues))
    {
        return;
    }
    Items::Json::WarnAboutUnknownKeys(root, {Items::Json::Keys::FormatVersion, Keys::Buffs}, "",
                                      [&](const std::string& field, const std::string& message)
                                      { AddIssue(issues, ItemDataIssueSeverity::Warning, source, field, message); });
    const auto entries = root.find(Keys::Buffs);
    if (entries == root.end() || !entries->is_array())
    {
        AddIssue(issues, ItemDataIssueSeverity::Fatal, source, Keys::Buffs, "missing or not a list of buffs");
        return;
    }
    for (size_t index = 0; index < entries->size(); ++index)
    {
        BuffDefinition buff;
        if (EntryReader(index, source, issues).Read((*entries)[index], buff))
        {
            buffs.push_back(std::move(buff));
            if (fileEntries != nullptr)
            {
                fileEntries->push_back(index);
            }
        }
    }
}

std::string WriteBuffsJson(std::span<const BuffDefinition> buffs)
{
    OrderedJson entries = OrderedJson::array();
    for (const BuffDefinition& buff : buffs)
    {
        entries.push_back(WriteEntry(buff));
    }
    OrderedJson root = OrderedJson::object();
    root[Items::Json::Keys::FormatVersion] = BuffsFormatVersion;
    root[Keys::Buffs] = std::move(entries);
    return root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace) + "\n";
}

void ValidateBuffs(std::span<const BuffDefinition> buffs, std::span<const size_t> entries, const std::string& source,
                   std::vector<Items::ItemDataIssue>& issues)
{
    const auto fileEntry = [&](size_t position) { return position < entries.size() ? entries[position] : position; };
    std::map<int, size_t> numbers;
    std::map<std::string, size_t, std::less<>> names;
    std::set<std::pair<std::string, int>> tiers;
    std::map<std::string, BuffKind, std::less<>> familyKinds;
    for (size_t position = 0; position < buffs.size(); ++position)
    {
        const BuffDefinition& buff = buffs[position];
        const size_t entry = fileEntry(position);
        if (const auto [found, added] = numbers.emplace(buff.number, position); !added)
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, source, BuffField(entry, Keys::Number),
                     std::to_string(buff.number) + " is buffs[" + std::to_string(fileEntry(found->second)) + "] too");
        }
        if (const auto [found, added] = names.emplace(buff.name, position); !added)
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, source, BuffField(entry, Keys::Name),
                     buff.name + " is buffs[" + std::to_string(fileEntry(found->second)) + "] too");
        }
        if (!buff.family.empty() && !tiers.emplace(buff.family, buff.tier).second)
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, source, BuffField(entry, Keys::Tier),
                     "family " + buff.family + " has tier " + std::to_string(buff.tier) + " twice: which would show?");
        }
        if (!buff.family.empty() && familyKinds.emplace(buff.family, buff.kind).first->second != buff.kind)
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, source, BuffField(entry, Keys::Kind),
                     "family " + buff.family + " has buffs and debuffs: its tiers are one buff");
        }
    }
}

void ValidateBuffItems(std::span<const BuffDefinition> buffs, std::span<const size_t> entries,
                       const std::function<bool(int group, int number)>& itemExists, const std::string& source,
                       std::vector<ItemDataIssue>& issues)
{
    for (size_t position = 0; position < buffs.size(); ++position)
    {
        const BuffDefinition& buff = buffs[position];
        if (!buff.item || itemExists(buff.item->group, buff.item->number))
        {
            continue;
        }
        const size_t entry = position < entries.size() ? entries[position] : position;
        AddIssue(issues, ItemDataIssueSeverity::Error, source, BuffField(entry, Keys::Item),
                 "names item (" + std::to_string(buff.item->group) + ", " + std::to_string(buff.item->number) +
                     "), which the item list does not have; the buff gets no duration or values from it");
    }
}
} // namespace Data::Buffs
