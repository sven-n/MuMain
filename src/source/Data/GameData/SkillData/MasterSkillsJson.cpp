#include "stdafx.h"

#include "MasterSkillsJson.h"

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "Data/GameData/ItemData/ItemJsonCommon.h"

#include <algorithm>
#include <map>

namespace Data::Skills
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

namespace Keys
{
constexpr const char* MasterSkills = "masterSkills";
constexpr const char* Skill = "skill";
constexpr const char* Raises = "raises";
constexpr const char* How = "how";
constexpr const char* ExtendsDuration = "extendsDuration";
constexpr const char* CastsBuff = "castsBuff";
} // namespace Keys

constexpr const char* AddName = "add";
constexpr const char* MultiplyName = "multiply";

const Items::Json::KeySet EntryKeys{Keys::Skill, Keys::Raises, Keys::How, Keys::ExtendsDuration, Keys::CastsBuff};

void AddIssue(std::vector<ItemDataIssue>& issues, ItemDataIssueSeverity severity, const std::string& source,
              const std::string& field, const std::string& message)
{
    issues.push_back({severity, source, ItemDataIssue::NoItem, ItemDataIssue::NoItem, field, message});
}

// One entry; false, with a fatal problem added, at the first value of the wrong form.
bool ReadEntry(const OrderedJson& json, size_t index, const std::string& source, MasterSkillDefinition& skill,
               std::vector<ItemDataIssue>& issues)
{
    const auto fail = [&](const std::string& field, const std::string& message)
    {
        AddIssue(issues, ItemDataIssueSeverity::Fatal, source, field, message);
        return false;
    };
    if (!json.is_object())
    {
        return fail(MasterSkillField(index, ""), R"(must be an object: {"skill": 420, "raises": ..., "how": ...})");
    }
    static const OrderedJson Missing;
    const auto whole = [&](const char* key, int low, int high, int& value)
    {
        const auto found = json.find(key);
        return Items::Json::ReadWholeNumberInRange(
            found == json.end() ? Missing : *found, MasterSkillField(index, key), low, high, value,
            [&](const std::string& field, const std::string& message) { fail(field, message); });
    };
    if (!whole(Keys::Skill, 1, AT_SKILL_MASTER_END, skill.skill))
    {
        return false;
    }
    if (json.contains(Keys::Raises) &&
        !Items::Json::ReadNameField(json, Keys::Raises, MasterSkillField(index, ""), skill.raises,
                                    [&](const std::string& field, const std::string& message)
                                    { fail(field, message); }))
    {
        return false;
    }
    if (const auto how = json.find(Keys::How); how != json.end())
    {
        const std::string text = how->is_string() ? how->get<std::string>() : "";
        if (text != AddName && text != MultiplyName)
        {
            return fail(MasterSkillField(index, Keys::How), std::string("must be ") + AddName + " or " + MultiplyName);
        }
        skill.how = text == MultiplyName ? MasterSkillHow::Multiply : MasterSkillHow::Add;
    }
    if (const auto extends = json.find(Keys::ExtendsDuration); extends != json.end())
    {
        if (!extends->is_boolean())
        {
            return fail(MasterSkillField(index, Keys::ExtendsDuration), "must be true or false");
        }
        skill.extendsDuration = extends->get<bool>();
    }
    if (json.contains(Keys::CastsBuff) && !whole(Keys::CastsBuff, 1, Buffs::MaxBuffNumber, skill.castsBuff))
    {
        return false;
    }
    Items::Json::WarnAboutUnknownKeys(json, EntryKeys, MasterSkillField(index, "") + ".",
                                      [&](const std::string& field, const std::string& message)
                                      { AddIssue(issues, ItemDataIssueSeverity::Warning, source, field, message); });
    return true;
}

OrderedJson WriteEntry(const MasterSkillDefinition& skill)
{
    OrderedJson json = OrderedJson::object();
    json[Keys::Skill] = skill.skill;
    if (!skill.raises.empty())
    {
        json[Keys::Raises] = skill.raises;
        json[Keys::How] = skill.how == MasterSkillHow::Multiply ? MultiplyName : AddName;
    }
    if (skill.extendsDuration)
    {
        json[Keys::ExtendsDuration] = true;
    }
    if (skill.castsBuff != 0)
    {
        json[Keys::CastsBuff] = skill.castsBuff;
    }
    return json;
}

size_t FileEntry(std::span<const size_t> entries, size_t position)
{
    return position < entries.size() ? entries[position] : position;
}
} // namespace

std::string MasterSkillField(size_t entry, std::string_view key)
{
    std::string field = std::string(Keys::MasterSkills) + "[" + std::to_string(entry) + "]";
    return key.empty() ? field : field + "." + std::string(key);
}

