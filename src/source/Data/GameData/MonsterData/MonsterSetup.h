#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <string_view>
#include <utility>
#include <string>
#include <string_view>
#include <vector>

// What a monster's character gets when it appears (monsters design, D63, MN4):
// its look, its equipment, what it does at spawn and how it behaves. Every
// field is optional: one the entry does not name stays as the game sets a
// character up (CreateCharacterPointer).
namespace Data::Monsters
{
// The ranges the monster list's reader takes, which the editor keeps to: an
// item's level and excellent flags, the class step, the tile offset, a spawn
// effect's count and random scale, and a facing tile.
inline constexpr int MaxItemLevel = 15;
inline constexpr int MaxExcellentFlags = 255;
inline constexpr int MaxClassStep = 2;
inline constexpr int MaxTileOffset = 16;
// A look level and a move speed go into the character's 16-bit fields.
inline constexpr int MaxLookLevel = 65535;
inline constexpr int MaxMoveSpeed = 65535;
inline constexpr int MaxSpawnEffectCount = 100;
inline constexpr int MaxSpawnEffectScaleRandom = 10000;
inline constexpr int MaxFacingTile = 255;
// An eyes row's bones: the right and left eye, then up to two more pairs.
inline constexpr std::size_t MaxEyeBones = 6;
constexpr bool IsEyeBoneCount(std::size_t count)
{
    return count >= 2 && count <= MaxEyeBones && count % 2 == 0;
}

// An item a monster wears, as the item list names it.
struct MonsterItem
{
    int group = 0;
    int number = 0;
    int level = 0;
    int excellent = 0;
    // A body that is no item (D63), by its slot: a part model of the
    // monster's own model ("MODEL_MERCHANT_MAN_UPPER+1") or a player body
    // ("MODEL_BODY_HELM+15", the Doppelganger Magic Gladiator's bare head).
    // Empty for an item.
    std::string part;
    // A Dark Horse helper's horse look (Q-MN57): "doppelganger" (the mount's
    // SubType 1); empty for the normal one.
    std::string mountLook;

    bool operator==(const MonsterItem&) const = default;
};

// The slot of a body that is no item, by its symbol: a slot of the monster or
// NPC range, or a player body ("MODEL_BODY_ARMOR+3"); nullopt for another
// text. And the symbol of such a slot; empty for another slot.
std::optional<int> FindMonsterBodyPartSlot(std::string_view symbol);
std::string GetMonsterBodyPartSymbol(int slot);
// The horse looks a Dark Horse helper may name, and the mount's SubType of one.
std::optional<int> FindMountLook(std::string_view name);
// Whether the item is the Dark Horse, the helper that brings a horse (Q-MN57).
bool IsDarkHorse(const MonsterItem& item);

struct MonsterEquipment
{
    // Weapon[0] and Weapon[1]; nullopt: the slot stays empty.
    std::array<std::optional<MonsterItem>, 2> weapons;
    std::optional<MonsterItem> helm;
    std::optional<MonsterItem> armor;
    std::optional<MonsterItem> pants;
    std::optional<MonsterItem> gloves;
    std::optional<MonsterItem> boots;
    std::optional<MonsterItem> wings;
    std::optional<MonsterItem> helper;

    bool operator==(const MonsterEquipment&) const = default;
    bool IsEmpty() const;
};

// Something a monster brings when it appears, in its order (Q-MN36a): a joint
// on it (a trail of light, CreateJoint), an effect on it (CreateEffect), a
// particle (CreateParticle), an object beside it (CreateObject), or its eyes
// put on bones (MoveEye), which the eye trails after it follow.
struct MonsterSpawnEffect
{
    enum class Kind
    {
        Joint,
        Object,
        Effect,
        Eyes,
        Particle,
    };
    Kind kind = Kind::Joint;
    // The joint's, effect's or particle's type ("BITMAP_JOINT_ENERGY",
    // "MODEL_AURORA") or the object's model ("MODEL_BC_BOX"), as the code
    // names them.
    std::string type;
    // A joint's, effect's or particle's sub type; a joint's or particle's
    // scale (0: a particle's own, 1).
    int subType = 0;
    float scale = 0.0f;
    // A joint's colour, an effect's or particle's light; nullopt: the joint's
    // own, the monster's light.
    std::optional<std::array<float, 3>> color;
    // An effect's or joint's key (their PKKey: the Life Stone's aurora 120,
    // Daesuler's four rings 0 to 3).
    int key = -1;
    // The eyes' bones in MoveEye's order: right, left, then up to two more
    // pairs.
    std::vector<int> bones;
    // An object's facing beside the monster (Angle[2]); nullopt: the
    // monster's (Angela's supply box at 90).
    std::optional<float> angle;
    // A joint, effect or particle made at this bone of the monster, without
    // the monster as its owner: it waits for the monster's first frame on
    // screen, when the bone has a pose (the Dark Elf 412's crater, Q-MN38c).
    std::optional<int> bone;
    // How many of the joint, effect or particle.
    int count = 1;
    // A joint's, effect's or particle's place beside the monster's (x, y, z),
    // and a joint's end beside its place (a bolt from the ground up).
    std::optional<std::array<float, 3>> offset;
    std::optional<std::array<float, 3>> targetOffset;
    // A random part of the scale: Random::Rand() % scaleRandom more.
    int scaleRandom = 0;

