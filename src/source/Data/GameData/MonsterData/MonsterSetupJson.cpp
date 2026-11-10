#include "stdafx.h"

#include "MonsterSetupJson.h"

#include "Core/Globals/_define.h"
#include "Data/GameData/EffectData/EffectTypeSymbols.h"
#include "Data/GameData/ItemData/ItemJsonFormat.h"
#include "Data/GameData/MonsterData/MonsterAnimations.h"
#include "Data/GameData/MonsterData/MonsterJsonCommon.h"
#include "Data/GameData/MonsterData/MonsterModelSlots.h"
#include "Data/GameData/MonsterData/MonsterSoundIds.h"

#include <algorithm>
#include <limits>
#include <optional>
#include <string_view>

namespace Data::Monsters
{
namespace
{
using Items::ItemDataIssueSeverity;
using Items::Json::OrderedJson;

constexpr const char* CharacterScale = "character";
constexpr const char* WeaponsKey = "weapons";
constexpr const char* JointKey = "joint";
constexpr const char* ObjectKey = "object";
constexpr const char* EffectKey = "effect";
constexpr const char* EyesKey = "eyes";
constexpr const char* ParticleKey = "particle";
constexpr const char* BoneKey = "bone";
constexpr const char* CountKey = "count";
constexpr const char* OffsetKey = "offset";
constexpr const char* TargetOffsetKey = "targetOffset";
constexpr const char* ScaleRandomKey = "scaleRandom";
constexpr const char* ActionKey = "action";
constexpr const char* BlendKey = "blend";
constexpr const char* StartFrameKey = "startFrame";
constexpr const char* FadeInKey = "fadeIn";
constexpr const char* AppearTimeKey = "appearTime";
constexpr const char* SoundKey = "sound";
constexpr const char* EffectsKey = "effects";
constexpr const char* GateClosedKey = "closed";
constexpr const char* GateOpeningKey = "opening";
constexpr const char* GateOpenKey = "open";
constexpr const char* GateArrivalKey = "arrival";
constexpr const char* ColorKey = "color";
constexpr const char* KeyKey = "key";
constexpr const char* StateKey = "state";
constexpr const char* HookKey = "hook";
constexpr const char* AngleKey = "angle";
constexpr const char* TileXKey = "tileX";
constexpr const char* TileYKey = "tileY";
constexpr const char* FacingStepKey = "facingStep";
constexpr const char* SubTypeKey = "subType";
constexpr const char* ScaleKey = "scale";
constexpr const char* GroupKey = "group";
constexpr const char* NumberKey = "number";
constexpr const char* LevelKey = "level";
constexpr const char* ExcellentKey = "excellent";
constexpr const char* PartKey = "part";
constexpr const char* MountLookKey = "mountLook";

struct Reader
{
    const std::string& prefix;
    const SetupProblem& problem;
    bool valid = true;

    void Fatal(const std::string& key, const std::string& message)
    {
        problem(ItemDataIssueSeverity::Fatal, prefix + key, message);
        valid = false;
    }
    void Error(const std::string& key, const std::string& message)
    {
        problem(ItemDataIssueSeverity::Error, prefix + key, message);
    }
    void Warning(const std::string& key, const std::string& message)
    {
        problem(ItemDataIssueSeverity::Warning, prefix + key, message);
    }

