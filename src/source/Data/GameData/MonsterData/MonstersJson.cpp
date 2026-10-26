#include "stdafx.h"

#include "MonstersJson.h"

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterSoundIds.h"

#include <algorithm>
#include <optional>
#include <set>

namespace Data::Monsters
{
namespace
{
using Items::ItemDataIssue;
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

constexpr const char* MonstersKey = "monsters";
constexpr const char* NumberKey = "number";
constexpr const char* NameKey = "name";
constexpr const char* KindKey = "kind";
constexpr const char* ModelKey = "model";
constexpr const char* AttacksKey = "attacks";
constexpr const char* MonsterSkillsKey = "monsterSkills";
constexpr const char* SkillsKey = "skills";
constexpr const char* OtherwiseKey = "otherwise";
constexpr const char* OtherwiseRandomKey = "otherwiseRandom";
constexpr const char* ClearSkillKey = "clearSkill";
constexpr const char* SoundKey = "sound";
constexpr const char* SoundAtMonsterKey = "soundAtMonster";
constexpr const char* NoAnimationName = "none";
// The skill numbers the packets carry (a WORD; 65535 is "no skill").
constexpr long long MaxSkillNumber = 65534;

// The problems of one file; a problem of a monster names its number, so the
// editor can select it.
class IssueList
{
public:
    IssueList(const std::string& source, std::vector<ItemDataIssue>& issues) : m_source(source), m_issues(issues) {}

    void Error(const std::string& field, const std::string& message, int monster = ItemDataIssue::NoItem)
    {
        Add(ItemDataIssueSeverity::Error, field, message, monster);
    }

    void Warning(const std::string& field, const std::string& message, int monster = ItemDataIssue::NoItem)
    {
        Add(ItemDataIssueSeverity::Warning, field, message, monster);
    }

private:
    void Add(ItemDataIssueSeverity severity, const std::string& field, const std::string& message, int monster)
    {
        m_issues.push_back({severity, m_source, ItemDataIssue::NoItem, monster, field, message});
    }

