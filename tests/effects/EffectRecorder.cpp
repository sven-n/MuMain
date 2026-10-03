#include "stdafx.h"

#include "EffectRecorder.h"

#include "doctest.h"

#include "Core/Utilities/Random.h"
#include "Data/DataHandler/EffectData/EffectTypeStorage.h"
#include "Data/GameData/EffectData/EffectKind.h"
#include "Data/GameData/EffectData/EffectTypeCatalogue.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "GameLogic/Skills/SkillEffectMgr.h"
#include "Render/Effects/EffectRegistry.h"
#include "Render/Effects/ZzzEffect.h"
#include "Scenes/SceneCore.h"
#include "World/MapInfra/MapManager.h"

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <vector>

namespace EffectRecorder
{
namespace
{
constexpr unsigned RecordSeed = 20261003;
constexpr double PinnedWorldTime = 1234567.25;
constexpr int PinnedWorld = 0; // Lorencia
constexpr unsigned char HeroTileX = 102;
constexpr unsigned char HeroTileY = 98;
constexpr int MonsterModel = MODEL_PLAYER + 3; // any model number other than the player

// The second set of call arguments: uneven values that no case or default uses.
constexpr float UnevenScale = 1.75f;
constexpr short UnevenPkKey = 37;
constexpr std::uint16_t UnevenSkillIndex = 41;
constexpr std::uint16_t UnevenSkill = 43;
constexpr std::uint16_t UnevenSkillSerialNum = 47; // CreateEffect keeps it as a BYTE
constexpr short UnevenTargetIndex = 53;

// ---------------------------------------------------------------------------
// The named fields of the pool elements. Keep in step with OBJECT
// (w_ObjectInfo.h), PARTICLE and JOINT (_struct.h).
// ---------------------------------------------------------------------------
// clang-format off
#define RECORDER_OBJECT_FIELDS(F)                                                                                    \
    F(Live) F(bBillBoard) F(m_bCollisionCheck) F(m_bRenderShadow) F(EnableShadow) F(LightEnable) F(m_bActionStart)   \
    F(m_bRenderAfterCharacter) F(Visible) F(AlphaEnable) F(EnableBoneMatrix) F(ContrastEnable) F(ChromeEnable)       \
    F(AI) F(CurrentAction) F(PriorAction) F(ExtState) F(Teleport) F(Kind) F(Skill) F(m_byNumCloth)                   \
    F(m_byHurtByDeathstab) F(WeaponLevel) F(DamageTime) F(m_byBuildTime) F(m_bySkillCount) F(m_bySkillSerialNum)     \
    F(Block) F(m_pCloth) F(ScreenX) F(ScreenY) F(Weapon) F(Type) F(SubType) F(m_iAnimation) F(HiddenMesh)            \
    F(LifeTime) F(BlendMesh) F(AttackPoint) F(RenderType) F(InitialSceneTime) F(LinkBone) F(m_dwTime) F(Scale)       \
    F(BlendMeshLight) F(BlendMeshTexCoordU) F(BlendMeshTexCoordV) F(Timer) F(m_fEdgeScale) F(Velocity)               \
    F(CollisionRange) F(ShadowScale) F(Gravity) F(Distance) F(AnimationFrame) F(PriorAnimationFrame)                 \
    F(AlphaTarget) F(Alpha) F(LastHorseWaveEffect) F(PKKey) F(Light) F(Direction) F(m_vPosSword) F(StartPosition)    \
    F(BoundingBoxMin) F(BoundingBoxMax) F(m_vDownAngle) F(m_vDeadPosition) F(Position) F(Angle) F(HeadAngle)         \
    F(HeadTargetAngle) F(EyeLeft) F(EyeRight) F(EyeLeft2) F(EyeRight2) F(EyeLeft3) F(EyeRight3) F(Matrix)            \
    F(BoneTransform) F(OBB.StartPos) F(OBB.XAxis) F(OBB.YAxis) F(OBB.ZAxis) F(Owner) F(Prior) F(Next) F(m_BuffMap)   \
    F(m_sTargetIndex) F(m_bpcroom) F(m_v3PrePos1) F(m_v3PrePos2) F(m_Interpolates)

#define RECORDER_PARTICLE_FIELDS(F)                                                                                  \
    F(Live) F(Type) F(TexType) F(SubType) F(Scale) F(Position) F(Angle) F(Light) F(Alpha) F(LifeTime) F(Target)      \
    F(Rotation) F(Frame) F(bEnableMove) F(Gravity) F(Velocity) F(TurningForce) F(StartPosition) F(iNumBone)          \
    F(bRepeatedly) F(fRepeatedlyHeight)

#define RECORDER_JOINT_FIELDS(F)                                                                                     \
    F(Live) F(Type) F(TexType) F(SubType) F(RenderType) F(RenderFace) F(Scale) F(Position) F(StartPosition)          \
    F(Angle) F(HeadAngle) F(Light) F(Target) F(TargetPosition) F(byOnlyOneRender) F(LifeTime) F(Collision)           \
    F(Velocity) F(Direction) F(PKKey) F(Skill) F(Weapon) F(MultiUse) F(bTileMapping) F(m_byReverseUV)                \
    F(TargetIndex) F(m_bySkillSerialNum) F(m_iChaIndex) F(m_sTargetIndex) F(m_bCreateTails) F(NumTails)              \
    F(MaxTails) F(Tails)
// clang-format on

// Calls visitor(name, field of `target`, same field of `reference`) for every field.
template <typename Visitor> void VisitFields(OBJECT& target, const OBJECT& reference, Visitor&& visitor)
{
#define RECORDER_VISIT(name) visitor(#name, target.name, reference.name);
    RECORDER_OBJECT_FIELDS(RECORDER_VISIT)
#undef RECORDER_VISIT
}

template <typename Visitor> void VisitFields(PARTICLE& target, const PARTICLE& reference, Visitor&& visitor)
{
#define RECORDER_VISIT(name) visitor(#name, target.name, reference.name);
    RECORDER_PARTICLE_FIELDS(RECORDER_VISIT)
#undef RECORDER_VISIT
}

template <typename Visitor> void VisitFields(JOINT& target, const JOINT& reference, Visitor&& visitor)
{
#define RECORDER_VISIT(name) visitor(#name, target.name, reference.name);
    RECORDER_JOINT_FIELDS(RECORDER_VISIT)
#undef RECORDER_VISIT
}

// ---------------------------------------------------------------------------
// The world the recorder pins: owners, pattern targets, names of pointers.
// ---------------------------------------------------------------------------
using Bones = std::unique_ptr<vec34_t[]>;

struct World
{
    std::unique_ptr<CHARACTER[]> characters{new CHARACTER[MAX_CHARACTERS_CLIENT + 1]{}};
    OBJECT patternObject; // what Owner/Target point at in pattern B
    Bones patternBones{new vec34_t[MAX_BONES]{}};
    Bones heroBones{new vec34_t[MAX_BONES]{}};
    Bones monsterBones{new vec34_t[MAX_BONES]{}};
    OBJECT heroBefore;
    OBJECT monsterBefore;

