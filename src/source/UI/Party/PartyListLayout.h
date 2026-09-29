#pragma once

// Pure geometry of the party HUD list (CPartyListWindow), in the original's reference units.
namespace UI::Party::List
{
constexpr int CardWidth = 77;
constexpr int CardHeight = 23;
constexpr int CardSpacing = 24; // one card below the other, 1 unit apart
constexpr int HealthBarWidth = 69;
constexpr int HealthSteps = 10;
// RenderText()'s box for the name: narrower on the leader's card, which carries the flag.
constexpr int LeaderNameBoxWidth = 48;
constexpr int MemberNameBoxWidth = 58;

// The health bar's width for a member's health step (0..10, a step above 10 is a full bar).
float HealthBarLength(int stepHP);

} // namespace UI::Party::List
