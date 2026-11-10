#include "stdafx.h"

#include "MonsterCreationRecorder.h"

#include "../effects/RecorderFields.h"

#include "Core/Globals/_enum.h"
#include "Core/Text/Utf8.h"
#include "Core/Utilities/Random.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "GameLogic/Skills/SkillEffectMgr.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/BoneManager.h"

#include <algorithm>
#include <array>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <new>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

void MoveCharacterVisual(CHARACTER* c, OBJECT* o);

namespace MonsterRecorder
{
namespace
{
using RecorderFields::VisitFields;
using Dump = std::map<std::string, std::string>;

// The slot the creations take: the first after the hero.
constexpr int CreatedSlot = 1;
// The seeds of rand() and Random::, the same before every creation: a random
// look draws the same in both records of a comparison.
constexpr unsigned RecordSeed = 20261106;
// The key of a dirty slot's last character, which no creation uses.
constexpr int DirtyKey = 0x7E00;

// ---------------------------------------------------------------------------
// Values as text.
// ---------------------------------------------------------------------------
std::string Text(bool value)
{
    return value ? "true" : "false";
}
std::string Text(float value)
{
    char text[32];
    std::snprintf(text, sizeof(text), "%.9g", value);
    return text;
}
std::string Text(double value)
{
    char text[32];
    std::snprintf(text, sizeof(text), "%.17g", value);
    return text;
}

std::string NameOf(const void* pointer)
{
    if (pointer == nullptr)
        return "null";
    if (pointer == &CharactersClient[0].Object || pointer == &CharactersClient[0])
        return "hero";
    if (pointer == &CharactersClient[CreatedSlot].Object || pointer == &CharactersClient[CreatedSlot])
        return "monster";
    for (int i = 0; i < MAX_EFFECTS; ++i)
    {
        if (pointer == &Effects[i])
            return "Effects[" + std::to_string(i) + "]";
    }
    for (int i = 0; i <= MAX_CHARACTERS_CLIENT; ++i)
    {
        if (pointer == &CharactersClient[i].Object || pointer == &CharactersClient[i])
            return "characters[" + std::to_string(i) + "]";
    }
    // Heap memory the creation took (bones, cloth, parts): its address
    // differs from run to run.
    return "set";
}

std::string Text(const Buff& value)
{
    std::string text = "{";
    for (const auto& [buff, count] : value.m_Buff)
    {
        text += std::to_string(static_cast<int>(buff)) + ":" + std::to_string(count) + " ";
    }
    return text + "}";
}

template <typename Factor> std::uint64_t HashFactors(std::uint64_t hash, const std::vector<Factor>& factors)
{
    const auto* bytes = reinterpret_cast<const unsigned char*>(factors.data());
    for (size_t i = 0; i < factors.size() * sizeof(Factor); ++i)
        hash = (hash ^ bytes[i]) * 1099511628211ull;
    return hash;
}

std::string Text(const CInterpolateContainer& value)
{
    std::uint64_t hash = 1469598103934665603ull;
    hash = HashFactors(hash, value.m_vecInterpolatesAngle);
    hash = HashFactors(hash, value.m_vecInterpolatesPos);
    hash = HashFactors(hash, value.m_vecInterpolatesScale);
    hash = HashFactors(hash, value.m_vecInterpolatesAlpha);
    return std::to_string(value.m_vecInterpolatesAngle.size()) + " " +
           std::to_string(value.m_vecInterpolatesPos.size()) + " " +
           std::to_string(value.m_vecInterpolatesScale.size()) + " " +
           std::to_string(value.m_vecInterpolatesAlpha.size()) + " hash " + std::to_string(hash);
}

// ---------------------------------------------------------------------------
// A full dump of a character: every field by its path.
// ---------------------------------------------------------------------------
template <typename T> void Add(Dump& dump, const std::string& path, const T& value);

template <typename Struct> struct FieldDumper
{
    Dump& dump;
    std::string prefix;

    template <typename T> void operator()(const char* name, T& target, const T&)
    {
        Add(dump, prefix + name, target);
    }
};

template <typename Struct> void AddStruct(Dump& dump, const std::string& path, const Struct& value)
{
    auto& target = const_cast<Struct&>(value);
    VisitFields(target, target, FieldDumper<Struct>{dump, path + "."});
}

template <typename T> void Add(Dump& dump, const std::string& path, const T& value)
{
    if constexpr (std::is_array_v<T>)
    {
        using Element = std::remove_extent_t<T>;
        if constexpr (std::is_same_v<Element, wchar_t>)
        {
            dump[path] = Core::Text::ToUtf8(std::wstring(value, wcsnlen(value, std::extent_v<T>)).c_str());
        }
        else
        {
            for (size_t i = 0; i < std::extent_v<T>; ++i)
                Add(dump, path + "[" + std::to_string(i) + "]", value[i]);
        }
    }
    else if constexpr (std::is_same_v<T, std::array<std::uint32_t, 4>>)
    {
        for (size_t i = 0; i < value.size(); ++i)
            Add(dump, path + "[" + std::to_string(i) + "]", value[i]);
    }
    else if constexpr (std::is_same_v<T, OBJECT> || std::is_same_v<T, PART_t> || std::is_same_v<T, PATH_t> ||
                       std::is_same_v<T, PET_INFO> || std::is_same_v<T, PARTICLE> || std::is_same_v<T, JOINT>)
    {
        AddStruct(dump, path, value);
    }
    else if constexpr (std::is_pointer_v<T>)
    {
        dump[path] = NameOf(value);
    }
    else if constexpr (std::is_enum_v<T>)
    {
        dump[path] = std::to_string(static_cast<long long>(value));
    }
    else if constexpr (std::is_same_v<T, bool> || std::is_floating_point_v<T>)
    {
        dump[path] = Text(value);
    }
    else if constexpr (std::is_integral_v<T>)
    {
        dump[path] = std::to_string(static_cast<long long>(value));
    }
    else
    {
        dump[path] = Text(value);
    }
}

Dump DumpCharacter(CHARACTER& character)
{
    Dump dump;
    VisitFields(character, character, FieldDumper<CHARACTER>{dump, "character."});
    AddStruct(dump, "object", character.Object);
    // Whether it has one: its count is read only through a pointer that may
    // be null.
    dump["character.postMoveProcess"] = Text(character.PostMoveProcess_IsProcessing());
    return dump;
}

// ---------------------------------------------------------------------------
// The slot before the creation.
// ---------------------------------------------------------------------------
// Every scalar gets a value of its own; what the game frees or reads as a
// pointer, a model or a pet stays safe.
struct DirtyFiller
{
    int counter = 0;