    CHARACTER* Hero()
    {
        return &characters[0];
    }
    CHARACTER* Monster()
    {
        return &characters[1];
    }
};

void FillBones(Bones& bones, float base)
{
    float value = base;
    for (int i = 0; i < MAX_BONES; ++i)
    {
        for (auto& row : bones[i])
        {
            for (float& element : row)
            {
                element = value;
                value += 0.125f;
            }
        }
    }
}

// The owners are set up with the game's own CreateCharacterPointer; it leaves
// BoneTransform uninitialised (new vec34_t[]), so the recorder points it at
// bones with fixed values.
void SetUpCharacter(CHARACTER* character, int model, unsigned char tileX, Bones& bones, float boneBase)
{
    CreateCharacterPointer(character, model, tileX, HeroTileY, 45.f);
    delete[] character->Object.BoneTransform;
    FillBones(bones, boneBase);
    character->Object.BoneTransform = bones.get();
}

World& GetWorld()
{
    static World world = []
    {
        World created;
        CHARACTER* previousCharacters = CharactersClient;
        CHARACTER* previousHero = Hero;
        CharactersClient = created.characters.get();
        Hero = created.Hero();
        SetUpCharacter(created.Hero(), MODEL_PLAYER, HeroTileX, created.heroBones, 1.f);
        SetUpCharacter(created.Monster(), MonsterModel, HeroTileX + 2, created.monsterBones, -1.f);
        FillBones(created.patternBones, 500.f);
        created.heroBefore = created.Hero()->Object;
        created.monsterBefore = created.Monster()->Object;
        CharactersClient = previousCharacters;
        Hero = previousHero;
        return created;
    }();
    return world;
}

struct PoolRange
{
    const char* name;
    const OBJECT* objects = nullptr;
    const PARTICLE* particles = nullptr;
    const JOINT* joints = nullptr;
    int size = 0;
};

std::string NameOfPointer(const void* pointer)
{
    if (pointer == nullptr)
        return "null";
    World& world = GetWorld();
    if (pointer == &world.Hero()->Object)
        return "hero";
    if (pointer == &world.Monster()->Object)
        return "monster";
    if (pointer == &world.patternObject)
        return "pattern";
    if (pointer == world.patternBones.get())
        return "patternBones";
    if (pointer == world.heroBones.get())
        return "heroBones";
    if (pointer == world.monsterBones.get())
        return "monsterBones";
    const PoolRange pools[] = {
        {"Effects", Effects, nullptr, nullptr, MAX_EFFECTS},
        {"SkillEffects", g_SkillEffects.GetEffect(0), nullptr, nullptr, MAX_SKILL_EFFECTS},
        {"Sprites", Sprites, nullptr, nullptr, MAX_SPRITES},
    };
    for (const PoolRange& pool : pools)
    {
        const auto* object = static_cast<const OBJECT*>(pointer);
        if (object >= pool.objects && object < pool.objects + pool.size)
            return std::string(pool.name) + "[" + std::to_string(object - pool.objects) + "]";
    }
    return "unknown";
}

// ---------------------------------------------------------------------------
// Pattern A and B: every scalar of a slot gets its own value, so a case that
// copies one old field into another shows which one.
// ---------------------------------------------------------------------------
struct PatternFiller
{
    SlotPattern pattern;
    int counter = 0;

