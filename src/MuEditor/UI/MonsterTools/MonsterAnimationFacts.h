#pragma once

#ifdef _EDITOR

#include <optional>
#include <string>
#include <vector>

// What the game's code does with a model's animations on top of the model
// list, for the Monster Model and Monster Preview tools. The editor shows
// it; the code decides it (ZzzCharacter.cpp: CharacterAnimation's speed,
// EtcStopAnimationSetting's next animation, AttackStage's impact frame), so
// a change there belongs here too.
namespace MuEditor::Monsters::AnimationFacts
{
// The frame of attack 1 and 2 at which the hit of a monster-range model
// lands (ATTACK_IMPACT_FRAME); nullopt for other animations and models.
std::optional<float> GetImpactFrame(int slot, int animation);

// The animation a monster-range model goes on with when this one ends:
// stop 1 after stop 2, shock and attacks 1 to 5, also after appear for
// Maya's hands and Selupan. nullopt when the animation starts again or holds
// its last frame (die), and for the NPC range, where some models pick the
// next one in code.
std::optional<int> GetNextAnimation(int slot, int animation);

// The frame the game blends a model's pose from at `frame` (BMD::Animation
// blends from the prior frame to the frame): BMD::PlayAnimation keeps the key
// it left as the prior frame, so frame 3.5 shows key 2 halfway to key 3 and
// the model moves between its keys. In the first key it blends from the last
// of the `playedFrames` keys of an animation that starts again, and not at
// all in one that holds its last frame (it starts there).
float GetPriorFrame(float frame, int playedFrames, bool holdsLastFrame);

// Whether the model is one of the monster range, whose code is that of
// monsters (die with a blood pool, the next animation, the impact frame, a
// release at a map change).
bool IsMonsterRange(int slot);

// How the game changes an animation's speed on top of the model list.
struct SpeedFactor
{
    enum class When
    {
        // A player transformed into the monster (CHARACTER::Change).
        Transformed,
        // After frame 6 of the animation.
        AfterFrame6,
        Always,
    };
    float factor = 1.0f;
    When when = When::Always;
};
std::vector<SpeedFactor> GetSpeedFactors(int slot, int animation);

// All of it as one line in the UI language: "x1.5 when transformed; then
// stop1; hit at frame 5"; empty when the game adds nothing.
std::string DescribeInGame(int slot, int animation);
} // namespace MuEditor::Monsters::AnimationFacts

#endif // _EDITOR