    bool operator==(const MonsterSpawnEffect&) const = default;
};

// How a monster comes in that the server has just spawned (0x13 and 0x1F with
// their new flag), or that teleported (0x13's teleport flag): what
// the packets did by number before (Q-MN38c). Of an appear, a field
// it leaves out is the model's way: a player-shaped monster plays
// PLAYER_COME_UP and shows at once, any other fades in. A teleport does only
// what it names.
struct MonsterAppear
{
    // The animation it plays (SetAction), by the attacks' names ("stop2",
    // "appear").
    std::optional<std::string> action;
    // The first frame of that animation blends from the one before (the Maya
    // hands); else the animation starts alone (PriorAction).
    bool blend = false;
    // Object.AnimationFrame and PriorAnimationFrame (the Kalima gates' 0).
    std::optional<float> startFrame;
    // Object.Alpha 0, rising to its AlphaTarget, or 1 at once.
    std::optional<bool> fadeIn;
    // CHARACTER::Appear: the frames of the golden monsters' sparkle (60).
    std::optional<int> appearTime;
    // The sound it plays, by its SOUND_ name.
    std::optional<std::string> sound;
    // Joints, effects and particles made where it comes in, in the world: not
    // on the monster.
    std::vector<MonsterSpawnEffect> effects;

    bool operator==(const MonsterAppear&) const = default;
};

struct MonsterStateVariant;

// A castle gate, opened and closed by its buff (the Castle Siege's gate rule,
// Q-MN38c): the animations it plays closed, opening and open, by the attacks'
// names, and whether an open gate that comes into view plays its opening (as
// the old code did) or stands open. The gate switch, 0x13, 0x68 and the end
// of its animations read them.
struct MonsterGate
{
    std::string closed = "stop1";
    std::string opening = "stop2";
    std::string open = "walk";
    bool arrivalOpening = true;

    bool operator==(const MonsterGate&) const = default;
};

// A tile where a monster that arrives (0x13) turns by another step
// (Q-MN38b, the Ballista): one of its two coordinates, and the step.
struct MonsterTileFacing
{
    std::optional<int> tileX;
    std::optional<int> tileY;
    float step = 0.0f;

    bool operator==(const MonsterTileFacing&) const = default;
};

// Code of the maps that runs when the monster appears, by its name, with the
// values the code fixed (Q-MN36d): the hand-overs to a map's own state.
struct MonsterHook
{
    // One of GetMonsterHookKinds.
    std::string name;
    // Its values by their key ("boss": "Raymond").
    std::vector<std::pair<std::string, std::string>> values;