    void Float(const OrderedJson& value, const std::string& key, std::optional<float>& field)
    {
        if (!value.is_number())
        {
            Fatal(key, "must be a number");
            return;
        }
        field = value.get<float>();
    }
    void Whole(const OrderedJson& value, const std::string& key, std::optional<int>& field)
    {
        const std::optional<int> number =
            Json::ReadWholeIn(value, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
        if (!number)
        {
            Fatal(key, "must be a whole number");
            return;
        }
        field = number;
    }
    void WholeIn(const OrderedJson& value, const std::string& key, int low, int high, std::optional<int>& field)
    {
        const std::optional<int> number = Json::ReadWholeIn(value, low, high);
        if (!number)
        {
            Fatal(key, Json::RangeText(low, high));
            return;
        }
        field = number;
    }
    void Bool(const OrderedJson& value, const std::string& key, std::optional<bool>& field)
    {
        if (!value.is_boolean())
        {
            Fatal(key, "must be true or false");
            return;
        }
        field = value.get<bool>();
    }

    std::optional<MonsterItem> Item(const OrderedJson& value, const std::string& key)
    {
        // A body that is no item, by its slot.
        if (value.is_object() && value.contains(PartKey))
        {
            const OrderedJson& part = *value.find(PartKey);
            if (!part.is_string())
            {
                Fatal(key + "." + PartKey, "must be a slot as the code names it");
                return std::nullopt;
            }
            MonsterItem item;
            item.part = part.get<std::string>();
            if (!FindMonsterBodyPartSlot(item.part))
                Error(key + "." + PartKey, item.part + " is not a part model or player body slot");
            Items::Json::WarnAboutUnknownKeys(value, {PartKey}, prefix + key + " ",
                                              [&](const std::string& unknown, const std::string& message)
                                              { problem(ItemDataIssueSeverity::Warning, unknown, message); });
            return item;
        }
        if (!value.is_object())
        {
            Fatal(key, "must be an item: {\"group\", \"number\"} with \"level\" and \"excellent\" if any");
            return std::nullopt;
        }
        const auto read = [&](const char* name, long long high, int& field, bool required)
        {
            const auto found = value.find(name);
            if (found == value.end())
            {
                if (required)
                    Fatal(key + "." + name, "missing");
                return;
            }
            const std::optional<int> number = Json::ReadWholeIn(*found, 0, high);
            if (!number)
            {
                Fatal(key + "." + name, Json::RangeText(0, high));
                return;
            }
            field = *number;
        };
        MonsterItem item;
        read(GroupKey, MAX_ITEM_TYPE - 1, item.group, true);
        read(NumberKey, MAX_ITEM_INDEX - 1, item.number, true);
        read(LevelKey, MaxItemLevel, item.level, false);
        read(ExcellentKey, MaxExcellentFlags, item.excellent, false);
        if (const auto look = value.find(MountLookKey); look != value.end())
        {
            if (!look->is_string())
                Fatal(key + "." + MountLookKey, "must be a horse look: normal or doppelganger");
            else
            {
                item.mountLook = look->get<std::string>() == "normal" ? std::string() : look->get<std::string>();
                if (!FindMountLook(item.mountLook))
                    Error(key + "." + MountLookKey, look->get<std::string>() + " is not a horse look");
            }
        }
        Items::Json::WarnAboutUnknownKeys(value, {GroupKey, NumberKey, LevelKey, ExcellentKey, MountLookKey},
                                          prefix + key + " ",
                                          [&](const std::string& unknown, const std::string& message)
                                          { problem(ItemDataIssueSeverity::Warning, unknown, message); });
        return item;
    }

    void OptionalItem(const OrderedJson& value, const std::string& key, std::optional<MonsterItem>& field)
    {
        field = value.is_null() ? std::nullopt : Item(value, key);
    }

    void Equipment(const OrderedJson& value, const std::string& key, MonsterEquipment& equipment)
    {
        if (!value.is_object())
        {
            Fatal(key, "must be an object of weapons and armor");
            return;
        }
        equipment = {};
        if (const auto weapons = value.find(WeaponsKey); weapons != value.end())
        {
            if (!weapons->is_array() || weapons->size() > equipment.weapons.size())
            {
                Fatal(key + "." + WeaponsKey, "must be a list of up to two items (null for an empty hand)");
            }
            else
            {
                for (size_t i = 0; i < weapons->size(); ++i)
                {
                    OptionalItem((*weapons)[i], key + "." + WeaponsKey + "[" + std::to_string(i) + "]",
                                 equipment.weapons[i]);
                }
            }
        }
        const std::pair<const char*, std::optional<MonsterItem>*> parts[] = {
            {"helm", &equipment.helm},     {"armor", &equipment.armor}, {"pants", &equipment.pants},
            {"gloves", &equipment.gloves}, {"boots", &equipment.boots}, {"wings", &equipment.wings},
            {"helper", &equipment.helper},
        };
        Items::Json::KeySet known{WeaponsKey};
        for (const auto& [name, field] : parts)
        {
            known.insert(name);
            if (const auto part = value.find(name); part != value.end())
                OptionalItem(*part, key + "." + name, *field);
        }
        Items::Json::WarnAboutUnknownKeys(value, known, prefix + key + " ",
                                          [&](const std::string& unknown, const std::string& message)
                                          { problem(ItemDataIssueSeverity::Warning, unknown, message); });
    }

    void SpawnEffects(const OrderedJson& value, const std::string& key, std::vector<MonsterSpawnEffect>& effects)
    {
        if (!value.is_array())
        {
            Fatal(key, "must be a list of joints, effects, particles, objects and eyes");
            return;
        }
        effects.clear();
        for (size_t i = 0; i < value.size(); ++i)
        {
            const OrderedJson& row = value[i];
            const std::string rowKey = key + "[" + std::to_string(i) + "]";
            const auto named = [&](const char* name) { return row.is_object() && row.contains(name); };
            const int kinds =
                named(JointKey) + named(ObjectKey) + named(EffectKey) + named(ParticleKey) + named(EyesKey);
            if (kinds != 1)
            {
                Fatal(rowKey, "must have one of \"joint\", \"effect\", \"particle\", \"object\" (a type as the "
                              "code names it) or \"eyes\" (bones)");
                continue;
            }
            MonsterSpawnEffect effect;
            if (named(EyesKey))
            {
                effect.kind = MonsterSpawnEffect::Kind::Eyes;
                const OrderedJson& bones = *row.find(EyesKey);
                if (!bones.is_array() || !IsEyeBoneCount(bones.size()))
                {
                    Fatal(rowKey + "." + EyesKey, "must be the bones of the right and left eye, then up to " +
                                                      std::to_string(MaxEyeBones / 2 - 1) + " more pairs");
                    continue;
                }
                for (const OrderedJson& bone : bones)
                {
                    const std::optional<int> number = Json::ReadWholeIn(bone, 0, std::numeric_limits<int>::max());
                    if (!number)
                    {
                        Fatal(rowKey + "." + EyesKey, "must be bones: whole numbers from 0");
                        break;
                    }
                    effect.bones.push_back(*number);
                }
                Items::Json::WarnAboutUnknownKeys(row, {EyesKey}, prefix + rowKey + " ",
                                                  [&](const std::string& unknown, const std::string& message)
                                                  { problem(ItemDataIssueSeverity::Warning, unknown, message); });
                effects.push_back(std::move(effect));
                continue;
            }
            const char* typeKey = named(JointKey)      ? JointKey
                                  : named(EffectKey)   ? EffectKey
                                  : named(ParticleKey) ? ParticleKey
                                                       : ObjectKey;
            if (!(*row.find(typeKey)).is_string())
            {
                Fatal(rowKey + "." + typeKey, "must be a type as the code names it");
                continue;
            }
            effect.kind = typeKey == JointKey      ? MonsterSpawnEffect::Kind::Joint
                          : typeKey == EffectKey   ? MonsterSpawnEffect::Kind::Effect
                          : typeKey == ParticleKey ? MonsterSpawnEffect::Kind::Particle
                                                   : MonsterSpawnEffect::Kind::Object;
            effect.type = (*row.find(typeKey)).get<std::string>();
            if (effect.kind == MonsterSpawnEffect::Kind::Object)
            {
                if (!FindMonsterModelSlot(effect.type))
                    Error(rowKey + "." + ObjectKey, effect.type + " is not a model slot");
                if (const auto angle = row.find(AngleKey); angle != row.end())
                    Float(*angle, rowKey + "." + AngleKey, effect.angle);
            }
            else
            {
                const Effects::EffectKind kind =
                    effect.kind == MonsterSpawnEffect::Kind::Joint      ? Effects::EffectKind::Joint
                    : effect.kind == MonsterSpawnEffect::Kind::Particle ? Effects::EffectKind::Particle
                                                                        : Effects::EffectKind::Effect;
                const auto symbols = Effects::GetEffectTypeSymbols(kind);
                if (std::ranges::none_of(symbols, [&](const auto& symbol) { return symbol.code == effect.type; }))
                    Error(rowKey + "." + typeKey, effect.type + " is not a " +
                                                      std::string(Effects::GetEffectKindName(kind)) +
                                                      " type the code has");
                if (const auto subType = row.find(SubTypeKey); subType != row.end())
                {
                    std::optional<int> number;
                    Whole(*subType, rowKey + "." + SubTypeKey, number);
                    effect.subType = number.value_or(0);
                }
            }
            const bool jointOrParticle =
                effect.kind == MonsterSpawnEffect::Kind::Joint || effect.kind == MonsterSpawnEffect::Kind::Particle;
            const bool madeAtPlace = jointOrParticle || effect.kind == MonsterSpawnEffect::Kind::Effect;
            if (jointOrParticle)
            {
                if (const auto scale = row.find(ScaleKey); scale != row.end())
                {
                    std::optional<float> number;
                    Float(*scale, rowKey + "." + ScaleKey, number);
                    effect.scale = number.value_or(0.0f);
                }
            }
            if (madeAtPlace)
            {
                if (const auto color = row.find(ColorKey); color != row.end())
                {
                    if (!color->is_array() || color->size() != 3 ||
                        std::ranges::any_of(*color, [](const OrderedJson& part) { return !part.is_number(); }))
                        Fatal(rowKey + "." + ColorKey, "must be three numbers: [red, green, blue]");
                    else
                        effect.color = std::array<float, 3>{(*color)[0].get<float>(), (*color)[1].get<float>(),
                                                            (*color)[2].get<float>()};
                }
                if (const auto bone = row.find(BoneKey); bone != row.end())
                {
                    effect.bone = Json::ReadWholeIn(*bone, 0, std::numeric_limits<int>::max());
                    if (!effect.bone)
                        Fatal(rowKey + "." + BoneKey, "must be a bone: a whole number from 0");
                }
                if (const auto count = row.find(CountKey); count != row.end())
                {
                    const std::optional<int> number = Json::ReadWholeIn(*count, 1, MaxSpawnEffectCount);
                    if (!number)
                        Fatal(rowKey + "." + CountKey, Json::RangeText(1, MaxSpawnEffectCount));
                    else
                        effect.count = *number;
                }
                Point(row, OffsetKey, rowKey, effect.offset);
                if (effect.kind == MonsterSpawnEffect::Kind::Joint)
                    Point(row, TargetOffsetKey, rowKey, effect.targetOffset);
                if (const auto random = row.find(ScaleRandomKey); random != row.end())
                {
                    const std::optional<int> number = Json::ReadWholeIn(*random, 1, MaxSpawnEffectScaleRandom);
                    if (!number)
                        Fatal(rowKey + "." + ScaleRandomKey, Json::RangeText(1, MaxSpawnEffectScaleRandom));
                    else
                        effect.scaleRandom = *number;
                }
            }
            if (effect.kind == MonsterSpawnEffect::Kind::Effect || effect.kind == MonsterSpawnEffect::Kind::Joint)
            {
                if (const auto keyValue = row.find(KeyKey); keyValue != row.end())
                {
                    const std::optional<int> number = Json::ReadWholeIn(*keyValue, std::numeric_limits<short>::min(),
                                                                        std::numeric_limits<short>::max());
                    if (!number)
                        Fatal(rowKey + "." + KeyKey,
                              Json::RangeText(std::numeric_limits<short>::min(), std::numeric_limits<short>::max()));
                    else
                        effect.key = *number;
                }
            }
            Items::Json::WarnAboutUnknownKeys(
                row,
                {JointKey, EffectKey, ParticleKey, ObjectKey, SubTypeKey, ScaleKey, ColorKey, KeyKey, AngleKey, BoneKey,
                 CountKey, OffsetKey, TargetOffsetKey, ScaleRandomKey},
                prefix + rowKey + " ", [&](const std::string& unknown, const std::string& message)
                { problem(ItemDataIssueSeverity::Warning, unknown, message); });
            effects.push_back(std::move(effect));
        }
    }

    // Three numbers: x, y and z.
    void Point(const OrderedJson& row, const char* name, const std::string& rowKey,
               std::optional<std::array<float, 3>>& point)
    {
        const auto value = row.find(name);
        if (value == row.end())
            return;
        if (!value->is_array() || value->size() != 3 ||
            std::ranges::any_of(*value, [](const OrderedJson& part) { return !part.is_number(); }))
        {
            Fatal(rowKey + "." + name, "must be three numbers: [x, y, z]");
            return;
        }
        point = std::array<float, 3>{(*value)[0].get<float>(), (*value)[1].get<float>(), (*value)[2].get<float>()};
    }

    // How it comes in (Q-MN38c): an animation, a fade, a sound and what it
    // brings, the joints, effects and particles of spawn effects.
    void Appear(const OrderedJson& value, const std::string& key, std::optional<MonsterAppear>& appear)
    {
        if (!value.is_object())
        {
            Fatal(key, "must be an object: action, blend, startFrame, fadeIn, appearTime, sound, effects");
            return;
        }
        MonsterAppear read;
        if (const auto action = value.find(ActionKey); action != value.end())
        {
            if (!action->is_string())
                Fatal(key + "." + ActionKey, "must be an animation by its name: stop2, appear, ...");
            else
            {
                read.action = action->get<std::string>();
                if (!FindMonsterAnimation(*read.action))
                    Error(key + "." + ActionKey, *read.action + " is not an animation's name");
            }
        }
        if (const auto blend = value.find(BlendKey); blend != value.end())
        {
            std::optional<bool> flag;
            Bool(*blend, key + "." + BlendKey, flag);
            read.blend = flag.value_or(false);
        }
        if (const auto frame = value.find(StartFrameKey); frame != value.end())
            Float(*frame, key + "." + StartFrameKey, read.startFrame);
        if (const auto fadeIn = value.find(FadeInKey); fadeIn != value.end())
            Bool(*fadeIn, key + "." + FadeInKey, read.fadeIn);
        if (const auto time = value.find(AppearTimeKey); time != value.end())
            Whole(*time, key + "." + AppearTimeKey, read.appearTime);
        if (const auto sound = value.find(SoundKey); sound != value.end())
        {
            if (!sound->is_string())
                Fatal(key + "." + SoundKey, "must be a sound by its SOUND_ name");
            else
            {
                read.sound = sound->get<std::string>();
                if (!FindMonsterSoundId(*read.sound))
                    Error(key + "." + SoundKey, *read.sound + " is not a SOUND_ id that the monster data can name");
            }
        }
        if (const auto effects = value.find(EffectsKey); effects != value.end())
        {
            const std::string effectsKey = key + "." + EffectsKey;
            SpawnEffects(*effects, effectsKey, read.effects);
            for (size_t i = 0; i < read.effects.size(); ++i)
            {
                const MonsterSpawnEffect::Kind kind = read.effects[i].kind;
                if (kind == MonsterSpawnEffect::Kind::Object || kind == MonsterSpawnEffect::Kind::Eyes)
                    Fatal(effectsKey + "[" + std::to_string(i) + "]", "must be a joint, an effect or a particle");
            }
        }
        Items::Json::WarnAboutUnknownKeys(
            value, {ActionKey, BlendKey, StartFrameKey, FadeInKey, AppearTimeKey, SoundKey, EffectsKey},
            prefix + key + " ", [&](const std::string& unknown, const std::string& message)
            { problem(ItemDataIssueSeverity::Warning, unknown, message); });
        appear = std::move(read);
    }

    // A castle gate's animations and how an open one comes into view (Q-MN38c).
    void Gate(const OrderedJson& value, const std::string& key, std::optional<MonsterGate>& gate)
    {
        if (!value.is_object())
        {
            Fatal(key, "must be an object: closed, opening, open, arrival");
            return;
        }
        MonsterGate read;
        const auto animation = [&](const char* name, std::string& to)
        {
            const auto found = value.find(name);
            if (found == value.end())
                return;
            if (!found->is_string())
            {
                Fatal(key + "." + name, "must be an animation by its name: stop1, stop2, walk, ...");
                return;
            }
            to = found->get<std::string>();
            if (!FindMonsterAnimation(to))
                Error(key + "." + name, to + " is not an animation's name");
        };
        animation(GateClosedKey, read.closed);
        animation(GateOpeningKey, read.opening);
        animation(GateOpenKey, read.open);
        if (const auto arrival = value.find(GateArrivalKey); arrival != value.end())
        {
            if (!arrival->is_string() ||
                (arrival->get<std::string>() != GateOpeningKey && arrival->get<std::string>() != GateOpenKey))
                Fatal(key + "." + GateArrivalKey, "must be opening (it opens again) or open (it stands open)");
            else
                read.arrivalOpening = arrival->get<std::string>() == GateOpeningKey;
        }
        Items::Json::WarnAboutUnknownKeys(value, {GateClosedKey, GateOpeningKey, GateOpenKey, GateArrivalKey},
                                          prefix + key + " ",
                                          [&](const std::string& unknown, const std::string& message)
                                          { problem(ItemDataIssueSeverity::Warning, unknown, message); });
        gate = std::move(read);
    }

    void Names(const OrderedJson& value, const std::string& key, std::vector<std::string>& names)
    {
        if (!value.is_array() || std::ranges::any_of(value, [](const OrderedJson& name) { return !name.is_string(); }))
        {
            Fatal(key, "must be a list of names");
            return;
        }
        names.clear();
        for (const OrderedJson& name : value)
        {
            names.push_back(name.get<std::string>());
            if (!FindMonsterBuff(names.back()))
                Error(key, names.back() + " is not a buff a monster can get at spawn");
        }
    }

    std::optional<std::array<float, 3>> Point(const OrderedJson& value, const std::string& key)
    {
        if (!value.is_array() || value.size() != 3 ||
            std::ranges::any_of(value, [](const OrderedJson& number) { return !number.is_number(); }))
        {
            Fatal(key, "must be three numbers: [x, y, z]");
            return std::nullopt;
        }
        return std::array<float, 3>{value[0].get<float>(), value[1].get<float>(), value[2].get<float>()};
    }

    void Box(const OrderedJson& value, const std::string& key, MonsterSetup& setup)
    {
        if (!value.is_object() || (!value.contains("min") && !value.contains("max")))
        {
            Fatal(key, "must be an object with min, max or both");
            return;
        }
        setup.boxMin = value.contains("min") ? Point(*value.find("min"), key + ".min") : std::nullopt;
        setup.boxMax = value.contains("max") ? Point(*value.find("max"), key + ".max") : std::nullopt;
        Items::Json::WarnAboutUnknownKeys(value, {"min", "max"}, prefix + key + " ",
                                          [&](const std::string& unknown, const std::string& message)
                                          { problem(ItemDataIssueSeverity::Warning, unknown, message); });
    }

    void WholeList(const OrderedJson& value, const std::string& key, std::vector<int>& values)
    {
        if (!value.is_array() || value.empty())
        {
            Fatal(key, "must be a list of whole numbers");
            return;
        }
        values.clear();
        for (const OrderedJson& number : value)
        {
            std::optional<int> read;
            Whole(number, key, read);
            if (read)
                values.push_back(*read);
        }
    }

    void Field(std::string_view key, const OrderedJson& value, MonsterSetup& setup)
    {
        const std::string name(key);
        if (key == "scale")
        {
            if (value.is_string() && value.get_ref<const std::string&>() == CharacterScale)
            {
                setup.scale.reset();
                setup.scaleByCharacter = true;
                return;
            }
            setup.scaleByCharacter = false;
            Float(value, name, setup.scale);
        }
        else if (key == "outline")
            Float(value, name, setup.outline);
        else if (key == "subType")
            Whole(value, name, setup.subType);
        else if (key == "lookLevel")
            WholeIn(value, name, 0, MaxLookLevel, setup.lookLevel);
        else if (key == "glowMesh")
            Whole(value, name, setup.glowMesh);
        else if (key == "glowLight")
            Float(value, name, setup.glowLight);
        else if (key == "hiddenMesh")
            Whole(value, name, setup.hiddenMesh);
        else if (key == "shadow")
            Bool(value, name, setup.shadow);
        else if (key == "hideShadow")
            Bool(value, name, setup.hideShadow);
        else if (key == "alphaTarget")
            Float(value, name, setup.alphaTarget);
        else if (key == "light")
            Bool(value, name, setup.light);
        else if (key == "pkColor")
        {
            if (!value.is_string())
            {
                Fatal(name, "must be a standing: hero2, hero1, neutral, caution, murderer1 or murderer2");
                return;
            }
            setup.pkColor = FindPkColor(value.get<std::string>());
            if (!setup.pkColor)
                Error(name, value.get<std::string>() + " is not a standing");
        }
        else if (key == "helperPlate")
            Bool(value, name, setup.helperPlate);
        else if (key == "selectable")
            Bool(value, name, setup.selectable);
        else if (key == "class")
        {
            const auto keys = Items::GetClassKeys();
            const auto found = value.is_string() ? std::ranges::find(keys, value.get<std::string>()) : keys.end();
            if (!value.is_string())
                Fatal(name, "must be a class as the items name them: darkWizard, darkKnight, ...");
            else if (found == keys.end())
                Error(name, value.get<std::string>() + " is not a class");
            else
                setup.playerClass = static_cast<int>(found - keys.begin());
        }
        else if (key == "classStep")
        {
            const std::optional<int> step = Json::ReadWholeIn(value, 0, MaxClassStep);
            if (!step)
                Fatal(name, Json::RangeText(0, MaxClassStep) + ": the class, its second or third class");
            else
                setup.classStep = step;
        }
        else if (key == "equipment")
            Equipment(value, name, setup.equipment);
        else if (key == "spawnEffects")
            SpawnEffects(value, name, setup.spawnEffects);
        else if (key == "spawnBuffs")
            Names(value, name, setup.spawnBuffs);
        else if (key == "spawnSound")
        {
            if (!value.is_string())
            {
                Fatal(name, "must be a sound by its SOUND_ name");
                return;
            }
            setup.spawnSound = value.get<std::string>();
            if (!FindMonsterSoundId(*setup.spawnSound))
                Error(name, *setup.spawnSound + " is not a SOUND_ id that the monster data can name");
        }
        else if (key == "randomLook")
            WholeList(value, name, setup.randomLook);
        else if (key == "randomStart")
        {
            std::optional<bool> random;
            Bool(value, name, random);
            setup.randomStart = random.value_or(false);
        }
        else if (key == "positionOffset")
        {
            if (!value.is_array() || value.size() != 2 || !value[0].is_number() || !value[1].is_number())
            {
                Fatal(name, "must be two numbers: [x, y]");
                return;
            }
            setup.positionOffset = std::array<float, 2>{value[0].get<float>(), value[1].get<float>()};
        }
        else if (key == "keepGroundHeight")
            Bool(value, name, setup.keepGroundHeight);
        else if (key == "heightOffset")
            Float(value, name, setup.heightOffset);
        else if (key == "height")
            Float(value, name, setup.height);
        else if (key == "tileOffset")
        {
            const auto x = value.is_array() && value.size() == 2
                               ? Json::ReadWholeIn(value[0], -MaxTileOffset, MaxTileOffset)
                               : std::nullopt;
            const auto y = value.is_array() && value.size() == 2
                               ? Json::ReadWholeIn(value[1], -MaxTileOffset, MaxTileOffset)
                               : std::nullopt;
            if (!x || !y)
            {
                Fatal(name, "must be two whole numbers from " + std::to_string(-MaxTileOffset) + " to " +
                                std::to_string(MaxTileOffset) + ": [x, y]");
                return;
            }
            setup.tileOffset = std::array<int, 2>{*x, *y};
        }
        else if (key == "boundingBox")
            Box(value, name, setup);
        else if (key == "startAction")
            Whole(value, name, setup.startAction);
        else if (key == "startFrame")
            Float(value, name, setup.startFrame);
        else if (key == "deathEffect")
            Whole(value, name, setup.deathEffect);
        else if (key == "live")
            Bool(value, name, setup.live);
        else if (key == "noTurnOnHit")
            Bool(value, name, setup.noTurnOnHit);
        else if (key == "moveSpeed")
            WholeIn(value, name, 0, MaxMoveSpeed, setup.moveSpeed);
        else if (key == "noBloodPool")
            Bool(value, name, setup.noBloodPool);
        else if (key == "stateVariants")
            StateVariants(value, name, setup.stateVariants);
        else if (key == "hooks")
            Hooks(value, name, setup.hooks);
        else if (key == "facing")
            Float(value, name, setup.facing);
        else if (key == "facingStep")
            Float(value, name, setup.facingStep);
        else if (key == "facingByTile")
            TileFacings(value, name, setup.facingByTile);
        else if (key == "appear")
            Appear(value, name, setup.appear);
        else if (key == "teleport")
            Appear(value, name, setup.teleport);
        else if (key == "gate")
            Gate(value, name, setup.gate);
    }

    // Each a tile coordinate, x or y, and the step there.
    void TileFacings(const OrderedJson& value, const std::string& key, std::vector<MonsterTileFacing>& rows)
    {
        if (!value.is_array())
        {
            Fatal(key, "must be a list of tiles with their facing step");
            return;
        }
        rows.clear();
        for (size_t i = 0; i < value.size(); ++i)
        {
            const OrderedJson& row = value[i];
            const std::string rowKey = key + "[" + std::to_string(i) + "]";
            const bool x = row.is_object() && row.contains(TileXKey);
            const bool y = row.is_object() && row.contains(TileYKey);
            if (x == y || !row.contains(FacingStepKey))
            {
                Fatal(rowKey, "must have a tileX or a tileY and a facingStep");
                continue;
            }
            MonsterTileFacing facing;
            const std::optional<int> tile = Json::ReadWholeIn(*row.find(x ? TileXKey : TileYKey), 0, MaxFacingTile);
            if (!tile)
            {
                Fatal(rowKey + "." + (x ? TileXKey : TileYKey), Json::RangeText(0, MaxFacingTile));
                continue;
            }
            (x ? facing.tileX : facing.tileY) = tile;
            std::optional<float> step;
            Float(*row.find(FacingStepKey), rowKey + "." + FacingStepKey, step);
            if (!step)
                continue;
            facing.step = *step;
            Items::Json::WarnAboutUnknownKeys(row, {TileXKey, TileYKey, FacingStepKey}, prefix + rowKey + " ",
                                              [&](const std::string& unknown, const std::string& message)
                                              { problem(ItemDataIssueSeverity::Warning, unknown, message); });
            rows.push_back(facing);
        }
    }

    // Each a hook of GetMonsterHookKinds with its value.
    void Hooks(const OrderedJson& value, const std::string& key, std::vector<MonsterHook>& hooks)
    {
        if (!value.is_array())
        {
            Fatal(key, "must be a list of hooks");
            return;
        }
        hooks.clear();
        for (size_t i = 0; i < value.size(); ++i)
        {
            const OrderedJson& row = value[i];
            const std::string rowKey = key + "[" + std::to_string(i) + "]";
            const auto name = row.is_object() ? row.find(HookKey) : row.end();
            if (!row.is_object() || name == row.end() || !name->is_string())
            {
                Fatal(rowKey, "must be an object with the hook's name and its value");
                continue;
            }
            MonsterHook hook;
            hook.name = name->get<std::string>();
            Items::Json::KeySet known{HookKey};
            const MonsterHookKind* kind = FindMonsterHookKind(hook.name);
            if (kind == nullptr)
                Error(rowKey + "." + HookKey, hook.name + " is not a hook the maps run at spawn");
            else if (!kind->valueKey.empty())
            {
                const std::string valueKey(kind->valueKey);
                known.insert(valueKey);
                const auto found = row.find(valueKey);
                if (found == row.end() || !found->is_string())
                {
                    Fatal(rowKey + "." + valueKey, "must be the hook's " + valueKey);
                    continue;
                }
                const std::string hookValue = found->get<std::string>();
                if (std::ranges::find(kind->values, hookValue) == kind->values.end())
                    Error(rowKey + "." + valueKey, hookValue + " is not a " + valueKey + " the hook knows");
                hook.values.emplace_back(valueKey, hookValue);
            }
            Items::Json::WarnAboutUnknownKeys(row, known, prefix + rowKey + " ",
                                              [&](const std::string& unknown, const std::string& message)
                                              { problem(ItemDataIssueSeverity::Warning, unknown, message); });
            hooks.push_back(std::move(hook));
        }
    }

    // A state variant: the state, and the fields it changes; it changes and
    // clears none, and holds no state variants of its own.
    void StateVariants(const OrderedJson& value, const std::string& key, std::vector<MonsterStateVariant>& variants)
    {
        if (!value.is_array())
        {
            Fatal(key, "must be a list of state variants");
            return;
        }
        variants.clear();
        for (size_t i = 0; i < value.size(); ++i)
        {
            const OrderedJson& row = value[i];
            const std::string rowKey = key + "[" + std::to_string(i) + "]";
            const auto state = row.is_object() ? row.find(StateKey) : row.end();
            if (!row.is_object() || state == row.end() || !state->is_string())
            {
                Fatal(rowKey, "must be an object with a state and the fields it changes");
                continue;
            }
            MonsterStateVariant variant;
            variant.state = state->get<std::string>();
            if (!IsMonsterState(variant.state))
                Error(rowKey + "." + StateKey, variant.state + " is not a state the game tells");
            bool rowValid = true;
            for (const MonsterSetupField& field : GetMonsterSetupFields())
            {
                const auto fieldValue = row.find(field.key);
                if (fieldValue == row.end())
                    continue;
                if (field.key == "stateVariants" || fieldValue->is_null())
                {
                    Fatal(rowKey + "." + std::string(field.key),
                          field.key == "stateVariants" ? "a state variant holds no state variants"
                                                       : "must be a value: a state variant changes a field, it does "
                                                         "not clear it");
                    rowValid = false;
                    continue;
                }
                Field(field.key, *fieldValue, variant.setup);
            }
            Items::Json::KeySet known{StateKey};
            AddMonsterSetupKeys(known);
            Items::Json::WarnAboutUnknownKeys(row, known, prefix + rowKey + " ",
                                              [&](const std::string& unknown, const std::string& message)
                                              { problem(ItemDataIssueSeverity::Warning, unknown, message); });
            if (rowValid)
                variants.push_back(std::move(variant));
        }
    }
};

OrderedJson WriteItem(const MonsterItem& item)
{
    OrderedJson json = OrderedJson::object();
    if (!item.part.empty())
    {
        json[PartKey] = item.part;
        return json;
    }
    json[GroupKey] = item.group;
    json[NumberKey] = item.number;
    if (item.level != 0)
        json[LevelKey] = item.level;
    if (item.excellent != 0)
        json[ExcellentKey] = item.excellent;
    if (!item.mountLook.empty())
        json[MountLookKey] = item.mountLook;
    return json;
}

OrderedJson WriteEquipment(const MonsterEquipment& equipment)
{
    OrderedJson json = OrderedJson::object();
    // An empty second hand is left out; an empty first one before a second is null.
    const size_t hands = equipment.weapons[1] ? 2 : (equipment.weapons[0] ? 1 : 0);
    if (hands > 0)
    {
        OrderedJson weapons = OrderedJson::array();
        for (size_t i = 0; i < hands; ++i)
            weapons.push_back(equipment.weapons[i] ? WriteItem(*equipment.weapons[i]) : OrderedJson(nullptr));
        json[WeaponsKey] = std::move(weapons);
    }
    const std::pair<const char*, const std::optional<MonsterItem>*> parts[] = {
        {"helm", &equipment.helm},     {"armor", &equipment.armor}, {"pants", &equipment.pants},
        {"gloves", &equipment.gloves}, {"boots", &equipment.boots}, {"wings", &equipment.wings},
        {"helper", &equipment.helper},
    };
    for (const auto& [name, part] : parts)
    {
        if (*part)
            json[name] = WriteItem(**part);
    }
    return json;
}

OrderedJson WriteSpawnEffects(const std::vector<MonsterSpawnEffect>& effects)
{
    OrderedJson list = OrderedJson::array();
    for (const MonsterSpawnEffect& effect : effects)
    {
        OrderedJson row = OrderedJson::object();
        // Where and how many: a joint's, effect's or particle's.
        const auto placed = [&]()
        {
            if (effect.color)
                row[ColorKey] =
                    OrderedJson::array({Json::WriteFloat((*effect.color)[0]), Json::WriteFloat((*effect.color)[1]),
                                        Json::WriteFloat((*effect.color)[2])});
            if (effect.bone)
                row[BoneKey] = *effect.bone;
            if (effect.count != 1)
                row[CountKey] = effect.count;
            const auto point = [](const std::array<float, 3>& value)
            {
                return OrderedJson::array(
                    {Json::WriteFloat(value[0]), Json::WriteFloat(value[1]), Json::WriteFloat(value[2])});
            };
            if (effect.offset)
                row[OffsetKey] = point(*effect.offset);
            if (effect.targetOffset)
                row[TargetOffsetKey] = point(*effect.targetOffset);
            if (effect.scaleRandom != 0)
                row[ScaleRandomKey] = effect.scaleRandom;
        };
        switch (effect.kind)
        {
        case MonsterSpawnEffect::Kind::Joint:
            row[JointKey] = effect.type;
            if (effect.subType != 0)
                row[SubTypeKey] = effect.subType;
            if (effect.scale != 0.0f)
                row[ScaleKey] = Json::WriteFloat(effect.scale);
            if (effect.key != -1)
                row[KeyKey] = effect.key;
            placed();
            break;
        case MonsterSpawnEffect::Kind::Effect:
            row[EffectKey] = effect.type;
            if (effect.subType != 0)
                row[SubTypeKey] = effect.subType;
            if (effect.key != -1)
                row[KeyKey] = effect.key;
            placed();
            break;
        case MonsterSpawnEffect::Kind::Particle:
            row[ParticleKey] = effect.type;
            if (effect.subType != 0)
                row[SubTypeKey] = effect.subType;
            if (effect.scale != 0.0f)
                row[ScaleKey] = Json::WriteFloat(effect.scale);
            placed();
            break;
        case MonsterSpawnEffect::Kind::Object:
            row[ObjectKey] = effect.type;
            if (effect.angle)
                row[AngleKey] = Json::WriteFloat(*effect.angle);
            break;
        case MonsterSpawnEffect::Kind::Eyes:
            row[EyesKey] = effect.bones;
            break;
        }
        list.push_back(std::move(row));
    }
    return list;
}

OrderedJson WriteAppear(const MonsterAppear& appear)
{
    OrderedJson json = OrderedJson::object();
    if (appear.action)
        json[ActionKey] = *appear.action;
    if (appear.blend)
        json[BlendKey] = true;
    if (appear.startFrame)
        json[StartFrameKey] = Json::WriteFloat(*appear.startFrame);
    if (appear.fadeIn)
        json[FadeInKey] = *appear.fadeIn;
    if (appear.appearTime)
        json[AppearTimeKey] = *appear.appearTime;
    if (appear.sound)
        json[SoundKey] = *appear.sound;
    if (!appear.effects.empty())
        json[EffectsKey] = WriteSpawnEffects(appear.effects);
    return json;
}

template <typename T> OrderedJson WriteOptional(const std::optional<T>& value)
{
    if constexpr (std::is_same_v<T, float>)
        return Json::WriteFloat(*value);
    else
        return *value;
}

// The value of a field the setup has.
OrderedJson WriteField(std::string_view key, const MonsterSetup& setup)
{
    if (key == "scale")
        return setup.scaleByCharacter ? OrderedJson(CharacterScale) : Json::WriteFloat(*setup.scale);
    if (key == "outline")
        return WriteOptional(setup.outline);
    if (key == "subType")
        return WriteOptional(setup.subType);
    if (key == "lookLevel")
        return WriteOptional(setup.lookLevel);
    if (key == "glowMesh")
        return WriteOptional(setup.glowMesh);
    if (key == "glowLight")
        return WriteOptional(setup.glowLight);
    if (key == "hiddenMesh")
        return WriteOptional(setup.hiddenMesh);
    if (key == "shadow")
        return WriteOptional(setup.shadow);
    if (key == "hideShadow")
        return WriteOptional(setup.hideShadow);
    if (key == "alphaTarget")
        return WriteOptional(setup.alphaTarget);
    if (key == "light")
        return WriteOptional(setup.light);
    if (key == "pkColor")
        return std::string(GetPkColorName(*setup.pkColor));
    if (key == "helperPlate")
        return WriteOptional(setup.helperPlate);
    if (key == "selectable")
        return WriteOptional(setup.selectable);
    if (key == "class")
        return Items::GetClassKeys()[static_cast<size_t>(*setup.playerClass)];
    if (key == "classStep")
        return WriteOptional(setup.classStep);
    if (key == "equipment")
        return WriteEquipment(setup.equipment);
    if (key == "spawnEffects")
        return WriteSpawnEffects(setup.spawnEffects);
    if (key == "spawnBuffs")
        return setup.spawnBuffs;
    if (key == "spawnSound")
        return *setup.spawnSound;
    if (key == "randomLook")
        return setup.randomLook;
    if (key == "randomStart")
        return setup.randomStart;
    if (key == "positionOffset")
        return OrderedJson::array(
            {Json::WriteFloat((*setup.positionOffset)[0]), Json::WriteFloat((*setup.positionOffset)[1])});
    if (key == "keepGroundHeight")
        return WriteOptional(setup.keepGroundHeight);
    if (key == "heightOffset")
        return WriteOptional(setup.heightOffset);
    if (key == "height")
        return WriteOptional(setup.height);
    if (key == "tileOffset")
        return OrderedJson::array({(*setup.tileOffset)[0], (*setup.tileOffset)[1]});
    if (key == "boundingBox")
    {
        OrderedJson box = OrderedJson::object();
        const auto point = [](const std::array<float, 3>& value)
        {
            return OrderedJson::array(
                {Json::WriteFloat(value[0]), Json::WriteFloat(value[1]), Json::WriteFloat(value[2])});
        };
        if (setup.boxMin)
            box["min"] = point(*setup.boxMin);
        if (setup.boxMax)
            box["max"] = point(*setup.boxMax);
        return box;
    }
    if (key == "startAction")
        return WriteOptional(setup.startAction);
    if (key == "startFrame")
        return WriteOptional(setup.startFrame);
    if (key == "deathEffect")
        return WriteOptional(setup.deathEffect);
    if (key == "live")
        return WriteOptional(setup.live);
    if (key == "noTurnOnHit")
        return WriteOptional(setup.noTurnOnHit);
    if (key == "moveSpeed")
        return WriteOptional(setup.moveSpeed);
    if (key == "noBloodPool")
        return WriteOptional(setup.noBloodPool);
    if (key == "facing")
        return WriteOptional(setup.facing);
    if (key == "facingStep")
        return WriteOptional(setup.facingStep);
    if (key == "facingByTile")
    {
        OrderedJson list = OrderedJson::array();
        for (const MonsterTileFacing& facing : setup.facingByTile)
        {
            OrderedJson row = OrderedJson::object();
            if (facing.tileX)
                row[TileXKey] = *facing.tileX;
            if (facing.tileY)
                row[TileYKey] = *facing.tileY;
            row[FacingStepKey] = Json::WriteFloat(facing.step);
            list.push_back(std::move(row));
        }
        return list;
    }
    if (key == "appear")
        return WriteAppear(*setup.appear);
    if (key == "teleport")
        return WriteAppear(*setup.teleport);
    if (key == "gate")
    {
        OrderedJson gate = OrderedJson::object();
        gate[GateClosedKey] = setup.gate->closed;
        gate[GateOpeningKey] = setup.gate->opening;
        gate[GateOpenKey] = setup.gate->open;
        gate[GateArrivalKey] = setup.gate->arrivalOpening ? GateOpeningKey : GateOpenKey;
        return gate;
    }
    if (key == "hooks")
    {
        OrderedJson list = OrderedJson::array();
        for (const MonsterHook& hook : setup.hooks)
        {
            OrderedJson row = OrderedJson::object();
            row[HookKey] = hook.name;
            for (const auto& [valueKey, hookValue] : hook.values)
                row[valueKey] = hookValue;
            list.push_back(std::move(row));
        }
        return list;
    }
    if (key == "stateVariants")
    {
        OrderedJson list = OrderedJson::array();
        for (const MonsterStateVariant& variant : setup.stateVariants)
        {
            OrderedJson row = OrderedJson::object();
            row[StateKey] = variant.state;
            WriteMonsterSetup(row, variant.setup);
            list.push_back(std::move(row));
        }
        return list;
    }
    return nullptr;
}
} // namespace

bool ReadMonsterSetup(const OrderedJson& json, const std::string& fieldPrefix, MonsterSetup& setup,
                      const SetupProblem& problem)
{
    Reader reader{fieldPrefix, problem};
    for (const MonsterSetupField& field : GetMonsterSetupFields())
    {
        const auto value = json.find(field.key);
        if (value == json.end())
            continue;
        if (value->is_null())
        {
            field.copy(setup, MonsterSetup{});
            continue;
        }
        reader.Field(field.key, *value, setup);
    }
    return reader.valid;
}

void WriteMonsterSetup(OrderedJson& json, const MonsterSetup& setup)
{
    for (const MonsterSetupField& field : GetMonsterSetupFields())
    {
        if (field.isSet(setup))
            json[std::string(field.key)] = WriteField(field.key, setup);
    }
}

void WriteMonsterSetupDifferences(OrderedJson& json, const MonsterSetup& shared, const MonsterSetup& entry)
{
    for (const MonsterSetupField& field : GetMonsterSetupFields())
    {
        if (field.isSame(shared, entry))
            continue;
        json[std::string(field.key)] = field.isSet(entry) ? WriteField(field.key, entry) : OrderedJson(nullptr);
    }
}

bool ReadMonsterEquipment(const OrderedJson& value, const std::string& field, MonsterEquipment& equipment,
                          const SetupProblem& problem)
{
    const std::string noPrefix;
    Reader reader{noPrefix, problem};
    reader.Equipment(value, field, equipment);
    return reader.valid;
}

OrderedJson WriteMonsterEquipment(const MonsterEquipment& equipment)
{
    return WriteEquipment(equipment);
}

void AddMonsterSetupKeys(Items::Json::KeySet& keys)
{
    for (const MonsterSetupField& field : GetMonsterSetupFields())
        keys.insert(std::string(field.key));
}
} // namespace Data::Monsters
