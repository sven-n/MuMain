#include "stdafx.h"

#include "MonstersJson.h"

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterJsonCommon.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterSetupJson.h"
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
constexpr const char* UnknownKey = "unknownMonsters";
constexpr const char* UnknownModelKey = "model";
constexpr const char* UnknownScaleKey = "scale";
constexpr const char* UnknownGlowKey = "glow";
constexpr const char* UnknownMarksKey = "marks";
constexpr const char* UnknownMessageKey = "messageSeconds";
constexpr const char* UnknownForKey = "for";
constexpr const char* TalkTextsKey = "talkTexts";
constexpr const char* ForNoEntry = "noEntry";
constexpr const char* ForNoModel = "noModel";
constexpr const char* ForMissingFile = "missingFile";
constexpr const char* NumberKey = "number";
constexpr const char* NameKey = "name";
constexpr const char* KindKey = "kind";
constexpr const char* ModelKey = "model";
constexpr const char* AttacksKey = "attacks";
constexpr const char* MapVariantsKey = "mapVariants";
constexpr const char* SoundSetKey = "soundSet";
constexpr const char* RandomVariantsKey = "randomVariants";
constexpr const char* EquipmentKey = "equipment";
constexpr const char* NumbersKey = "numbers";
constexpr const char* MapsKey = "maps";
constexpr const char* MonsterSkillsKey = "monsterSkills";
constexpr const char* SkillsKey = "skills";
constexpr const char* OtherwiseKey = "otherwise";
constexpr const char* OtherwiseRandomKey = "otherwiseRandom";
constexpr const char* ClearSkillKey = "clearSkill";
constexpr const char* SoundKey = "sound";
constexpr const char* SoundAtMonsterKey = "soundAtMonster";
constexpr const char* NoAnimationName = "none";
// `otherwise`'s value for the generic pattern (Q-MN69).
constexpr const char* GenericPatternName = "generic";
// The skill numbers the packets carry (a WORD; 65535 is "no skill").
constexpr long long MaxSkillNumber = 65534;
// The version before monsters had numbers (D61): each entry is one number.
constexpr int OneNumberEntriesFormatVersion = 1;

// The problems of one file; a problem of a monster names its number, so the
// editor can select it.
class IssueList
{
public:
    IssueList(const std::string& source, std::vector<ItemDataIssue>& issues) : m_source(source), m_issues(issues) {}

    void Fatal(const std::string& field, const std::string& message, int monster = ItemDataIssue::NoItem)
    {
        Add(ItemDataIssueSeverity::Fatal, field, message, monster);
    }

    void Warning(const std::string& field, const std::string& message, int monster = ItemDataIssue::NoItem)
    {
        Add(ItemDataIssueSeverity::Warning, field, message, monster);
    }

    void Error(const std::string& field, const std::string& message, int monster = ItemDataIssue::NoItem)
    {
        Add(ItemDataIssueSeverity::Error, field, message, monster);
    }