    void Set(bool& value)
    {
        value = counter++ % 2 == 0;
    }
    void Set(wchar_t& value)
    {
        value = static_cast<wchar_t>(L'a' + counter++ % 26);
    }
    template <typename T> void Set(T& value)
    {
        if constexpr (std::is_enum_v<T>)
        {
            value = static_cast<T>(1);
            ++counter;
        }
        else if constexpr (std::is_pointer_v<T>)
        {
            value = nullptr;
            ++counter;
        }
        else if constexpr (std::is_floating_point_v<T>)
        {
            value = static_cast<T>(1000.25 + counter++);
        }
        else if constexpr (std::is_integral_v<T>)
        {
            value = static_cast<T>(40 + counter++ % 80);
        }
        else if constexpr (std::is_array_v<T>)
        {
            for (auto& element : value)
                Set(element);
        }
        else if constexpr (std::is_same_v<T, std::array<std::uint32_t, 4>>)
        {
            for (std::uint32_t& element : value)
                Set(element);
        }
        else if constexpr (std::is_same_v<T, OBJECT> || std::is_same_v<T, PART_t> || std::is_same_v<T, PATH_t>)
        {
            VisitFields(value, value, *this);
        }
        else
        {
            // Buff, CInterpolateContainer and the pet information stay as a
            // fresh slot has them.
            ++counter;
        }
    }