    bool IsA() const
    {
        return pattern == SlotPattern::A;
    }
    int Next()
    {
        return counter++;
    }

    void Set(bool& value)
    {
        value = !IsA();
        Next();
    }
    void Set(unsigned char& value)
    {
        value = static_cast<unsigned char>((IsA() ? 10 : 140) + Next() % 100);
    }
    void Set(unsigned short& value)
    {
        value = static_cast<unsigned short>((IsA() ? 1000 : 3000) + Next());
    }
    void Set(short& value)
    {
        value = static_cast<short>(IsA() ? 1000 + Next() : -3000 - Next());
    }
    void Set(int& value)
    {
        value = IsA() ? 100000 + Next() : -200000 - Next();
    }
    void Set(unsigned int& value)
    {
        value = (IsA() ? 0xA0000000u : 0xB0000000u) + static_cast<unsigned>(Next());
    }
    void Set(unsigned long& value)
    {
        value = (IsA() ? 0xA0000000ul : 0xB0000000ul) + static_cast<unsigned long>(Next());
    }
    void Set(float& value)
    {
        value = IsA() ? 1000.25f + static_cast<float>(Next()) : -2000.5f - static_cast<float>(Next());
    }
    void Set(OBJECT*& value)
    {
        value = IsA() ? nullptr : &GetWorld().patternObject;
        Next();
    }
    void Set(vec34_t*& value)
    {
        value = IsA() ? nullptr : GetWorld().patternBones.get();
        Next();
    }
    void Set(void*& value)
    {
        value = nullptr;
        Next();
    } // m_pCloth: cloth code would free it
    void Set(Buff& value)
    {
        value.m_Buff.clear();
        if (!IsA())
            value.m_Buff[eBuff_Attack] = 7;
        Next();
    }
    void Set(CInterpolateContainer& value)
    {
        value.ClearContainer();
        if (!IsA())
        {
            const vec3_t start{1.f, 2.f, 3.f};
            const vec3_t end{4.f, 5.f, 6.f};
            value.m_vecInterpolatesAngle.emplace_back(0.f, 0.5f, start, end);
            value.m_vecInterpolatesPos.emplace_back(0.f, 0.5f, start, end);
            value.m_vecInterpolatesScale.emplace_back(0.f, 0.5f, 1.f, 2.f);
            value.m_vecInterpolatesAlpha.emplace_back(0.f, 0.5f, 1.f, 0.f);
        }
        Next();
    }
    template <typename T, size_t N> void Set(T (&values)[N])
    {
        for (T& value : values)
            Set(value);
    }