    const std::string& m_source;
    std::vector<ItemDataIssue>& m_issues;
};

std::string EntryField(size_t index)
{
    return std::string(MonstersKey) + "[" + std::to_string(index) + "]";
}

// How the problems after the number name the monster: "monster 523 name".
std::string MonsterField(int number, const char* key)
{
    return "monster " + std::to_string(number) + " " + key;
}

std::optional<int> ReadMonsterNumber(const OrderedJson& json, const std::string& field, IssueList& issues)
{
    const auto value = json.find(NumberKey);
    long long number = 0;
    if (value == json.end() || !value->is_number_integer() || !Items::Json::ReadWholeNumber(*value, number) ||
        number < 0 || number >= MonsterNumberCount)
    {
        issues.Error(field + "." + NumberKey,
                     "missing or not a whole number from 0 to " + std::to_string(MonsterNumberCount - 1));
        return std::nullopt;
    }
    return static_cast<int>(number);
}

bool ReadNames(const OrderedJson& json, int number, LocalizedString& names, IssueList& issues)
{
    const auto value = json.find(NameKey);
    if (value == json.end())
    {
        issues.Error(MonsterField(number, NameKey), "missing", number);
        return false;
    }
    return Items::Json::ReadLocalizedString(*value, MonsterField(number, NameKey), names,
                                            [&](const std::string& field, const std::string& message)
                                            { issues.Error(field, message, number); });
}

bool ReadKind(const OrderedJson& json, int number, MonsterKind& kind, IssueList& issues)
{
    const auto value = json.find(KindKey);
    const std::optional<MonsterKind> found =
        value != json.end() && value->is_string() ? FindMonsterKind(value->get<std::string>()) : std::nullopt;
    if (!found)
    {
        issues.Error(MonsterField(number, KindKey), "missing or not monster, npc, trap or other", number);
        return false;
    }
    kind = *found;
    return true;
}

bool ReadModel(const OrderedJson& json, int number, std::string& model, IssueList& issues)
{
    const auto value = json.find(ModelKey);
    if (value == json.end())
    {
        return true;
    }
    if (!value->is_string() || !FindMonsterModelReference(value->get<std::string>()))
    {
        issues.Error(MonsterField(number, ModelKey),
                     "must be a model of the model list (MONSTER_MODEL_..., MODEL_...), player, MODEL_WARCRAFT, "
                     "MODEL_BALL or mapObject <number>",
                     number);
        return false;
    }
    model = value->get<std::string>();
    return true;
}

// An animation name (MonsterAnimations.h) or "none".
std::optional<int> ReadAttackAnimation(const OrderedJson& value)
{
    if (!value.is_string())
    {
        return std::nullopt;
    }
    const std::string name = value.get<std::string>();
    if (name == NoAnimationName)
    {
        return NoAttackAnimation;
    }
    return FindMonsterAnimation(name);
}

std::string AttackAnimationName(int animation)
{
    return animation == NoAttackAnimation ? std::string(NoAnimationName) : GetMonsterAnimationName(animation);
}

bool ReadAttackEntries(const OrderedJson& json, const char* key, int number, std::vector<MonsterAttackEntry>& entries,
                       IssueList& issues)
{
    const auto value = json.find(key);
    if (value == json.end())
    {
        return true;
    }
    const std::string field = MonsterField(number, AttacksKey) + "." + key;
    if (!value->is_object())
    {
        issues.Error(field, "must be an object of skill numbers and animations", number);
        return false;
    }
    bool valid = true;
    for (auto entry = value->begin(); entry != value->end(); ++entry)
    {
        const std::string& skillText = entry.key();
        long long skill = -1;
        // Without leading zeros: "7" and "007" would both be skill 7.
        const bool isNumber = !skillText.empty() && skillText.size() <= 5 &&
                              skillText.find_first_not_of("0123456789") == std::string::npos &&
                              (skillText.size() == 1 || skillText.front() != '0');
        if (isNumber)
        {
            skill = std::stoll(skillText);
        }
        if (!isNumber || skill > MaxSkillNumber)
        {
            issues.Error(field + "." + skillText,
                         "must be a skill number from 0 to " + std::to_string(MaxSkillNumber) +
                             ", without leading zeros",
                         number);
            valid = false;
            continue;
        }
        const std::optional<int> animation = ReadAttackAnimation(entry.value());
        if (!animation)
        {
            issues.Error(field + "." + skillText, "must be an animation (attack1, shock, appear, ...) or none", number);
            valid = false;
            continue;
        }
        entries.emplace_back(static_cast<int>(skill), *animation);
    }
    std::sort(entries.begin(), entries.end());
    return valid;
}

bool ReadAttacks(const OrderedJson& json, int number, std::optional<MonsterAttacks>& attacks, IssueList& issues)
{
    const auto value = json.find(AttacksKey);
    if (value == json.end())
    {
        return true;
    }
    const std::string field = MonsterField(number, AttacksKey);
    if (!value->is_object())
    {
        issues.Error(field, "must be an object", number);
        return false;
    }
    MonsterAttacks read;
    bool valid = ReadAttackEntries(*value, MonsterSkillsKey, number, read.monsterSkills, issues);
    valid = ReadAttackEntries(*value, SkillsKey, number, read.skills, issues) && valid;
    const auto otherwise = value->find(OtherwiseKey);
    const auto random = value->find(OtherwiseRandomKey);
    if (otherwise != value->end() && random != value->end())
    {
        issues.Error(field, "has otherwise or otherwiseRandom, not both", number);
        valid = false;
    }
    if (otherwise != value->end())
    {
        const std::optional<int> animation = ReadAttackAnimation(*otherwise);
        if (animation)
        {
            read.otherwise = *animation;
        }
        else
        {
            issues.Error(field + "." + OtherwiseKey, "must be an animation (attack1, shock, appear, ...) or none",
                         number);
            valid = false;
        }
    }
    if (random != value->end())
    {
        read.otherwiseRandom = random->is_string() ? FindMonsterAttackRandom(random->get<std::string>()) : std::nullopt;
        if (!read.otherwiseRandom)
        {
            issues.Error(field + "." + OtherwiseRandomKey, "must be rand, randFps or bali", number);
            valid = false;
        }
    }
    const auto clear = value->find(ClearSkillKey);
    if (clear != value->end())
    {
        const std::optional<MonsterSkillClear> found =
            clear->is_string() ? FindMonsterSkillClear(clear->get<std::string>()) : std::nullopt;
        if (found)
        {
            read.clearSkill = *found;
        }
        else
        {
            issues.Error(field + "." + ClearSkillKey, "must be never, onMatch, onNoMatch, always or afterAttack",
                         number);
            valid = false;
        }
    }
    if (const auto sound = value->find(SoundKey); sound != value->end())
    {
        if (sound->is_string() && FindMonsterSoundId(sound->get<std::string>()))
        {
            read.sound = sound->get<std::string>();
        }
        else
        {
            issues.Error(field + "." + SoundKey, "must be a SOUND_ id that the monster data can name", number);
            valid = false;
        }
    }
    if (const auto atMonster = value->find(SoundAtMonsterKey); atMonster != value->end())
    {
        if (atMonster->is_boolean())
        {
            read.soundAtMonster = atMonster->get<bool>();
        }
        else
        {
            issues.Error(field + "." + SoundAtMonsterKey, "must be true or false", number);
            valid = false;
        }
    }
    Items::Json::WarnAboutUnknownKeys(
        *value,
        {MonsterSkillsKey, SkillsKey, OtherwiseKey, OtherwiseRandomKey, ClearSkillKey, SoundKey, SoundAtMonsterKey},
        field + " ",
        [&](const std::string& unknown, const std::string& message) { issues.Warning(unknown, message, number); });
    if (valid)
    {
        attacks = std::move(read);
    }
    return valid;
}

OrderedJson WriteAttackEntries(const std::vector<MonsterAttackEntry>& entries)
{
    OrderedJson json = OrderedJson::object();
    for (const auto& [skill, animation] : entries)
    {
        json[std::to_string(skill)] = AttackAnimationName(animation);
    }
    return json;
}

OrderedJson WriteAttacks(const MonsterAttacks& attacks)
{
    OrderedJson json = OrderedJson::object();
    if (!attacks.monsterSkills.empty())
    {
        json[MonsterSkillsKey] = WriteAttackEntries(attacks.monsterSkills);
    }
    if (!attacks.skills.empty())
    {
        json[SkillsKey] = WriteAttackEntries(attacks.skills);
    }
    if (attacks.otherwiseRandom)
    {
        json[OtherwiseRandomKey] = std::string(GetMonsterAttackRandomName(*attacks.otherwiseRandom));
    }
    else
    {
        json[OtherwiseKey] = AttackAnimationName(attacks.otherwise);
    }
    json[ClearSkillKey] = std::string(GetMonsterSkillClearName(attacks.clearSkill));
    if (!attacks.sound.empty())
    {
        json[SoundKey] = attacks.sound;
        if (attacks.soundAtMonster)
        {
            json[SoundAtMonsterKey] = true;
        }
    }
    return json;
}

std::optional<MonsterDefinition> ReadMonster(const OrderedJson& json, const std::string& field, IssueList& issues)
{
    if (!json.is_object())
    {
        issues.Error(field, "must be an object");
        return std::nullopt;
    }
    const std::optional<int> number = ReadMonsterNumber(json, field, issues);
    if (!number)
    {
        return std::nullopt;
    }
    MonsterDefinition monster;
    monster.number = *number;
    bool valid = ReadNames(json, monster.number, monster.names, issues);
    valid = ReadKind(json, monster.number, monster.kind, issues) && valid;
    valid = ReadModel(json, monster.number, monster.model, issues) && valid;
    valid = ReadAttacks(json, monster.number, monster.attacks, issues) && valid;
    Items::Json::WarnAboutUnknownKeys(json, {NumberKey, NameKey, KindKey, ModelKey, AttacksKey},
                                      "monster " + std::to_string(*number) + " ",
                                      [&](const std::string& unknown, const std::string& message)
                                      { issues.Warning(unknown, message, monster.number); });
    return valid ? std::optional<MonsterDefinition>(std::move(monster)) : std::nullopt;
}

OrderedJson WriteMonster(const MonsterDefinition& monster)
{
    OrderedJson json = OrderedJson::object();
    json[NumberKey] = monster.number;
    json[NameKey] = Items::Json::WriteLocalizedString(monster.names);
    json[KindKey] = GetMonsterKindName(monster.kind);
    if (!monster.model.empty())
    {
        json[ModelKey] = monster.model;
    }
    if (monster.attacks)
    {
        json[AttacksKey] = WriteAttacks(*monster.attacks);
    }
    return json;
}
} // namespace

void ReadMonstersJson(std::string_view text, const std::string& source, std::vector<MonsterDefinition>& monsters,
                      std::vector<ItemDataIssue>& issues)
{
    IssueList list(source, issues);
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, MonstersFormatVersion, root, issues))
    {
        return;
    }
    Items::Json::WarnAboutUnknownKeys(root, {Items::Json::Keys::FormatVersion, MonstersKey}, "",
                                      [&](const std::string& unknown, const std::string& message)
                                      { list.Warning(unknown, message); });
    const auto entries = root.find(MonstersKey);
    if (entries == root.end() || !entries->is_array())
    {
        list.Error(MonstersKey, "missing or not a list of monsters");
        return;
    }
    std::set<int> numbers;
    for (size_t index = 0; index < entries->size(); ++index)
    {
        std::optional<MonsterDefinition> monster = ReadMonster((*entries)[index], EntryField(index), list);
        if (!monster)
        {
            continue;
        }
        if (!numbers.insert(monster->number).second)
        {
            list.Error(MonsterField(monster->number, NumberKey), "is in the list more than once", monster->number);
            continue;
        }
        monsters.push_back(std::move(*monster));
    }
}

std::string WriteMonstersJson(std::span<const MonsterDefinition> monsters)
{
    std::vector<const MonsterDefinition*> sorted;
    sorted.reserve(monsters.size());
    for (const MonsterDefinition& monster : monsters)
    {
        sorted.push_back(&monster);
    }
    std::stable_sort(sorted.begin(), sorted.end(), [](const MonsterDefinition* left, const MonsterDefinition* right)
                     { return left->number < right->number; });

    OrderedJson entries = OrderedJson::array();
    for (const MonsterDefinition* monster : sorted)
    {
        entries.push_back(WriteMonster(*monster));
    }
    OrderedJson root = OrderedJson::object();
    root[Items::Json::Keys::FormatVersion] = MonstersFormatVersion;
    root[MonstersKey] = std::move(entries);
    const std::string text = root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    std::string compact = Items::Json::PutObjectsOnOneLine(text, NameKey);
    compact = Items::Json::PutObjectsOnOneLine(compact, MonsterSkillsKey);
    compact = Items::Json::PutObjectsOnOneLine(compact, SkillsKey);
    return compact + "\n";
}
} // namespace Data::Monsters