    // The setup fields' problems of the monster.
    SetupProblem ForMonster(int monster)
    {
        return [this, monster](ItemDataIssueSeverity severity, const std::string& field, const std::string& message)
        { Add(severity, field, message, monster); };
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
        issues.Fatal(field + "." + NumberKey,
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
        issues.Fatal(MonsterField(number, NameKey), "missing", number);
        return false;
    }
    return Items::Json::ReadLocalizedString(*value, MonsterField(number, NameKey), names,
                                            [&](const std::string& field, const std::string& message)
                                            { issues.Fatal(field, message, number); });
}

bool ReadKind(const OrderedJson& json, int number, MonsterKind& kind, IssueList& issues)
{
    const auto value = json.find(KindKey);
    const std::optional<MonsterKind> found =
        value != json.end() && value->is_string() ? FindMonsterKind(value->get<std::string>()) : std::nullopt;
    if (!found)
    {
        issues.Fatal(MonsterField(number, KindKey), "missing or not monster, npc, trap or other", number);
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
        issues.Fatal(MonsterField(number, ModelKey),
                     "must be a model of the model list (MONSTER_MODEL_..., MODEL_...), player, MODEL_WARCRAFT, "
                     "MODEL_BALL or mapObject <number>",
                     number);
        return false;
    }
    model = value->get<std::string>();
    return true;
}

// The sound set the monster plays (Q-MN78); empty when the entry names none.
// Whether the set exists is checked with the sets (MonsterSoundValidation.h).
bool ReadSoundSet(const OrderedJson& json, const std::string& field, int number, std::string& soundSet,
                  IssueList& issues)
{
    const auto value = json.find(SoundSetKey);
    if (value == json.end())
    {
        return true;
    }
    if (!value->is_string() || value->get_ref<const std::string&>().empty())
    {
        issues.Fatal(field, "must be the name of a sound set", number);
        return false;
    }
    soundSet = value->get<std::string>();
    return true;
}

// The monster's other looks (D63, Q-MN83): each names its model, once, and
// what it changes.
bool ReadRandomVariants(const OrderedJson& json, const MonsterDefinition& monster,
                        std::vector<MonsterRandomVariant>& variants, IssueList& issues)
{
    const auto value = json.find(RandomVariantsKey);
    if (value == json.end())
    {
        return true;
    }
    const int number = monster.number;
    const std::string field = MonsterField(number, RandomVariantsKey);
    if (!value->is_array())
    {
        issues.Fatal(field, "must be a list of random variants", number);
        return false;
    }
    bool valid = true;
    for (size_t index = 0; index < value->size(); ++index)
    {
        const OrderedJson& entry = (*value)[index];
        const std::string key = field + "[" + std::to_string(index) + "]";
        const auto model = entry.is_object() ? entry.find(ModelKey) : entry.end();
        if (!entry.is_object() || model == entry.end() || !model->is_string() ||
            !FindMonsterModelReference(model->get<std::string>()))
        {
            issues.Fatal(key + "." + ModelKey, "must be the model of the look, as the monster's model names one",
                         number);
            valid = false;
            continue;
        }
        MonsterRandomVariant variant;
        variant.model = model->get<std::string>();
        const bool repeated = variant.model == monster.model ||
                              std::any_of(variants.begin(), variants.end(), [&](const MonsterRandomVariant& other)
                                          { return other.model == variant.model; });
        if (repeated)
        {
            issues.Fatal(key + "." + ModelKey, variant.model + " is the entry's model or another variant's", number);
            valid = false;
            continue;
        }
        std::string soundSet;
        if (!ReadSoundSet(entry, key + "." + SoundSetKey, number, soundSet, issues))
        {
            valid = false;
            continue;
        }
        if (!soundSet.empty())
        {
            variant.soundSet = std::move(soundSet);
        }
        if (const auto equipment = entry.find(EquipmentKey); equipment != entry.end())
        {
            MonsterEquipment worn;
            if (!ReadMonsterEquipment(*equipment, key + "." + EquipmentKey, worn, issues.ForMonster(number)))
            {
                valid = false;
                continue;
            }
            variant.equipment = std::move(worn);
        }
        Items::Json::WarnAboutUnknownKeys(entry, {ModelKey, SoundSetKey, EquipmentKey}, key + " ",
                                          [&](const std::string& unknown, const std::string& message)
                                          { issues.Warning(unknown, message, number); });
        variants.push_back(std::move(variant));
    }
    return valid;
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

// `attacksField` names the attacks in problems: "monster 459 attacks".
bool ReadAttackEntries(const OrderedJson& json, const char* key, const std::string& attacksField, int number,
                       std::vector<MonsterAttackEntry>& entries, IssueList& issues)
{
    const auto value = json.find(key);
    if (value == json.end())
    {
        return true;
    }
    const std::string field = attacksField + "." + key;
    if (!value->is_object())
    {
        issues.Fatal(field, "must be an object of skill numbers and animations", number);
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
            issues.Fatal(field + "." + skillText,
                         "must be a skill number from 0 to " + std::to_string(MaxSkillNumber) +
                             ", without leading zeros",
                         number);
            valid = false;
            continue;
        }
        const std::optional<int> animation = ReadAttackAnimation(entry.value());
        if (!animation)
        {
            issues.Fatal(field + "." + skillText, "must be an animation (attack1, shock, appear, ...) or none", number);
            valid = false;
            continue;
        }
        entries.emplace_back(static_cast<int>(skill), *animation);
    }
    std::sort(entries.begin(), entries.end());
    return valid;
}

// `otherwise`: one animation, the generic pattern, or a rotation of two or
// more animations.
bool ReadOtherwise(const OrderedJson& value, MonsterAttacks& attacks)
{
    if (value.is_string() && value.get<std::string>() == GenericPatternName)
    {
        attacks.otherwiseGeneric = true;
        return true;
    }
    if (const std::optional<int> animation = ReadAttackAnimation(value))
    {
        attacks.otherwise = {*animation};
        return true;
    }
    if (!value.is_array() || value.size() < 2)
    {
        return false;
    }
    std::vector<int> rotation;
    for (const OrderedJson& entry : value)
    {
        const std::optional<int> animation = ReadAttackAnimation(entry);
        if (!animation)
        {
            return false;
        }
        rotation.push_back(*animation);
    }
    attacks.otherwise = std::move(rotation);
    return true;
}

OrderedJson WriteOtherwise(const MonsterAttacks& attacks)
{
    if (attacks.otherwiseGeneric)
    {
        return GenericPatternName;
    }
    if (!IsRotation(attacks))
    {
        return AttackAnimationName(attacks.otherwise.empty() ? MONSTER01_ATTACK1 : attacks.otherwise.front());
    }
    OrderedJson rotation = OrderedJson::array();
    for (const int animation : attacks.otherwise)
    {
        rotation.push_back(AttackAnimationName(animation));
    }
    return rotation;
}

// The `attacks` of an entry or of a map variant; `field` names them in
// problems.
bool ReadAttacks(const OrderedJson& json, const std::string& field, int number, std::optional<MonsterAttacks>& attacks,
                 IssueList& issues)
{
    const auto value = json.find(AttacksKey);
    if (value == json.end())
    {
        return true;
    }
    if (!value->is_object())
    {
        issues.Fatal(field, "must be an object", number);
        return false;
    }
    MonsterAttacks read;
    bool valid = ReadAttackEntries(*value, MonsterSkillsKey, field, number, read.monsterSkills, issues);
    valid = ReadAttackEntries(*value, SkillsKey, field, number, read.skills, issues) && valid;
    const auto otherwise = value->find(OtherwiseKey);
    const auto random = value->find(OtherwiseRandomKey);
    if (otherwise != value->end() && random != value->end())
    {
        issues.Fatal(field, "has otherwise or otherwiseRandom, not both", number);
        valid = false;
    }
    if (otherwise != value->end() && !ReadOtherwise(*otherwise, read))
    {
        issues.Fatal(field + "." + OtherwiseKey,
                     "must be an animation (attack1, shock, appear, ...), none, generic or a list of two or more "
                     "animations played in turn",
                     number);
        valid = false;
    }
    if (random != value->end())
    {
        read.otherwiseRandom = random->is_string() ? FindMonsterAttackRandom(random->get<std::string>()) : std::nullopt;
        if (!read.otherwiseRandom)
        {
            issues.Fatal(field + "." + OtherwiseRandomKey, "must be rand, randFps or bali", number);
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
            issues.Fatal(field + "." + ClearSkillKey, "must be never, onMatch, onNoMatch, always or afterAttack",
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
            issues.Fatal(field + "." + SoundKey, "must be a SOUND_ id that the monster data can name", number);
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
            issues.Fatal(field + "." + SoundAtMonsterKey, "must be true or false", number);
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

// The maps of a map variant: one or more server maps or stages (D67).
bool ReadVariantMaps(const OrderedJson& variant, const std::string& field, int number,
                     std::vector<Data::Maps::MapRef>& maps, IssueList& issues)
{
    const std::string mapsField = field + "." + MapsKey;
    const auto value = variant.find(MapsKey);
    if (value == variant.end() || (value->is_array() && value->empty()))
    {
        issues.Fatal(mapsField, "must be a list of one or more maps", number);
        return false;
    }
    return Data::Maps::ReadMapRefs(*value, mapsField, maps,
                                   [&](const std::string& problemField, const std::string& message)
                                   { issues.Fatal(problemField, message, number); });
}

// "map 9", "map 9 stage 2".
std::string DescribeMapRef(const Data::Maps::MapRef& ref)
{
    return "map " + std::to_string(ref.map) + (ref.stage != 0 ? " stage " + std::to_string(ref.stage) : "");
}

// What the monster does differently on some maps (D62); a map in two
// variants contradicts itself.
bool ReadMapVariants(const OrderedJson& json, int number, std::vector<MonsterMapVariant>& variants, IssueList& issues)
{
    const auto value = json.find(MapVariantsKey);
    if (value == json.end())
    {
        return true;
    }
    const std::string field = MonsterField(number, MapVariantsKey);
    if (!value->is_array())
    {
        issues.Fatal(field, "must be a list of map variants", number);
        return false;
    }
    bool valid = true;
    std::vector<Data::Maps::MapRef> named;
    for (size_t index = 0; index < value->size(); ++index)
    {
        const OrderedJson& entry = (*value)[index];
        const std::string key = field + "[" + std::to_string(index) + "]";
        if (!entry.is_object())
        {
            issues.Fatal(key, "must be an object with maps and the fields it changes", number);
            valid = false;
            continue;
        }
        MonsterMapVariant variant;
        bool entryValid = ReadVariantMaps(entry, key, number, variant.maps, issues);
        for (const Data::Maps::MapRef& ref : variant.maps)
        {
            const auto earlier = std::find_if(named.begin(), named.end(), [&](const Data::Maps::MapRef& other)
                                              { return Data::Maps::Overlaps(ref, other); });
            if (earlier != named.end())
            {
                issues.Fatal(key + "." + MapsKey,
                             "names " + DescribeMapRef(ref) + ", which an earlier map variant names (" +
                                 DescribeMapRef(*earlier) + ")",
                             number);
                entryValid = false;
            }
        }
        named.insert(named.end(), variant.maps.begin(), variant.maps.end());
        if (const auto model = entry.find(ModelKey); model != entry.end())
        {
            if (!model->is_string() || !FindMonsterModelReference(model->get<std::string>()))
            {
                issues.Fatal(key + "." + ModelKey, "must be a model, as the monster's model names one", number);
                entryValid = false;
            }
            else
            {
                variant.model = model->get<std::string>();
            }
        }
        entryValid = ReadAttacks(entry, key + "." + AttacksKey, number, variant.attacks, issues) && entryValid;
        // A variant changes fields; the entry's stay where it names none.
        for (const MonsterSetupField& setupField : GetMonsterSetupFields())
        {
            const auto setupValue = entry.find(setupField.key);
            if (setupValue != entry.end() && setupValue->is_null())
            {
                issues.Fatal(key + "." + std::string(setupField.key),
                             "must be a value: a map variant changes a field, it does not clear it", number);
                entryValid = false;
            }
        }
        // Read always, so its problems show with those of the maps and attacks.
        entryValid = ReadMonsterSetup(entry, key + ".", variant.setup, issues.ForMonster(number)) && entryValid;
        const bool changesSetup = std::ranges::any_of(GetMonsterSetupFields(), [&](const MonsterSetupField& setupField)
                                                      { return setupField.isSet(variant.setup); });
        if (entryValid && !variant.model && !variant.attacks && !changesSetup)
        {
            issues.Warning(key, "changes nothing: it has no model, attacks or setup field", number);
        }
        Items::Json::KeySet knownKeys{MapsKey, ModelKey, AttacksKey};
        AddMonsterSetupKeys(knownKeys);
        Items::Json::WarnAboutUnknownKeys(entry, knownKeys, key + " ",
                                          [&](const std::string& unknown, const std::string& message)
                                          { issues.Warning(unknown, message, number); });
        if (entryValid)
        {
            variants.push_back(std::move(variant));
        }
        valid = entryValid && valid;
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
        json[OtherwiseKey] = WriteOtherwise(attacks);
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

// The values a monster's numbers share; `number`, its first number, names
// them in problems.
bool ReadSharedValues(const OrderedJson& json, int number, MonsterDefinition& shared, IssueList& issues)
{
    shared.number = number;
    bool valid = ReadNames(json, number, shared.names, issues);
    valid = ReadKind(json, number, shared.kind, issues) && valid;
    valid = ReadModel(json, number, shared.model, issues) && valid;
    valid = ReadAttacks(json, MonsterField(number, AttacksKey), number, shared.attacks, issues) && valid;
    valid = ReadMapVariants(json, number, shared.mapVariants, issues) && valid;
    valid = ReadSoundSet(json, MonsterField(number, SoundSetKey), number, shared.soundSet, issues) && valid;
    valid = ReadRandomVariants(json, shared, shared.randomVariants, issues) && valid;
    valid =
        ReadMonsterSetup(json, "monster " + std::to_string(number) + " ", shared.setup, issues.ForMonster(number)) &&
        valid;
    return valid;
}

// Whether the object has `key` as an empty text: a number's own "no model"
// or "no sound set".
bool HasEmptyText(const OrderedJson& json, const char* key)
{
    const auto value = json.find(key);
    return value != json.end() && value->is_string() && value->get_ref<const std::string&>().empty();
}

// The names and the kind of a number that names them, in place of the
// monster's.
bool ReadOwnNamesAndKind(const OrderedJson& json, MonsterDefinition& entry, IssueList& issues)
{
    const int number = entry.number;
    bool valid = true;
    if (json.contains(NameKey))
    {
        LocalizedString names;
        valid = ReadNames(json, number, names, issues);
        entry.names = std::move(names);
    }
    if (json.contains(KindKey))
    {
        valid = ReadKind(json, number, entry.kind, issues) && valid;
    }
    return valid;
}

// The model, sound set and attacks of a number that names them, which may be
// none where the monster has them: "model": "", "soundSet": "", "attacks":
// null.
bool ReadOwnModelSetAndAttacks(const OrderedJson& json, MonsterDefinition& entry, IssueList& issues)
{
    const int number = entry.number;
    bool valid = true;
    if (HasEmptyText(json, ModelKey))
    {
        entry.model.clear();
    }
    else
    {
        valid = ReadModel(json, number, entry.model, issues);
    }
    if (HasEmptyText(json, SoundSetKey))
    {
        entry.soundSet.clear();
    }
    else
    {
        valid = ReadSoundSet(json, MonsterField(number, SoundSetKey), number, entry.soundSet, issues) && valid;
    }
    if (const auto attacks = json.find(AttacksKey); attacks != json.end() && attacks->is_null())
    {
        entry.attacks.reset();
    }
    else
    {
        valid = ReadAttacks(json, MonsterField(number, AttacksKey), number, entry.attacks, issues) && valid;
    }
    return valid;
}

// The lists of a number that names them, in place of the monster's, also
// empty: its map variants and random variants, and the maps it appears on.
bool ReadOwnLists(const OrderedJson& json, MonsterDefinition& entry, IssueList& issues)
{
    const int number = entry.number;
    bool valid = true;
    if (json.contains(MapVariantsKey))
    {
        entry.mapVariants.clear();
        valid = ReadMapVariants(json, number, entry.mapVariants, issues);
    }
    if (json.contains(RandomVariantsKey))
    {
        entry.randomVariants.clear();
        valid = ReadRandomVariants(json, entry, entry.randomVariants, issues) && valid;
    }
    if (const auto maps = json.find(MapsKey); maps != json.end())
    {
        entry.maps.clear();
        valid = Data::Maps::ReadMapRefs(*maps, MonsterField(number, MapsKey), entry.maps,
                                        [&](const std::string& field, const std::string& message)
                                        { issues.Fatal(field, message, number); }) &&
                valid;
    }
    return valid;
}

// An own model may not be the model of a look the number shares.
bool CheckModelIsNoLook(const MonsterDefinition& entry, IssueList& issues)
{
    const bool modelIsALook = std::any_of(entry.randomVariants.begin(), entry.randomVariants.end(),
                                          [&](const MonsterRandomVariant& look) { return look.model == entry.model; });
    if (!modelIsALook)
    {
        return true;
    }
    issues.Fatal(MonsterField(entry.number, RandomVariantsKey), entry.model + " is the entry's model and a variant's",
                 entry.number);
    return false;
}

// The values a number of a monster has as its own, read onto `entry`, which
// holds the shared ones: each field the object has replaces the entry's. They
// may be empty where a monster's are not: "model": "", "soundSet": "",
// "attacks": null, and empty lists of variants.
bool ReadOwnValues(const OrderedJson& json, MonsterDefinition& entry, IssueList& issues)
{
    // Each field WriteOwnValue writes is read here; a new one needs both.
    static_assert(MonsterValueFields.size() == 7, "read the new own value here and write it in WriteOwnValue");
    bool valid = ReadOwnNamesAndKind(json, entry, issues);
    valid = ReadOwnModelSetAndAttacks(json, entry, issues) && valid;
    valid = ReadOwnLists(json, entry, issues) && valid;
    // The setup fields too, each a value of its own (WriteMonsterSetupDifferences).
    valid = ReadMonsterSetup(json, "monster " + std::to_string(entry.number) + " ", entry.setup,
                             issues.ForMonster(entry.number)) &&
            valid;
    return CheckModelIsNoLook(entry, issues) && valid;
}

void WarnAboutUnknownMonsterKeys(const OrderedJson& json, const Items::Json::KeySet& known, int number,
                                 IssueList& issues)
{
    Items::Json::WarnAboutUnknownKeys(json, known, "monster " + std::to_string(number) + " ",
                                      [&](const std::string& unknown, const std::string& message)
                                      { issues.Warning(unknown, message, number); });
}

// An entry of a monster's "numbers" and the number it names.
struct NumberJson
{
    const OrderedJson* json = nullptr;
    int number = 0;
};

// The entries of a monster's "numbers" that name a number; the others are
// problems.
std::vector<NumberJson> ReadNumberList(const OrderedJson& list, const std::string& field, IssueList& issues)
{
    std::vector<NumberJson> numbers;
    for (size_t index = 0; index < list.size(); ++index)
    {
        const OrderedJson& entry = list[index];
        const std::string key = field + "." + NumbersKey + "[" + std::to_string(index) + "]";
        if (!entry.is_object())
        {
            issues.Fatal(key, "must be an object with the number and its own values");
            continue;
        }
        if (const std::optional<int> number = ReadMonsterNumber(entry, key, issues))
        {
            numbers.push_back({&entry, *number});
        }
    }
    return numbers;
}

// The entry of each number: the shared values with the number's own. A
// number with an error in its own values is left out.
std::vector<MonsterDefinition> ReadNumberEntries(const std::vector<NumberJson>& numbers,
                                                 const MonsterDefinition& shared, IssueList& issues)
{
    std::vector<MonsterDefinition> entries;
    for (const NumberJson& number : numbers)
    {
        MonsterDefinition entry = shared;
        entry.number = number.number;
        const bool valid = ReadOwnValues(*number.json, entry, issues);
        Items::Json::KeySet ownKeys{NumberKey,   MapsKey,    NameKey,        KindKey,          ModelKey,
                                    SoundSetKey, AttacksKey, MapVariantsKey, RandomVariantsKey};
        AddMonsterSetupKeys(ownKeys);
        WarnAboutUnknownMonsterKeys(*number.json, ownKeys, number.number, issues);
        if (valid)
        {
            entries.push_back(std::move(entry));
        }
    }
    return entries;
}

// A monster of the list: its shared values and its numbers with their own.
std::optional<MonsterGroup> ReadMonster(const OrderedJson& json, const std::string& field, IssueList& issues)
{
    if (!json.is_object())
    {
        issues.Fatal(field, "must be an object");
        return std::nullopt;
    }
    const auto list = json.find(NumbersKey);
    if (list == json.end() || !list->is_array() || list->empty())
    {
        issues.Fatal(field + "." + NumbersKey, "missing or not a list of one or more numbers");
        return std::nullopt;
    }
    const std::vector<NumberJson> numbers = ReadNumberList(*list, field, issues);
    if (numbers.empty())
    {
        return std::nullopt;
    }
    MonsterGroup monster;
    const int first = numbers.front().number;
    const bool valid = ReadSharedValues(json, first, monster.shared, issues);
    Items::Json::KeySet sharedKeys{NameKey,    KindKey,        ModelKey,          SoundSetKey,
                                   AttacksKey, MapVariantsKey, RandomVariantsKey, NumbersKey};
    AddMonsterSetupKeys(sharedKeys);
    WarnAboutUnknownMonsterKeys(json, sharedKeys, first, issues);
    if (!valid)
    {
        return std::nullopt;
    }
    monster.numbers = ReadNumberEntries(numbers, monster.shared, issues);
    if (monster.numbers.empty())
    {
        return std::nullopt;
    }
    return monster;
}

// An entry of version 1: one number with its values, a monster of its own.
std::optional<MonsterGroup> ReadOneNumberEntry(const OrderedJson& json, const std::string& field, IssueList& issues)
{
    if (!json.is_object())
    {
        issues.Fatal(field, "must be an object");
        return std::nullopt;
    }
    const std::optional<int> number = ReadMonsterNumber(json, field, issues);
    if (!number)
    {
        return std::nullopt;
    }
    MonsterDefinition entry;
    const bool valid = ReadSharedValues(json, *number, entry, issues);
    Items::Json::KeySet keys{NumberKey,   NameKey,    KindKey,        ModelKey,
                             SoundSetKey, AttacksKey, MapVariantsKey, RandomVariantsKey};
    AddMonsterSetupKeys(keys);
    WarnAboutUnknownMonsterKeys(json, keys, *number, issues);
    if (!valid)
    {
        return std::nullopt;
    }
    return MonsterGroup{entry, {entry}};
}

OrderedJson WriteMapVariants(const std::vector<MonsterMapVariant>& variants)
{
    OrderedJson list = OrderedJson::array();
    for (const MonsterMapVariant& variant : variants)
    {
        OrderedJson entry = OrderedJson::object();
        entry[MapsKey] = Data::Maps::WriteMapRefs(variant.maps);
        if (variant.model)
        {
            entry[ModelKey] = *variant.model;
        }
        if (variant.attacks)
        {
            entry[AttacksKey] = WriteAttacks(*variant.attacks);
        }
        WriteMonsterSetup(entry, variant.setup);
        list.push_back(std::move(entry));
    }
    return list;
}

OrderedJson WriteRandomVariants(const std::vector<MonsterRandomVariant>& variants)
{
    OrderedJson list = OrderedJson::array();
    for (const MonsterRandomVariant& variant : variants)
    {
        OrderedJson entry = OrderedJson::object();
        entry[ModelKey] = variant.model;
        if (variant.soundSet)
        {
            entry[SoundSetKey] = *variant.soundSet;
        }
        if (variant.equipment)
        {
            entry[EquipmentKey] = WriteMonsterEquipment(*variant.equipment);
        }
        list.push_back(std::move(entry));
    }
    return list;
}

OrderedJson WriteSharedValues(const MonsterDefinition& shared)
{
    OrderedJson json = OrderedJson::object();
    json[NameKey] = Items::Json::WriteLocalizedString(shared.names);
    json[KindKey] = GetMonsterKindName(shared.kind);
    if (!shared.model.empty())
    {
        json[ModelKey] = shared.model;
    }
    if (!shared.soundSet.empty())
    {
        json[SoundSetKey] = shared.soundSet;
    }
    if (shared.attacks)
    {
        json[AttacksKey] = WriteAttacks(*shared.attacks);
    }
    if (!shared.mapVariants.empty())
    {
        json[MapVariantsKey] = WriteMapVariants(shared.mapVariants);
    }
    if (!shared.randomVariants.empty())
    {
        json[RandomVariantsKey] = WriteRandomVariants(shared.randomVariants);
    }
    WriteMonsterSetup(json, shared.setup);
    return json;
}

// A number's own value of the field, as WriteNumber writes it: also empty
// where the monster's is not (ReadOwnValues reads it back).
void WriteOwnValue(OrderedJson& json, const MonsterDefinition& entry, MonsterValueField field)
{
    switch (field)
    {
    case MonsterValueField::Names:
        json[NameKey] = Items::Json::WriteLocalizedString(entry.names);
        break;
    case MonsterValueField::Kind:
        json[KindKey] = GetMonsterKindName(entry.kind);
        break;
    case MonsterValueField::Model:
        json[ModelKey] = entry.model;
        break;
    case MonsterValueField::SoundSet:
        json[SoundSetKey] = entry.soundSet;
        break;
    case MonsterValueField::Attacks:
        json[AttacksKey] = entry.attacks ? WriteAttacks(*entry.attacks) : OrderedJson(nullptr);
        break;
    case MonsterValueField::MapVariants:
        json[MapVariantsKey] = WriteMapVariants(entry.mapVariants);
        break;
    case MonsterValueField::RandomVariants:
        json[RandomVariantsKey] = WriteRandomVariants(entry.randomVariants);
        break;
    case MonsterValueField::SetupFirst:
        // The setup fields are written by their own table.
        break;
    }
}

// A number with its maps and the values its entry has differently from the
// shared ones, in the order of MonsterValueFields.
OrderedJson WriteNumber(const MonsterDefinition& shared, const MonsterDefinition& entry)
{
    OrderedJson json = OrderedJson::object();
    json[NumberKey] = entry.number;
    if (!entry.maps.empty())
    {
        json[MapsKey] = Data::Maps::WriteMapRefs(entry.maps);
    }
    for (const MonsterValueField field : MonsterValueFields)
    {
        if (!HasSameValue(entry, shared, field))
        {
            WriteOwnValue(json, entry, field);
        }
    }
    WriteMonsterSetupDifferences(json, shared.setup, entry.setup);
    return json;
}

// The monster's numbers, sorted.
std::vector<const MonsterDefinition*> SortedNumbers(const MonsterGroup& monster)
{
    std::vector<const MonsterDefinition*> numbers;
    numbers.reserve(monster.numbers.size());
    for (const MonsterDefinition& entry : monster.numbers)
    {
        numbers.push_back(&entry);
    }
    std::sort(numbers.begin(), numbers.end(), [](const MonsterDefinition* left, const MonsterDefinition* right)
              { return left->number < right->number; });
    return numbers;
}

OrderedJson WriteMonster(const MonsterGroup& monster)
{
    const MonsterDefinition& shared = GetSharedValues(monster);
    OrderedJson json = WriteSharedValues(shared);
    OrderedJson numbers = OrderedJson::array();
    for (const MonsterDefinition* entry : SortedNumbers(monster))
    {
        numbers.push_back(WriteNumber(shared, *entry));
    }
    json[NumbersKey] = std::move(numbers);
    return json;
}

int FirstNumber(const MonsterGroup& monster)
{
    int first = MonsterNumberCount;
    for (const MonsterDefinition& entry : monster.numbers)
    {
        first = std::min(first, entry.number);
    }
    return first;
}
} // namespace

namespace
{
// The unknown monsters' look (D64): each value the section has replaces the
// default; a value of the wrong kind is fatal, a model the list lacks an error.
void ReadUnknownLook(const OrderedJson& root, UnknownMonsterLook& unknown, IssueList& list)
{
    const auto section = root.find(UnknownKey);
    if (section == root.end())
        return;
    if (!section->is_object())
    {
        list.Fatal(UnknownKey, "must be an object: model, scale, glow, marks, messageSeconds, for");
        return;
    }
    const auto field = [](const char* key) { return std::string(UnknownKey) + "." + key; };
    if (const auto model = section->find(UnknownModelKey); model != section->end())
    {
        if (!model->is_string())
            list.Fatal(field(UnknownModelKey), "must be a model, as a monster's model names one");
        else if (!FindMonsterModelReference(model->get<std::string>()))
            list.Error(field(UnknownModelKey), model->get<std::string>() + " is no model: the default stays");
        else
            unknown.model = model->get<std::string>();
    }
    if (const auto scale = section->find(UnknownScaleKey); scale != section->end())
    {
        if (!scale->is_number() || scale->get<float>() <= 0.0f)
            list.Fatal(field(UnknownScaleKey), "must be a number above 0");
        else
            unknown.scale = scale->get<float>();
    }
    if (const auto glow = section->find(UnknownGlowKey); glow != section->end())
    {
        if (!glow->is_array() || glow->size() != 3 ||
            std::ranges::any_of(*glow, [](const OrderedJson& part) { return !part.is_number(); }))
            list.Fatal(field(UnknownGlowKey), "must be three numbers: [red, green, blue]");
        else
            unknown.glow = {(*glow)[0].get<float>(), (*glow)[1].get<float>(), (*glow)[2].get<float>()};
    }
    if (const auto marks = section->find(UnknownMarksKey); marks != section->end())
    {
        if (!marks->is_boolean())
            list.Fatal(field(UnknownMarksKey), "must be true or false");
        else
            unknown.marks = marks->get<bool>();
    }
    if (const auto seconds = section->find(UnknownMessageKey); seconds != section->end())
    {
        if (!seconds->is_number_integer() || seconds->get<int>() < 0 || seconds->get<int>() > MaxUnknownMessageSeconds)
            list.Fatal(field(UnknownMessageKey), "must be whole seconds from 0 to " +
                                                     std::to_string(MaxUnknownMessageSeconds) +
                                                     " (0: only when it appears)");
        else
            unknown.messageSeconds = seconds->get<int>();
    }
    if (const auto who = section->find(UnknownForKey); who != section->end())
    {
        if (!who->is_array() || std::ranges::any_of(*who, [](const OrderedJson& name) { return !name.is_string(); }))
        {
            list.Fatal(field(UnknownForKey), "must be a list of noEntry, noModel and missingFile");
        }
        else
        {
            unknown.forNoEntry = false;
            unknown.forNoModel = false;
            unknown.forMissingFile = false;
            for (const OrderedJson& name : *who)
            {
                const std::string value = name.get<std::string>();
                if (value == ForNoEntry)
                    unknown.forNoEntry = true;
                else if (value == ForNoModel)
                    unknown.forNoModel = true;
                else if (value == ForMissingFile)
                    unknown.forMissingFile = true;
                else
                    list.Error(field(UnknownForKey), value + " is not noEntry, noModel or missingFile");
            }
        }
    }
    Items::Json::WarnAboutUnknownKeys(
        *section, {UnknownModelKey, UnknownScaleKey, UnknownGlowKey, UnknownMarksKey, UnknownMessageKey, UnknownForKey},
        std::string(UnknownKey) + " ",
        [&](const std::string& unknownKey, const std::string& message) { list.Warning(unknownKey, message); });
}

// The shared talk texts: each set an object of an altar contract's six texts.
void ReadTalkTexts(const OrderedJson& root, MonsterTalkTexts& talkTexts, IssueList& list)
{
    talkTexts.clear();
    const auto section = root.find(TalkTextsKey);
    if (section == root.end())
        return;
    if (!section->is_object())
    {
        list.Fatal(TalkTextsKey, "must be an object of talk text sets by name");
        return;
    }
    const std::span<const char* const> keys = GetAltarContractTextKeys();
    for (const auto& [name, set] : section->items())
    {
        const std::string setKey = std::string(TalkTextsKey) + "." + name;
        if (!set.is_object())
        {
            list.Fatal(setKey, "must be an object of its texts: canContract, proceed, onMount, needGuardian, "
                               "destroyed, ongoing");
            // Kept empty, so the steps naming it add no problem of their own.
            talkTexts.emplace(name, MonsterAltarContract{});
            continue;
        }
        // A set with a wrong text is kept, so its problem is the text's only
        // and not also every step that names it.
        MonsterAltarContract contract;
        const auto texts = AltarContractTexts(contract);
        for (size_t index = 0; index < keys.size(); ++index)
        {
            const std::string textKey = setKey + "." + keys[index];
            const auto text = set.find(keys[index]);
            if (text == set.end())
            {
                list.Fatal(textKey, "missing");
                continue;
            }
            Items::Json::ReadLocalizedString(*text, textKey, *texts[index],
                                             [&](const std::string& field, const std::string& message)
                                             { list.Fatal(field, message); });
        }
        Items::Json::KeySet known(keys.begin(), keys.end());
        Items::Json::WarnAboutUnknownKeys(set, known, setKey + " ",
                                          [&](const std::string& unknownKey, const std::string& message)
                                          { list.Warning(unknownKey, message); });
        talkTexts.emplace(name, std::move(contract));
    }
}

OrderedJson WriteTalkTexts(const MonsterTalkTexts& talkTexts)
{
    OrderedJson section = OrderedJson::object();
    const std::span<const char* const> keys = GetAltarContractTextKeys();
    for (const auto& [name, contract] : talkTexts)
    {
        OrderedJson set = OrderedJson::object();
        const auto texts = AltarContractTexts(contract);
        for (size_t index = 0; index < keys.size(); ++index)
            set[keys[index]] = Items::Json::WriteLocalizedString(*texts[index]);
        section[name] = std::move(set);
    }
    return section;
}

// An altar contract step names a set the talk texts have; a fatal problem
// once per field.
void CheckTalkTextNames(std::span<const MonsterGroup> monsters, const MonsterTalkTexts& talkTexts, IssueList& list)
{
    std::set<std::string> reported;
    const auto check = [&](const MonsterSetup& setup, int number, const std::string& where)
    {
        for (size_t index = 0; index < setup.onTalk.size(); ++index)
        {
            const MonsterTalkBlock& block = setup.onTalk[index];
            if (block.kind != MonsterTalkBlock::Kind::CrywolfAltarContract || talkTexts.contains(block.altarTexts))
                continue;
            const std::string field =
                MonsterField(number, (where + "onTalk[" + std::to_string(index) + "].crywolfAltarContract").c_str());
            if (reported.insert(field).second)
                list.Fatal(field, "names " + block.altarTexts + ", which talkTexts does not have", number);
        }
    };
    for (const MonsterDefinition& entry : CollectNumberEntries(monsters))
    {
        const auto withStates = [&](const MonsterSetup& setup, const std::string& where)
        {
            check(setup, entry.number, where);
            for (size_t index = 0; index < setup.stateVariants.size(); ++index)
                check(setup.stateVariants[index].setup, entry.number,
                      where + "stateVariants[" + std::to_string(index) + "].");
        };
        withStates(entry.setup, "");
        for (size_t index = 0; index < entry.mapVariants.size(); ++index)
            withStates(entry.mapVariants[index].setup, "mapVariants[" + std::to_string(index) + "].");
    }
}

OrderedJson WriteUnknownLook(const UnknownMonsterLook& unknown)
{
    OrderedJson section = OrderedJson::object();
    section[UnknownModelKey] = unknown.model;
    section[UnknownScaleKey] = Json::WriteFloat(unknown.scale);
    section[UnknownGlowKey] = OrderedJson::array(
        {Json::WriteFloat(unknown.glow[0]), Json::WriteFloat(unknown.glow[1]), Json::WriteFloat(unknown.glow[2])});
    section[UnknownMarksKey] = unknown.marks;
    section[UnknownMessageKey] = unknown.messageSeconds;
    OrderedJson who = OrderedJson::array();
    if (unknown.forNoEntry)
        who.push_back(ForNoEntry);
    if (unknown.forNoModel)
        who.push_back(ForNoModel);
    if (unknown.forMissingFile)
        who.push_back(ForMissingFile);
    section[UnknownForKey] = std::move(who);
    return section;
}
} // namespace

void ReadMonstersJson(std::string_view text, const std::string& source, std::vector<MonsterGroup>& monsters,
                      std::vector<ItemDataIssue>& issues)
{
    UnknownMonsterLook unknown;
    ReadMonstersJson(text, source, monsters, unknown, issues);
}

void ReadMonstersJson(std::string_view text, const std::string& source, std::vector<MonsterGroup>& monsters,
                      UnknownMonsterLook& unknown, std::vector<ItemDataIssue>& issues)
{
    MonsterTalkTexts talkTexts;
    ReadMonstersJson(text, source, monsters, unknown, talkTexts, issues);
}

void ReadMonstersJson(std::string_view text, const std::string& source, std::vector<MonsterGroup>& monsters,
                      UnknownMonsterLook& unknown, MonsterTalkTexts& talkTexts, std::vector<ItemDataIssue>& issues)
{
    IssueList list(source, issues);
    OrderedJson root;
    if (!Items::Json::ReadFileVersion(text, source, MonstersFormatVersion, root, issues))
    {
        return;
    }
    Items::Json::WarnAboutUnknownKeys(root, {Items::Json::Keys::FormatVersion, UnknownKey, TalkTextsKey, MonstersKey},
                                      "", [&](const std::string& unknownKey, const std::string& message)
                                      { list.Warning(unknownKey, message); });
    ReadUnknownLook(root, unknown, list);
    ReadTalkTexts(root, talkTexts, list);
    const auto entries = root.find(MonstersKey);
    if (entries == root.end() || !entries->is_array())
    {
        list.Fatal(MonstersKey, "missing or not a list of monsters");
        return;
    }
    const bool oneNumberEntries = root[Items::Json::Keys::FormatVersion].get<int>() == OneNumberEntriesFormatVersion;
    std::set<int> numbers;
    for (size_t index = 0; index < entries->size(); ++index)
    {
        const OrderedJson& entry = (*entries)[index];
        std::optional<MonsterGroup> monster = oneNumberEntries ? ReadOneNumberEntry(entry, EntryField(index), list)
                                                               : ReadMonster(entry, EntryField(index), list);
        if (!monster)
        {
            continue;
        }
        std::erase_if(monster->numbers,
                      [&](const MonsterDefinition& number)
                      {
                          if (numbers.insert(number.number).second)
                          {
                              return false;
                          }
                          list.Fatal(MonsterField(number.number, NumberKey), "is in the list more than once",
                                     number.number);
                          return true;
                      });
        if (!monster->numbers.empty())
        {
            monsters.push_back(std::move(*monster));
        }
    }
    CheckTalkTextNames(monsters, talkTexts, list);
}

void ReadMonstersJson(std::string_view text, const std::string& source, std::vector<MonsterDefinition>& numbers,
                      std::vector<ItemDataIssue>& issues)
{
    std::vector<MonsterGroup> monsters;
    ReadMonstersJson(text, source, monsters, issues);
    const std::vector<MonsterDefinition> entries = CollectNumberEntries(monsters);
    numbers.insert(numbers.end(), entries.begin(), entries.end());
}

namespace
{
std::string WriteFile(std::span<const MonsterGroup> monsters, const UnknownMonsterLook* unknown,
                      const MonsterTalkTexts* talkTexts)
{
    std::vector<const MonsterGroup*> sorted;
    sorted.reserve(monsters.size());
    for (const MonsterGroup& monster : monsters)
    {
        if (!monster.numbers.empty())
        {
            sorted.push_back(&monster);
        }
    }
    std::stable_sort(sorted.begin(), sorted.end(), [](const MonsterGroup* left, const MonsterGroup* right)
                     { return FirstNumber(*left) < FirstNumber(*right); });

    OrderedJson entries = OrderedJson::array();
    for (const MonsterGroup* monster : sorted)
    {
        entries.push_back(WriteMonster(*monster));
    }
    OrderedJson root = OrderedJson::object();
    root[Items::Json::Keys::FormatVersion] = MonstersFormatVersion;
    if (unknown != nullptr)
        root[UnknownKey] = WriteUnknownLook(*unknown);
    if (talkTexts != nullptr && !talkTexts->empty())
        root[TalkTextsKey] = WriteTalkTexts(*talkTexts);
    root[MonstersKey] = std::move(entries);
    const std::string text = root.dump(Items::Json::Indent, ' ', false, OrderedJson::error_handler_t::replace);
    std::string compact = Items::Json::PutObjectsOnOneLine(text, NameKey);
    compact = Items::Json::PutObjectsOnOneLine(compact, MonsterSkillsKey);
    compact = Items::Json::PutObjectsOnOneLine(compact, SkillsKey);
    compact = Items::Json::PutListsOnOneLine(compact, OtherwiseKey);
    compact = Items::Json::PutListsOnOneLine(compact, MapsKey);
    compact = Items::Json::PutListsOnOneLine(compact, UnknownGlowKey);
    compact = Items::Json::PutListsOnOneLine(compact, UnknownForKey);
    compact = Items::Json::PutListEntriesOnOneLine(compact, RandomVariantsKey);
    compact = Items::Json::PutListEntriesOnOneLine(compact, NumbersKey);
    return compact + "\n";
}
} // namespace

std::string WriteMonstersJson(std::span<const MonsterGroup> monsters)
{
    return WriteFile(monsters, nullptr, nullptr);
}

std::string WriteMonstersJson(std::span<const MonsterGroup> monsters, const UnknownMonsterLook& unknown,
                              const MonsterTalkTexts& talkTexts)
{
    return WriteFile(monsters, &unknown, &talkTexts);
}

std::string WriteMonstersJson(std::span<const MonsterDefinition> numbers)
{
    const std::vector<MonsterGroup> monsters = MakeOneNumberGroups(numbers);
    return WriteMonstersJson(std::span<const MonsterGroup>(monsters));
}
} // namespace Data::Monsters
