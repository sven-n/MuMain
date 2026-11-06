#pragma once

#include "Data/GameData/ItemData/ItemDataIssue.h"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

// Which sound the game's code plays when (Data/Audio/SoundEvents.json, sound
// editor design Q-SE1): an event of the code (the hero's step, a player's
// swing) names what it knows, and the first of the event's rules that
// matches gives the sound.
//
//   { "formatVersion": 1,
//     "events": [
//       { "event": "heroFootstep", "rules": [
//           { "worlds": [0], "tiles": [0], "plays": ["SOUND_HUMAN_WALK_GRASS"] },
//           { "plays": ["SOUND_HUMAN_WALK_GROUND"] } ] },
//       { "event": "interfaceClick", "rules": [
//           { "kinds": ["confirm"], "plays": ["SOUND_CLICK01"] } ] } ] }
namespace Data::Sounds
{
constexpr int SoundEventsFormatVersion = 1;

// The highest world and tile a rule can name: the reader refuses more, and
// the Sound Editor's Events tab takes no more.
inline constexpr int MaxEventWorld = 255;
inline constexpr int MaxEventTile = 255;

// The events the code names.
inline constexpr std::string_view HeroFootstepEvent = "heroFootstep";
inline constexpr std::string_view PlayerAttackSwingEvent = "playerAttackSwing";
// A click on a button whose style names a click kind (the themes'
// `click-sound: confirm`, Q-UI1), or a key that stands for one.
inline constexpr std::string_view InterfaceClickEvent = "interfaceClick";
// A click on a character of the character select scene.
inline constexpr std::string_view CharacterSelectedEvent = "characterSelected";

// What a player holds at an attack's start, as the code tells it: the first
// that fits.
enum class HeldWeapon
{
    // No weapon in either hand.
    None,
    Bow,
    Crossbow,
    // The bow group's items 13 and 14 (Bluewing Crossbow and the one after),
    // which count as neither.
    MagicCrossbow,
    // A Light Saber or a Spear in the right hand.
    SaberOrSpear,
    // Any other weapon.
    Other,
};

struct SoundEventRule
{
    // The worlds (WD_*) it plays on, at most MaxEventWorld; empty: any.
    std::vector<int> worlds;
    // The hero's ground tiles (HeroTile), at most MaxEventTile; empty: any.
    std::vector<int> tiles;
    // Only in or outside the safe zone; none: either.
    std::optional<bool> safeZone;
    // Only for what the player holds; none: anything.
    std::optional<HeldWeapon> holds;
    // Only for buttons of these click kinds (IsClickKindWord); empty: any.
    std::vector<std::string> kinds;
    // The SOUND_ ids of the sounds: one plays, picked at random (Random::Rand() %
    // count) when there are more.
    std::vector<std::string> plays;
    // Their numbers, read with them.
    std::vector<int> numbers;

    bool operator==(const SoundEventRule&) const = default;
};

struct SoundEvent
{
    std::string event;
    std::vector<SoundEventRule> rules;

    bool operator==(const SoundEvent&) const = default;
};

// What the game knows when an event happens.
struct SoundEventContext
{
    int world = -1;
    int tile = -1;
    bool safeZone = false;
    HeldWeapon holds = HeldWeapon::None;
    // The clicked button's click kind.
    std::string_view kind;
};

// A click kind as the themes and the file write it: lower-case letters,
// digits and '-', at least one ("confirm", "tab-select").
bool IsClickKindWord(std::string_view word);

// Reads the events into `events`. An event or rule with a value of the wrong
// form, a SOUND_ id that is none, a rule that plays nothing or an event named
// twice is fatal ("events[0].rules[2].plays[0]"); unknown fields are warnings.
void ReadSoundEventsJson(std::string_view text, const std::string& source, std::vector<SoundEvent>& events,
                         std::vector<Items::ItemDataIssue>& issues);

// The file's text, each rule on its own line.
std::string WriteSoundEventsJson(std::span<const SoundEvent> events);

// The first rule of the event that matches; nullptr for none.
const SoundEventRule* MatchRule(const SoundEvent& event, const SoundEventContext& context);

// The word of a held weapon in the file ("magicCrossbow"), and back.
std::string_view HeldWeaponWord(HeldWeapon weapon);
std::optional<HeldWeapon> FindHeldWeapon(std::string_view word);
} // namespace Data::Sounds