void ReadMasterSkillsJson(std::string_view text, const std::string& source, std::vector<MasterSkillDefinition>& skills,
                          std::vector<Items::ItemDataIssue>& issues, std::vector<size_t>* fileEntries)
{
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, MasterSkillsFormatVersion, root, issues))
    {
        return;
    }
    Items::Json::WarnAboutUnknownKeys(root, {Items::Json::Keys::FormatVersion, Keys::MasterSkills}, "",
                                      [&](const std::string& field, const std::string& message)
                                      { AddIssue(issues, ItemDataIssueSeverity::Warning, source, field, message); });
    const auto entries = root.find(Keys::MasterSkills);
    if (entries == root.end() || !entries->is_array())
    {
        AddIssue(issues, ItemDataIssueSeverity::Fatal, source, Keys::MasterSkills,
                 "missing or not a list of master skills");
        return;
    }
    for (size_t index = 0; index < entries->size(); ++index)
    {
        MasterSkillDefinition skill;
        if (ReadEntry((*entries)[index], index, source, skill, issues))
        {
            skills.push_back(std::move(skill));
            if (fileEntries != nullptr)
            {
                fileEntries->push_back(index);
            }
        }
    }
}

std::string WriteMasterSkillsJson(std::span<const MasterSkillDefinition> skills)
{
    OrderedJson entries = OrderedJson::array();
    for (const MasterSkillDefinition& skill : skills)
    {
        entries.push_back(WriteEntry(skill));
    }
    OrderedJson root = OrderedJson::object();
    root[Items::Json::Keys::FormatVersion] = MasterSkillsFormatVersion;
    root[Keys::MasterSkills] = std::move(entries);
    return root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace) + "\n";
}

void ValidateMasterSkills(std::span<const MasterSkillDefinition> skills, std::span<const size_t> entries,
                          const std::string& source, std::vector<Items::ItemDataIssue>& issues)
{
    std::map<int, size_t> seen;
    for (size_t position = 0; position < skills.size(); ++position)
    {
        const MasterSkillDefinition& skill = skills[position];
        const size_t entry = FileEntry(entries, position);
        if (const auto [found, added] = seen.emplace(skill.skill, position); !added)
        {
            AddIssue(issues, ItemDataIssueSeverity::Fatal, source, MasterSkillField(entry, Keys::Skill),
                     std::to_string(skill.skill) + " is masterSkills[" +
                         std::to_string(FileEntry(entries, found->second)) + "] too");
        }
        if (!skill.raises.empty() && !Buffs::IsBuffAttribute(skill.raises))
        {
            AddIssue(issues, ItemDataIssueSeverity::Error, source, MasterSkillField(entry, Keys::Raises),
                     skill.raises + " is no attribute a buff can raise (BuffAttributes.h)");
        }
        if (skill.raises.empty() && !skill.extendsDuration && skill.castsBuff == 0)
        {
            AddIssue(issues, ItemDataIssueSeverity::Warning, source, MasterSkillField(entry, ""),
                     "raises nothing, extends no duration and casts no other buff: the entry does nothing");
        }
    }
}

void ValidateMasterSkillBuffs(std::span<const MasterSkillDefinition> skills, std::span<const size_t> entries,
                              std::span<const Buffs::BuffDefinition> buffs,
                              const std::function<int(int skill)>& castBuff, const std::string& source,
                              std::vector<Items::ItemDataIssue>& issues)
{
    const auto findBuff = [&](int number)
    {
        const auto found = std::find_if(buffs.begin(), buffs.end(),
                                        [number](const Buffs::BuffDefinition& buff) { return buff.number == number; });
        return found != buffs.end() ? &*found : nullptr;
    };
    for (size_t position = 0; position < skills.size(); ++position)
    {
        const MasterSkillDefinition& skill = skills[position];
        const size_t entry = FileEntry(entries, position);
        if (skill.castsBuff != 0 && findBuff(skill.castsBuff) == nullptr)
        {
            AddIssue(issues, ItemDataIssueSeverity::Error, source, MasterSkillField(entry, Keys::CastsBuff),
                     std::to_string(skill.castsBuff) + " is no buff of Buffs.json");
            continue;
        }
        if (skill.raises.empty())
        {
            continue;
        }
        const int cast = castBuff ? castBuff(skill.skill) : 0;
        const int number = cast != 0 ? cast : skill.castsBuff;
        const Buffs::BuffDefinition* buff = number != 0 ? findBuff(number) : nullptr;
        if (buff == nullptr)
        {
            AddIssue(issues, ItemDataIssueSeverity::Error, source, MasterSkillField(entry, Keys::Raises),
                     "skill " + std::to_string(skill.skill) + " casts no buff of Buffs.json, so " + skill.raises +
                         " raises nothing");
            continue;
        }
        const bool raisesPowerUp =
            std::any_of(buff->powerUps.begin(), buff->powerUps.end(),
                        [&](const Buffs::BuffPowerUp& powerUp) { return powerUp.attribute == skill.raises; });
        if (!raisesPowerUp)
        {
            AddIssue(issues, ItemDataIssueSeverity::Error, source, MasterSkillField(entry, Keys::Raises),
                     buff->name + " (" + std::to_string(buff->number) + ") has no power-up " + skill.raises +
                         ": the master skill adds nothing");
        }
    }
}
} // namespace Data::Skills
