#pragma once

#include "Core/Globals/_enum.h"

#include <cstdint>
#include <span>
#include <string_view>

// HUD changes the server reports. Strings are borrowed for the call.
namespace UI::Hud
{
// This player's Gens standing: contribution points, rank, and the points the next rank needs.
void SetGensStanding(int contribution, int ranking, int nextContribution);

// Shows the current map's name banner.
void ShowMapName();

// Experience from a kill, against the master-level bar when `master` is set.
void ShowExperienceGain(std::int64_t previous, std::int64_t gained, bool master);

void ClearSkillHotkeys();
// `skillIndex` is the slot in the character's skill list.
void SetSkillHotkey(int hotkey, int skillIndex);

enum class ItemHotkey : std::uint8_t
{
    Q,
    W,
    E,
    R,
};
void SetItemHotkey(ItemHotkey hotkey, int itemType, int itemLevel);

// A scrolling notice across the top of the screen.
void AddSlideNotice(int loopCount, int loopDelay, std::wstring_view text, int type, float speed,
                    std::uint32_t color);

// Suspends or resumes hotkeys while the death screen is up.
void SetGameOver(bool gameOver);

// The key the server expects back with the next map-move request.
void SetMoveCommandKey(std::uint32_t key);

struct MasterSkill
{
    int index;
    std::uint8_t level;
    float value;
    float nextValue;
};
// Rebuilds the master skill tree for `heroClass` from the learned skills.
void ReplaceMasterSkills(CLASS_TYPE heroClass, std::span<const MasterSkill> skills);
void UpgradeMasterSkill(const MasterSkill& skill);
}