    template <typename T> void operator()(const char*, T& target, const T&)
    {
        Set(target);
    }
};

// A slot as a new character table has it: zeroed memory, then the
// constructor, which leaves some fields (HealthStatus, ShieldStatus) as the
// memory has them.
void ResetSlot(CHARACTER& character)
{
    delete[] character.Object.BoneTransform;
    character.Object.BoneTransform = nullptr;
    character.~CHARACTER();
    std::memset(static_cast<void*>(&character), 0, sizeof(CHARACTER));
    new (&character) CHARACTER();
}

void PrepareSlot(CHARACTER& character, CreationSlot slot, int key)
{
    ResetSlot(character);
    if (slot == CreationSlot::Clean)
        return;
    DirtyFiller filler;
    VisitFields(character, character, filler);
    VisitFields(character.Object, character.Object, filler);
    OBJECT& object = character.Object;
    object.Type = MODEL_MONSTER01;
    object.m_byNumCloth = 0;
    object.BoneTransform = nullptr;
    object.OwnsBoneTransform = false;
    for (PART_t* part :
         {&character.Wing, &character.Helper, &character.Flag, &character.Weapon[0], &character.Weapon[1]})
        part->m_byNumCloth = 0;
    for (PART_t& part : character.BodyPart)
        part.m_byNumCloth = 0;
    object.Live = slot == CreationSlot::SameKey;
    character.Key = static_cast<SHORT>(slot == CreationSlot::SameKey ? key : DirtyKey);
}

// ---------------------------------------------------------------------------
// The effect pools: cleared before, and the slots that came to life after.
// ---------------------------------------------------------------------------
// An object slot of a pool as fresh memory has it: zeroed, then the
// constructor. Assigned from OBJECT{}, it would hold what the temporary's
// stack held in the fields the constructor leaves (an effect's
// LastHorseWaveEffect and m_v3PrePos1, which CreateEffect does not set
// either), and two records of one creation would differ there.
void ResetObject(OBJECT& object)
{
    object.~OBJECT();
    std::memset(static_cast<void*>(&object), 0, sizeof(OBJECT));
    new (&object) OBJECT();
}

void ClearPools()
{
    for (int i = 0; i < MAX_EFFECTS; ++i)
        ResetObject(Effects[i]);
    for (int i = 0; i < MAX_SKILL_EFFECTS; ++i)
        ResetObject(*g_SkillEffects.GetEffect(i));
    for (int i = 0; i < MAX_SPRITES; ++i)
        ResetObject(Sprites[i]);
    for (int i = 0; i < MAX_PARTICLES; ++i)
        Particles[i] = PARTICLE{};
    for (int i = 0; i < MAX_JOINTS; ++i)
        Joints[i] = JOINT{};
    for (int i = 0; i < MAX_MOUNTS; ++i)
        ResetObject(Mounts[i]);
}

template <typename Element>
void RecordLiveSlots(Record& record, const char* pool, Element* slots, int count, bool clear)
{
    for (int i = 0; i < count; ++i)
    {
        if (!slots[i].Live)
            continue;
        Dump dump;
        AddStruct(dump, std::string(pool) + "[" + std::to_string(i) + "]", slots[i]);
        for (auto& [path, value] : dump)
            record.push_back({path, value});
        if (!clear)
            continue;
        if constexpr (std::is_same_v<Element, OBJECT>)
            ResetObject(slots[i]);
        else
            slots[i] = Element{};
    }
}

// The objects the terrain blocks hold before a creation: it adds behind
// them.
using BlockTails = std::array<OBJECT*, 256>;

BlockTails TakeBlockTails()
{
    BlockTails tails{};
    for (size_t i = 0; i < tails.size(); ++i)
        tails[i] = ObjectBlock[i].Tail;
    return tails;
}

// Records the objects a creation placed (CreateObject) and removes them.
void RecordObjects(Record& record, const BlockTails& before)
{
    for (size_t block = 0; block < before.size(); ++block)
    {
        OBJECT* object = before[block] != nullptr ? before[block]->Next : ObjectBlock[block].Head;
        for (int index = 0; object != nullptr; ++index)
        {
            OBJECT* next = object->Next;
            // What CreateObject and a creator set: a new OBJECT leaves other
            // fields as the heap has them (LastHorseWaveEffect), and where it
            // sits in its block's list is not the creation's choice.
            const std::string path = "objects[" + std::to_string(block) + "][" + std::to_string(index) + "].";
            Dump dump;
            Add(dump, path + "Type", object->Type);
            Add(dump, path + "SubType", object->SubType);
            Add(dump, path + "Position", object->Position);
            Add(dump, path + "Angle", object->Angle);
            Add(dump, path + "Scale", object->Scale);
            Add(dump, path + "Live", object->Live);
            Add(dump, path + "Visible", object->Visible);
            Add(dump, path + "LifeTime", object->LifeTime);
            Add(dump, path + "HiddenMesh", object->HiddenMesh);
            Add(dump, path + "BlendMesh", object->BlendMesh);
            Add(dump, path + "AlphaTarget", object->AlphaTarget);
            Add(dump, path + "LightEnable", object->LightEnable);
            for (auto& [field, value] : dump)
                record.push_back({field, value});
            DeleteObject(object, &ObjectBlock[block]);
            object = next;
        }
    }
}

void RecordPools(Record& record)
{
    RecordLiveSlots(record, "Effects", Effects, MAX_EFFECTS, true);
    RecordLiveSlots(record, "SkillEffects", g_SkillEffects.GetEffect(0), MAX_SKILL_EFFECTS, true);
    RecordLiveSlots(record, "Sprites", Sprites, MAX_SPRITES, true);
    RecordLiveSlots(record, "Particles", Particles, MAX_PARTICLES, true);
    RecordLiveSlots(record, "Joints", Joints, MAX_JOINTS, true);
    // A Dark Horse helper's horse (Q-MN57).
    RecordLiveSlots(record, "Mounts", Mounts, MAX_MOUNTS, true);
}

} // namespace

const char* Describe(CreationSlot slot)
{
    switch (slot)
    {
    case CreationSlot::Clean:
        return "clean slot";
    case CreationSlot::Dirty:
        return "dirty slot";
    case CreationSlot::SameKey:
        return "same-key slot";
    }
    return "";
}

CHARACTER& PrepareCreationSlot(CreationSlot slot, int key)
{
    // The characters after the hero start free; the creation takes the first.
    for (int i = CreatedSlot + 1; i < MAX_CHARACTERS_CLIENT; ++i)
    {
        if (CharactersClient[i].Object.Live)
            ResetSlot(CharactersClient[i]);
    }
    CHARACTER& target = CharactersClient[CreatedSlot];
    PrepareSlot(target, slot, key);
    BoneManager::UnregisterAll();
    return target;
}

Record RecordCreation(CreationScope& scope, int number, CreationSlot slot)
{
    return RecordCreation(scope, number, slot, RecordSeed);
}

Record RecordCreation(CreationScope& scope, int number, CreationSlot slot, unsigned seed)
{
    return RecordCreation(scope, number, slot, seed, CreationPath::CreateMonster);
}

Record RecordCreation(CreationScope& scope, int number, CreationSlot slot, unsigned seed, CreationPath path)
{
    // Every slot as new: a joint keeps fields of an earlier one in its slot
    // that its creation does not set (an eye trail's target position).
    ClearPools();
    CHARACTER& target = PrepareCreationSlot(slot, CreationScope::KeyOf(path));
    scope.Load().TakePlayedSounds();
    const Dump before = DumpCharacter(target);
    const BlockTails tails = TakeBlockTails();
    Random::SeedRand(seed);
    Random::Seed(seed);

    CHARACTER* created = scope.Create(number, path);

    Record record;
    record.push_back({"created", created == nullptr ? "null" : NameOf(created)});
    const Dump after = DumpCharacter(target);
    for (const auto& [path, value] : after)
    {
        const auto old = before.find(path);
        if (old == before.end() || old->second != value)
            record.push_back({path, value});
    }
    for (const auto& [name, bone] : BoneManager::ListBones(&target))
        record.push_back({"bone " + Core::Text::ToUtf8(name.c_str()), std::to_string(bone)});
    for (int i = CreatedSlot + 1; i < MAX_CHARACTERS_CLIENT; ++i)
    {
        if (CharactersClient[i].Object.Live)
        {
            record.push_back(
                {"characters[" + std::to_string(i) + "] created", std::to_string(CharactersClient[i].Object.Type)});
        }
    }
    RecordPools(record);
    RecordObjects(record, tails);
    const std::vector<PlayedSound> sounds = scope.Load().TakePlayedSounds();
    for (size_t i = 0; i < sounds.size(); ++i)
    {
        record.push_back(
            {"sound[" + std::to_string(i) + "]",
             std::to_string(sounds[i].id) + " at " + NameOf(sounds[i].object) + (sounds[i].looped ? " looped" : "")});
    }
    return record;
}

Record RecordVisualFrame(CreationScope& scope, CHARACTER& character, unsigned seed)
{
    ClearPools();
    scope.Load().TakePlayedSounds();
    Random::SeedRand(seed);
    Random::Seed(seed);
    character.Object.Visible = true;
    MoveCharacterVisual(&character, &character.Object);
    Record record;
    RecordPools(record);
    const std::vector<PlayedSound> sounds = scope.Load().TakePlayedSounds();
    for (size_t i = 0; i < sounds.size(); ++i)
        record.push_back({"sound[" + std::to_string(i) + "]", std::to_string(sounds[i].id)});
    return record;
}

std::string ToText(const Record& record)
{
    std::string text;
    for (const RecordedValue& value : record)
        text += value.path + " = " + value.value + "\n";
    return text;
}

} // namespace MonsterRecorder