    template <typename T> void operator()(const char*, T& target, const T&)
    {
        Set(target);
    }
};

template <typename Element> void FillSlot(Element& slot, SlotPattern pattern)
{
    PatternFiller filler{pattern};
    VisitFields(slot, slot, filler);
    slot.Live = false; // every slot stays free
}

// ---------------------------------------------------------------------------
// Formatting and comparing values.
// ---------------------------------------------------------------------------
std::string Format(bool value)
{
    return value ? "true" : "false";
}
std::string Format(unsigned char value)
{
    return std::to_string(value);
}
std::string Format(unsigned short value)
{
    return std::to_string(value);
}
std::string Format(short value)
{
    return std::to_string(value);
}
std::string Format(int value)
{
    return std::to_string(value);
}
std::string Format(unsigned int value)
{
    return std::to_string(value);
}
std::string Format(unsigned long value)
{
    return std::to_string(value);
}
std::string Format(float value)
{
    char text[32];
    std::snprintf(text, sizeof(text), "%.9g", value);
    return text;
}
std::string Format(const OBJECT* value)
{
    return NameOfPointer(value);
}
std::string Format(const vec34_t* value)
{
    return NameOfPointer(value);
}
std::string Format(const void* value)
{
    return NameOfPointer(value);
}
std::string Format(const Buff& value)
{
    std::string text = "{";
    for (const auto& [state, count] : value.m_Buff)
        text += std::to_string(static_cast<int>(state)) + ":" + std::to_string(count) + " ";
    return text + "}";
}
std::string FormatFactors(const CInterpolateContainer::VEC_INTERPOLATES& factors)
{
    std::string text = std::to_string(factors.size());
    for (const auto& factor : factors)
    {
        text += " [" + Format(factor.fRateStart) + " " + Format(factor.fRateEnd);
        for (int i = 0; i < 3; ++i)
            text += " " + Format(factor.v3Start[i]);
        for (int i = 0; i < 3; ++i)
            text += " " + Format(factor.v3End[i]);
        text += "]";
    }
    return text;
}
std::string FormatFactors(const CInterpolateContainer::VEC_INTERPOLATES_F& factors)
{
    std::string text = std::to_string(factors.size());
    for (const auto& factor : factors)
        text += " [" + Format(factor.fRateStart) + " " + Format(factor.fRateEnd) + " " + Format(factor.fStart) + " " +
                Format(factor.fEnd) + "]";
    return text;
}
std::string Format(const CInterpolateContainer& value)
{
    return "angle " + FormatFactors(value.m_vecInterpolatesAngle) + "; pos " +
           FormatFactors(value.m_vecInterpolatesPos) + "; scale " + FormatFactors(value.m_vecInterpolatesScale) +
           "; alpha " + FormatFactors(value.m_vecInterpolatesAlpha);
}

bool Same(float left, float right)
{
    return std::bit_cast<std::uint32_t>(left) == std::bit_cast<std::uint32_t>(right);
}
template <typename T> bool Same(const T& left, const T& right)
{
    return left == right;
}
bool Same(const Buff& left, const Buff& right)
{
    return left.m_Buff == right.m_Buff;
}
template <typename Factor> bool SameFactors(const std::vector<Factor>& left, const std::vector<Factor>& right)
{
    return left.size() == right.size() &&
           (left.empty() || std::memcmp(left.data(), right.data(), left.size() * sizeof(Factor)) == 0);
}
bool Same(const CInterpolateContainer& left, const CInterpolateContainer& right)
{
    return SameFactors(left.m_vecInterpolatesAngle, right.m_vecInterpolatesAngle) &&
           SameFactors(left.m_vecInterpolatesPos, right.m_vecInterpolatesPos) &&
           SameFactors(left.m_vecInterpolatesScale, right.m_vecInterpolatesScale) &&
           SameFactors(left.m_vecInterpolatesAlpha, right.m_vecInterpolatesAlpha);
}

// The tails of a joint are 200 x 4 points; they are recorded as one hash.
using JointTails = vec3_t[MAX_TAILS][4];
std::uint64_t Hash(const JointTails& tails)
{
    std::uint64_t hash = 1469598103934665603ull;
    const auto* bytes = reinterpret_cast<const unsigned char*>(&tails);
    for (size_t i = 0; i < sizeof(JointTails); ++i)
        hash = (hash ^ bytes[i]) * 1099511628211ull;
    return hash;
}

struct Comparer
{
    Record& record;
    const char* slotName; // "Effects", "hero"
    int slotIndex;        // -1 for a single object