    bool operator==(const MonsterHook&) const = default;
    // The value of the key; empty when it has none.
    std::string_view Value(std::string_view key) const;
};

// A hook the maps run, and the value it takes, if any, with the values it
// knows.
struct MonsterHookKind
{
    std::string_view name;
    std::string_view valueKey;
    std::span<const std::string_view> values;
};

std::span<const MonsterHookKind> GetMonsterHookKinds();
const MonsterHookKind* FindMonsterHookKind(std::string_view name);

struct MonsterSetup
{
    // Object.Scale; or the scale a player has with the monster's equipment
    // (SetCharacterScale).
    std::optional<float> scale;
    bool scaleByCharacter = false;
    // m_fEdgeScale.
    std::optional<float> outline;
    // Object.SubType, the skin.
    std::optional<int> subType;
    // CHARACTER::Level, the render tier.
    std::optional<int> lookLevel;
    std::optional<int> glowMesh;
    std::optional<float> glowLight;
    std::optional<int> hiddenMesh;
    // m_bRenderShadow.
    std::optional<bool> shadow;
    // CHARACTER::HideShadow.
    std::optional<bool> hideShadow;
    std::optional<float> alphaTarget;
    // Object.LightEnable.
    std::optional<bool> light;
    // CHARACTER::PK: the colour of a player of that standing (PVP_ value).
    std::optional<int> pkColor;
    // m_bpcroom: the helper NPCs' plate armor lights.
    std::optional<bool> helperPlate;
    // CHARACTER::Class of a player-shaped monster (CLASS_WIZARD ...), and its
    // class step, CHARACTER::Skin: 0 the class, 1 its second class (the Blade
    // Knight), 2 its third.
    std::optional<int> playerClass;
    std::optional<int> classStep;
    // CHARACTER::m_bIsSelected: false, the mouse does not point at it (the
    // Battle Castle Shield).
    std::optional<bool> selectable;
    MonsterEquipment equipment;
    std::vector<MonsterSpawnEffect> spawnEffects;
    // The buffs it gets at spawn, by name ("poison").
    std::vector<std::string> spawnBuffs;
    // The sound it plays when it appears, by its SOUND_ name (Erohim's
    // entrance).
    std::optional<std::string> spawnSound;
    // A sub type drawn at spawn: one of these (Random::Rand() % count).
    std::vector<int> randomLook;
    // The glow ring of the Maya battle monsters starts at a random distance
    // (Gravity 0, Distance Random::Rand() % 20 / 10).
    bool randomStart = false;
    // x and y beside its tile in world units (the Gateway Machine's -20, -200).
    std::optional<std::array<float, 2>> positionOffset;
    // Object.Velocity keeps the ground height the character starts on, for
    // the map's code that raises it there (the Battle Castle crown switches).
    std::optional<bool> keepGroundHeight;
    // Above the ground it stands on.
    std::optional<float> heightOffset;
    // At this height, whatever the ground (Selupan while he waits).
    std::optional<float> height;
    // Tiles beside the one the server names, x and y (the Smelting NPC).
    std::optional<std::array<int, 2>> tileOffset;
    // A bounding box of its own over the model's (boundingBox in Models.json).
    std::optional<std::array<float, 3>> boxMin;
    std::optional<std::array<float, 3>> boxMax;
    // The first action (Object.CurrentAction, without SetAction) and frame.
    std::optional<int> startAction;
    std::optional<float> startFrame;
    // Object.LifeTime: the death effect armed (100).
    std::optional<int> deathEffect;
    // Object.Live: false drops the character at spawn (the Wolf Status, P24).
    std::optional<bool> live;
    // CHARACTER::NotRotateOnMagicHit.
    std::optional<bool> noTurnOnHit;
    std::optional<int> moveSpeed;
    // CHARACTER::Blood: the blood pool of its death is done already.
    std::optional<bool> noBloodPool;
    // How a monster the server sends (0x13) faces (Q-MN38a/b): a fixed
    // angle, whatever the direction the server sends (the Little Santas'
    // 90); else the direction times its step, 45 when it has none, or the
    // step of the last of its tiles it stands on.
    std::optional<float> facing;
    std::optional<float> facingStep;
    std::vector<MonsterTileFacing> facingByTile;
    // How it comes in when the server has just spawned it, and when it
    // teleported (Q-MN38c).
    std::optional<MonsterAppear> appear;
    std::optional<MonsterAppear> teleport;
    // It is a castle gate (Q-MN38c).
    std::optional<MonsterGate> gate;
    // Fields that are other while a state of the game is on (Q-MN36c).
    std::vector<MonsterStateVariant> stateVariants;
    // The maps' code it hands over to when it appears (Q-MN36d), in order.
    std::vector<MonsterHook> hooks;

    bool operator==(const MonsterSetup&) const = default;
};

// What a monster has differently while a state of the game is on (the siege,
// Raklion's waiting boss): each field it has replaces the setup's.
struct MonsterStateVariant
{
    // One of GetMonsterStates.
    std::string state;
    MonsterSetup setup;

    bool operator==(const MonsterStateVariant&) const = default;
};

// The states a state variant may name, as the game's code tells them
// (World::Monsters::IsSpawnStateOn).
std::span<const std::string_view> GetMonsterStates();
bool IsMonsterState(std::string_view name);

// A setup field as the monster list names it, each a value a number may have
// as its own (Q-MN50).
struct MonsterSetupField
{
    std::string_view key;
    bool (*isSet)(const MonsterSetup& setup);
    bool (*isSame)(const MonsterSetup& left, const MonsterSetup& right);
    void (*copy)(MonsterSetup& to, const MonsterSetup& from);
};

// The fields in the order the file writes them.
std::span<const MonsterSetupField> GetMonsterSetupFields();

// The buffs a setup may name ("poison") and their eBuffState; nullopt for
// another name.
std::optional<int> FindMonsterBuff(std::string_view name);
std::string_view GetMonsterBuffName(int buff);
// The buff names a setup may name, in their order (Monster Setup offers them).
std::vector<std::string_view> GetMonsterBuffNames();

// The PVP_ standings a pkColor may name ("murderer2"); nullopt for another
// name.
std::optional<int> FindPkColor(std::string_view name);
std::string_view GetPkColorName(int pk);
} // namespace Data::Monsters
