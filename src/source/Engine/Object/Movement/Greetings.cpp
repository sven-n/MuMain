#include "stdafx.h"
#include "Engine/Object/Movement/Greetings.h"

#include "Character/CharacterManager.h"
#include "Core/Utilities/Random.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/EnhancedMovement.h"
#include "Engine/Object/Movement/FrameTime.h"
#include "Engine/Object/Movement/LookAt.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Models/ZzzBMD.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace Engine::Characters
{
namespace
{
// A nod: the head dips this far and back up in this long. A player or NPC this many tiles from the hero at most nods at
// it after looking at it this long, at most once in this long. The hero always answers a nod it can turn its head to
// (HeroLookDegrees from where it faces: the head and shoulders, never a step): it looks at the nodder for this long
// and nods back after this beat.
constexpr int NodTiles = 2;
// Once it greeted the hero, a character greets it again only after the hero has gone: off the screen or this many
// tiles away.
constexpr int NodForgetTiles = 8;
// A nod is slow and plain to see: the head dips this far and the body bows this far, in this long. Another player
// standing greets the hero instead (its Hello).
constexpr float NodDegrees = 25.f;
constexpr float NodBowDegrees = 5.f;
constexpr float NodSeconds = 0.9f;
constexpr float NodWatchSeconds = 1.5f;
constexpr float NodCooldownSeconds = 10.f;
// The hero answers after a beat, taking the greeting in first, never quite the same (between these): two people
// don't greet at the same moment. Its head stays on them until its nod is done, and a little longer.
constexpr float HeroNodReplySeconds[2] = {1.6f, 2.8f};
constexpr float HeroNodAttentionAfterSeconds = 0.4f;

// The hero's answer to a nod: who nodded (an index into CharactersClient, -1: none), how long its head stays on
// them, and how long until it nods back (negative: done or none).
int s_heroNodder = -1;
float s_heroNodAttention = 0.f;
float s_heroNodReplyIn = -1.f;

void StartNod(CHARACTER* c)
{
    c->Motion.NodLeft = NodSeconds;
    c->Motion.NodCooldown = NodCooldownSeconds;
}

// Whether `c` is one of the client's character slots (a character screen's isn't).
bool InClientSlots(const CHARACTER* c)
{
    const std::ptrdiff_t slot = c - CharactersClient;
    return slot >= 0 && slot <= MAX_CHARACTERS_CLIENT;
}

// A character greeting the hero instead of nodding (only on this screen: the server isn't told). Another player, or
// an NPC built on the player skeleton (Leo the Helper), standing: its Hello. Another NPC with a second animation
// plays that once (its own idle variation, which reads as a greeting). Whether it greeted.
bool Greet(CHARACTER* c)
{
    OBJECT* o = &c->Object;
    const BMD& model = Models[o->Type];
    const bool playerSkeleton = o->Type == MODEL_PLAYER || model.NumActions == Models[MODEL_PLAYER].NumActions;
    if (playerSkeleton)
    {
        if (o->CurrentAction < PLAYER_STOP_MALE || o->CurrentAction > PLAYER_STOP_RIDE_WEAPON)
        {
            return false;
        }
        SetAction(o, gCharacterManager.IsFemale(c->Class) ? PLAYER_GREETING_FEMALE1 : PLAYER_GREETING1);
    }
    else if (o->Kind == KIND_NPC && model.NumActions >= 2 && o->CurrentAction == 0 &&
             model.Actions[1].NumAnimationKeys > 1 && model.Actions[1].PlaySpeed > 0.f)
    {
        if (!InClientSlots(c))
        {
            return false;
        }
        SetAction(o, 1);
        // Back to standing once it has played through.
        c->Motion.GreetLeft = static_cast<float>(model.Actions[1].NumAnimationKeys) /
                              (model.Actions[1].PlaySpeed * static_cast<float>(REFERENCE_FPS));
    }
    else
    {
        return false;
    }
    c->Motion.NodCooldown = NodCooldownSeconds;
    return true;
}

// An NPC's greeting played through: back to standing.
void EndGreeting(CHARACTER* c, float seconds)
{
    if (c->Motion.GreetLeft <= 0.f)
    {
        return;
    }
    c->Motion.GreetLeft -= seconds;
    if (c->Motion.GreetLeft <= 0.f && c->Object.CurrentAction == 1)
    {
        SetAction(&c->Object, 0);
    }
}

// The bow of a nod: on and off the body's pitch, as the slope lean is.
void BowFor(CHARACTER* c, float bow)
{
    c->Object.Angle[0] += bow - c->Motion.NodBowApplied;
    c->Motion.NodBowApplied = bow;
}

// The hero answering a nod: the head on them for a moment, and a nod back after a beat.
void AnswerNod(CHARACTER* hero, float seconds)
{
    s_heroNodAttention = std::max(0.f, s_heroNodAttention - seconds);
    if (s_heroNodReplyIn >= 0.f)
    {
        s_heroNodReplyIn -= seconds;
        if (s_heroNodReplyIn < 0.f)
        {
            StartNod(hero);
        }
    }
}

int TilesFromHero(const CHARACTER* c)
{
    return std::max(std::abs(c->PositionX - Hero->PositionX), std::abs(c->PositionY - Hero->PositionY));
}

// Looking at the hero, close: after a moment, one greeting, and the hero answers it if it can turn its head to it.
void WatchHero(CHARACTER* c, float seconds)
{
    c->Motion.WatchedHeroSeconds += seconds;
    const bool close = TilesFromHero(c) <= NodTiles;
    if (!close || c->Motion.NoddedAtHero || c->Motion.WatchedHeroSeconds < NodWatchSeconds ||
        c->Motion.NodCooldown > 0.f)
    {
        return;
    }
    c->Motion.NoddedAtHero = true;
    if (!Greet(c))
    {
        StartNod(c);
    }
    const float toNodder = CreateAngle2D(Hero->Object.Position, c->Object.Position);
    const bool inView = std::fabs(FarAngle(Hero->Object.Angle[2], toNodder, false)) <= HeroLookDegrees;
    if (inView && s_heroNodReplyIn < 0.f && Hero->Dead == 0)
    {
        s_heroNodder = static_cast<int>(c - CharactersClient);
        s_heroNodReplyIn = Random::RangeFloat(HeroNodReplySeconds[0], HeroNodReplySeconds[1]);
        s_heroNodAttention = s_heroNodReplyIn + NodSeconds + HeroNodAttentionAfterSeconds;
    }
}

// Not looking at the hero: it greets the hero again only once the hero has gone, off the screen or far away. Looking
// elsewhere meanwhile (the hero stepping round it) doesn't count.
void ForgetHero(CHARACTER* c)
{
    c->Motion.WatchedHeroSeconds = 0.f;
    if (Hero == nullptr || !c->Object.Visible || TilesFromHero(c) > NodForgetTiles)
    {
        c->Motion.NoddedAtHero = false;
    }
}

// One frame of a nod under way: down and back up, eased, the body bowing with it.
void PlayNod(CHARACTER* c, float seconds)
{
    if (c->Motion.NodLeft <= 0.f)
    {
        if (c->Motion.NodBowApplied != 0.f)
        {
            BowFor(c, 0.f);
        }
        return;
    }
    c->Motion.NodLeft = std::max(0.f, c->Motion.NodLeft - seconds);
    // A positive pitch looks up, so a nod is negative; a positive body pitch leans forward.
    const float through = 1.f - c->Motion.NodLeft / NodSeconds;
    const float dip = std::sin(through * static_cast<float>(Q_PI));
    c->Motion.NodApplied = -NodDegrees * dip;
    c->Object.HeadAngle[1] += c->Motion.NodApplied;
    BowFor(c, NodBowDegrees * dip);
}
} // namespace

void RemoveNod(CHARACTER* c)
{
    c->Object.HeadAngle[1] -= c->Motion.NodApplied;
    c->Motion.NodApplied = 0.f;
}

void UpdateNod(CHARACTER* c)
{
    const float seconds = FrameSeconds();
    EndGreeting(c, seconds);
    c->Motion.NodCooldown = std::max(0.f, c->Motion.NodCooldown - seconds);
    if (!EnhancedMovementEnabled())
    {
        // Off: a nod under way stops, its bow comes off, and no answer is pending.
        c->Motion.NodLeft = 0.f;
        c->Motion.WatchedHeroSeconds = 0.f;
        c->Motion.NoddedAtHero = false;
        if (c == Hero)
        {
            s_heroNodReplyIn = -1.f;
            s_heroNodAttention = 0.f;
        }
        BowFor(c, 0.f);
        return;
    }
    if (c == Hero)
    {
        AnswerNod(c, seconds);
    }
    else if ((c->Object.Kind == KIND_PLAYER || c->Object.Kind == KIND_NPC) && Hero != nullptr && c->Dead == 0 &&
             c->Motion.LookTarget == static_cast<short>(Hero - CharactersClient))
    {
        WatchHero(c, seconds);
    }
    else
    {
        ForgetHero(c);
    }
    PlayNod(c, seconds);
}

const CHARACTER* HeroNodAttention()
{
    if (!EnhancedMovementEnabled() || s_heroNodAttention <= 0.f || s_heroNodder < 0 ||
        s_heroNodder >= MAX_CHARACTERS_CLIENT)
    {
        return nullptr;
    }
    const CHARACTER* nodder = &CharactersClient[s_heroNodder];
    return nodder->Object.Live ? nodder : nullptr;
}
} // namespace Engine::Characters