    std::string Path(std::string_view name) const
    {
        std::string path = slotName;
        if (slotIndex >= 0)
            path += "[" + std::to_string(slotIndex) + "]";
        return path + "." + std::string(name);
    }

    template <typename T> static bool AllSame(const T& after, const T& before)
    {
        if constexpr (std::is_array_v<T>)
        {
            for (size_t i = 0; i < std::extent_v<T>; ++i)
            {
                if (!AllSame(after[i], before[i]))
                    return false;
            }
            return true;
        }
        else
            return Same(after, before);
    }

    // Called only once a difference is known, so building names costs nothing for unchanged slots.
    template <typename T> void AddChanged(const std::string& path, const T& after, const T& before)
    {
        if constexpr (std::is_array_v<T>)
        {
            for (size_t i = 0; i < std::extent_v<T>; ++i)
            {
                if (!AllSame(after[i], before[i]))
                    AddChanged(path + "[" + std::to_string(i) + "]", after[i], before[i]);
            }
        }
        else
            record.push_back({path, Format(after)});
    }

    template <typename T> void operator()(const char* name, T& after, const T& before)
    {
        if (!AllSame(after, before))
            AddChanged(Path(name), after, before);
    }

    void operator()(const char* name, JointTails& after, const JointTails& before)
    {
        if (std::memcmp(after, before, sizeof(JointTails)) != 0)
            record.push_back({Path(name), "hash " + std::to_string(Hash(after))});
    }
};

template <typename Element>
void CompareSlot(Record& record, const char* slotName, int slotIndex, Element& after, const Element& before)
{
    Comparer comparer{record, slotName, slotIndex};
    VisitFields(after, before, comparer);
}

// ---------------------------------------------------------------------------
// The pools before a call: filled once per pattern, copied back before each call.
// ---------------------------------------------------------------------------
struct PoolImage
{
    std::vector<OBJECT> effects = std::vector<OBJECT>(MAX_EFFECTS);
    std::vector<OBJECT> skillEffects = std::vector<OBJECT>(MAX_SKILL_EFFECTS);
    std::vector<OBJECT> sprites = std::vector<OBJECT>(MAX_SPRITES);
    std::vector<PARTICLE> particles = std::vector<PARTICLE>(MAX_PARTICLES);
    std::vector<JOINT> joints = std::vector<JOINT>(MAX_JOINTS);
};

std::unique_ptr<PoolImage> MakeImage(SlotPattern pattern)
{
    auto image = std::make_unique<PoolImage>();
    for (OBJECT& slot : image->effects)
        FillSlot(slot, pattern);
    for (OBJECT& slot : image->skillEffects)
        FillSlot(slot, pattern);
    for (OBJECT& slot : image->sprites)
        FillSlot(slot, pattern);
    for (PARTICLE& slot : image->particles)
        FillSlot(slot, pattern);
    for (JOINT& slot : image->joints)
        FillSlot(slot, pattern);
    return image;
}

const PoolImage& GetImage(SlotPattern pattern)
{
    static const std::unique_ptr<PoolImage> a = MakeImage(SlotPattern::A);
    static const std::unique_ptr<PoolImage> b = MakeImage(SlotPattern::B);
    return pattern == SlotPattern::A ? *a : *b;
}

void RestorePools(const PoolImage& image)
{
    std::copy(image.effects.begin(), image.effects.end(), Effects);
    for (int i = 0; i < MAX_SKILL_EFFECTS; ++i)
        *g_SkillEffects.GetEffect(i) = image.skillEffects[i];
    std::copy(image.sprites.begin(), image.sprites.end(), Sprites);
    std::memcpy(Particles, image.particles.data(), sizeof(PARTICLE) * MAX_PARTICLES);
    std::memcpy(Joints, image.joints.data(), sizeof(JOINT) * MAX_JOINTS);
}

void ComparePools(Record& record, const PoolImage& image)
{
    for (int i = 0; i < MAX_EFFECTS; ++i)
        CompareSlot(record, "Effects", i, Effects[i], image.effects[i]);
    for (int i = 0; i < MAX_SKILL_EFFECTS; ++i)
        CompareSlot(record, "SkillEffects", i, *g_SkillEffects.GetEffect(i), image.skillEffects[i]);
    for (int i = 0; i < MAX_SPRITES; ++i)
        CompareSlot(record, "Sprites", i, Sprites[i], image.sprites[i]);
    for (int i = 0; i < MAX_PARTICLES; ++i)
    {
        if (std::memcmp(&Particles[i], &image.particles[i], sizeof(PARTICLE)) != 0)
            CompareSlot(record, "Particles", i, Particles[i], image.particles[i]);
    }
    for (int i = 0; i < MAX_JOINTS; ++i)
    {
        if (std::memcmp(&Joints[i], &image.joints[i], sizeof(JOINT)) != 0)
            CompareSlot(record, "Joints", i, Joints[i], image.joints[i]);
    }
}

// Pins the globals creation reads and restores them afterwards.
class PinnedGlobals
{
public:
    explicit PinnedGlobals(float frameFactor)
        : m_frameFactor(FPS_ANIMATION_FACTOR), m_worldTime(WorldTime), m_scene(SceneFlag),
          m_world(gMapManager.WorldActive), m_hero(Hero), m_characters(CharactersClient)
    {
        World& world = GetWorld();
        FPS_ANIMATION_FACTOR = frameFactor;
        WorldTime = PinnedWorldTime;
        SceneFlag = MAIN_SCENE;
        gMapManager.WorldActive = PinnedWorld;
        CharactersClient = world.characters.get();
        Hero = world.Hero();
    }
    ~PinnedGlobals()
    {
        FPS_ANIMATION_FACTOR = m_frameFactor;
        WorldTime = m_worldTime;
        SceneFlag = m_scene;
        gMapManager.WorldActive = m_world;
        Hero = m_hero;
        CharactersClient = m_characters;
    }
    PinnedGlobals(const PinnedGlobals&) = delete;
    PinnedGlobals& operator=(const PinnedGlobals&) = delete;

private:
    float m_frameFactor;
    double m_worldTime;
    EGameScene m_scene;
    int m_world;
    CHARACTER* m_hero;
    CHARACTER* m_characters;
};

OBJECT* OwnerObject(Owner owner)
{
    switch (owner)
    {
    case Owner::Hero:
        return &GetWorld().Hero()->Object;
    case Owner::Monster:
        return &GetWorld().Monster()->Object;
    case Owner::None:
        break;
    }
    return nullptr;
}

const char* OwnerName(Owner owner)
{
    switch (owner)
    {
    case Owner::Hero:
        return "hero";
    case Owner::Monster:
        return "monster";
    case Owner::None:
        break;
    }
    return "none";
}

void CompareArray(Record& record, const char* name, const vec3_t after, const std::array<float, 3>& before)
{
    for (int i = 0; i < 3; ++i)
    {
        if (!Same(after[i], before[i]))
            record.push_back({std::string(name) + "[" + std::to_string(i) + "]", Format(after[i])});
    }
}

// How many values a generator gave since it was seeded: the next two values
// after the call are searched for in the seeded sequence; "more than N" when
// the call drew more than the limit. The same draws give the same count on
// every platform, but rand() and the distributions give other values per
// standard library, so a case whose number of draws depends on a drawn value
// can count differently elsewhere.
constexpr int DrawLimit = 100000;

template <typename Reseed, typename Draw> std::string CountDraws(Reseed reseed, Draw draw)
{
    const auto first = draw();
    const auto second = draw();
    reseed();
    auto previous = draw();
    for (int count = 0; count < DrawLimit; ++count)
    {
        const auto current = draw();
        if (previous == first && current == second)
            return std::to_string(count);
        previous = current;
    }
    return "more than " + std::to_string(DrawLimit);
}

std::string CountRandDraws()
{
    return CountDraws([] { std::srand(RecordSeed); }, [] { return std::rand(); });
}

// RangeInt over 2^31 values takes exactly one value of the 32-bit engine on
// the standard libraries we build with, so this counts engine values
// (Random::UnitDouble and FpsCheck take two).
std::string CountRandomDraws()
{
    return CountDraws([] { Random::Seed(RecordSeed); }, [] { return Random::RangeInt(0, INT32_MAX); });
}

const Data::Effects::EffectTypesLoadResult& ShippedTypes()
{
    static const Data::Effects::EffectTypesLoadResult result =
        Data::Effects::LoadEffectTypeFiles(std::filesystem::path(MU_TEST_DATA_DIR) / "Effects");
    return result;
}
} // namespace

std::span<const Conditions> AllConditions()
{
    static const Conditions all[] = {
        {1.f, SlotPattern::A},
        {1.f, SlotPattern::B},
        {0.5f, SlotPattern::A},
        {0.5f, SlotPattern::B},
    };
    return all;
}

std::vector<EffectCall> CallsFor(int type, std::initializer_list<int> subTypes)
{
    std::vector<EffectCall> calls;
    for (int subType : subTypes)
    {
        for (Owner owner : {Owner::None, Owner::Hero, Owner::Monster})
        {
            EffectCall call;
            call.type = type;
            call.subType = subType;
            call.owner = owner;
            calls.push_back(call);

            call.scale = UnevenScale;
            call.pkKey = UnevenPkKey;
            call.skillIndex = UnevenSkillIndex;
            call.skill = UnevenSkill;
            call.skillSerialNum = UnevenSkillSerialNum;
            call.targetIndex = UnevenTargetIndex;
            calls.push_back(call);
        }
    }
    return calls;
}

Record RecordCall(const EffectCall& call, const Conditions& conditions)
{
    PinnedGlobals pinned(conditions.frameFactor);
    World& world = GetWorld();
    const PoolImage& image = GetImage(conditions.pattern);
    RestorePools(image);
    world.Hero()->Object = world.heroBefore;
    world.Monster()->Object = world.monsterBefore;

    vec3_t position{call.position[0], call.position[1], call.position[2]};
    vec3_t angle{call.angle[0], call.angle[1], call.angle[2]};
    vec3_t light{call.light[0], call.light[1], call.light[2]};
    std::srand(RecordSeed);
    Random::Seed(RecordSeed);
    CreateEffect(call.type, position, angle, light, call.subType, OwnerObject(call.owner), call.pkKey, call.skillIndex,
                 call.skill, call.skillSerialNum, call.scale, call.targetIndex);

    Record record;
    record.push_back({"draws.rand", CountRandDraws()});
    record.push_back({"draws.Random", CountRandomDraws()});
    CompareArray(record, "call.Position", position, call.position);
    CompareArray(record, "call.Angle", angle, call.angle);
    CompareArray(record, "call.Light", light, call.light);
    ComparePools(record, image);
    CompareSlot(record, "hero", -1, world.Hero()->Object, world.heroBefore);
    CompareSlot(record, "monster", -1, world.Monster()->Object, world.monsterBefore);
    return record;
}

std::vector<Difference> Compare(const Record& expected, const Record& actual)
{
    std::map<std::string, std::pair<std::string, std::string>> values;
    for (const RecordedValue& value : expected)
        values[value.path] = {value.value, "unchanged"};
    for (const RecordedValue& value : actual)
    {
        auto [it, inserted] = values.try_emplace(value.path, "unchanged", value.value);
        if (!inserted)
            it->second.second = value.value;
    }
    std::vector<Difference> differences;
    for (const auto& [path, pair] : values)
    {
        if (pair.first != pair.second)
            differences.push_back({path, pair.first, pair.second});
    }
    return differences;
}

std::optional<std::string> Find(const Record& record, std::string_view path)
{
    for (const RecordedValue& value : record)
    {
        if (value.path == path)
            return value.value;
    }
    return std::nullopt;
}

std::string Describe(const EffectCall& call, const Conditions& conditions)
{
    return "type " + std::to_string(call.type) + " subType " + std::to_string(call.subType) + " owner " +
           OwnerName(call.owner) + " scale " + Format(call.scale) + " pkKey " + std::to_string(call.pkKey) + " skill " +
           std::to_string(call.skillIndex) + "/" + std::to_string(call.skill) + "/" +
           std::to_string(call.skillSerialNum) + " target " + std::to_string(call.targetIndex) + " frameFactor " +
           Format(conditions.frameFactor) + " pattern " + (conditions.pattern == SlotPattern::A ? "A" : "B");
}

std::string ToText(const Record& record)
{
    std::string text;
    for (const RecordedValue& value : record)
        text += value.path + " = " + value.value + "\n";
    return text;
}

std::string ToText(const std::vector<Difference>& differences)
{
    std::string text;
    for (const Difference& difference : differences)
        text += difference.path + ": " + difference.expected + " -> " + difference.actual + "\n";
    return text;
}

void BuildShippedRegistry(std::span<const int> withoutCreationValuesOf,
                          std::span<const Data::Effects::EffectTypeCreateParams> extraRows)
{
    Data::Effects::EffectTypeCatalogue catalogue;
    catalogue.Build(Data::Effects::EffectKind::Effect,
                    ShippedTypes().types[Data::Effects::ToIndex(Data::Effects::EffectKind::Effect)]);
    std::vector<Data::Effects::EffectTypeCreateParams> rows;
    std::vector<int> left;
    for (const Data::Effects::EffectTypeCreateParams& row : catalogue.GetCreateParams())
    {
        const bool excluded = std::find(withoutCreationValuesOf.begin(), withoutCreationValuesOf.end(), row.type) !=
                              withoutCreationValuesOf.end();
        if (excluded)
            left.push_back(row.type);
        else
            rows.push_back(row);
    }
    // A type without a row would run its legacy case on both sides of a
    // comparison, which then proves nothing.
    for (const int type : withoutCreationValuesOf)
    {
        const bool hadRow = std::find(left.begin(), left.end(), type) != left.end();
        REQUIRE_MESSAGE(hadRow, "type " << type << " has no creation values in the catalogue");
    }
    rows.insert(rows.end(), extraRows.begin(), extraRows.end());
    Render::Effects::BuildRegistry(rows);
}
} // namespace EffectRecorder
