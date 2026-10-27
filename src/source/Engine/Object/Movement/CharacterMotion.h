#pragma once

#include <cmath>

// A character's state for the movement features (Enhanced Movement) that work after it moved: how it's drawn
// flying, trailing a turn, looking, greeting, slowing down and fading between actions. CHARACTER::Motion.
struct CharacterMotion
{
    // A flying character's feel (UpdateFlightFeel): the roll it banks by into a turn, how far the gentle up-and-down
    // of its flight has faded in (0 to 1), and the height it last added for it (Object.Position[2]).
    float FlightBank = 0.f;
    float FlightHover = 0.f;
    float FlightHoverApplied = 0.f;
    // The sideways shift last added (Object.Position x, y) so the bank pivots at the waist, not the feet.
    float FlightPivotShift[2] = {0.f, 0.f};
    // How far the wings and the ponytail trail behind the body's turn (UpdateTurnLag), in degrees, how fast that
    // changes, and the facing it last saw (NAN: none yet).
    float TurnLag = 0.f;
    float TurnLagSpeed = 0.f;
    float TurnLagFacing = NAN;
    // A walking character's momentum (StepMomentum): how much of its speed it has (0 to 1), the way it really moves
    // (NAN: not moving), and the world time it last moved (milliseconds; a gap means it starts again from rest).
    float MomentumPace = 1.f;
    float MomentumHeading = NAN;
    double MomentumAt = -1.0;
    // The pitch a slope gives the character now (UpdateSlopeLean), in degrees, as last added to Object.Angle[0].
    float SlopePitch = 0.f;
    // A nod (UpdateNod): how far through it the head is (seconds left, 0: none), the pitch it last added to
    // Object.HeadAngle[1], how long until it may nod again, how long it has been looking at the hero, and whether it
    // already nodded at the hero this look.
    float NodLeft = 0.f;
    float NodApplied = 0.f;
    // The bow that goes with it (Object.Angle[0], as last added).
    float NodBowApplied = 0.f;
    float NodCooldown = 0.f;
    float WatchedHeroSeconds = 0.f;
    bool NoddedAtHero = false;
    // An NPC's greeting instead of a nod (Greet: its second animation, played once): seconds until it's back to
    // standing (0: none).
    float GreetLeft = 0.f;
    // The character its head looks at (UpdateLookAt; an index into CharactersClient, -1: none) and how long until it
    // looks around for another, in seconds.
    short LookTarget = -1;
    float LookScanSeconds = 0.f;
    // A sudden turn (an attack facing its target) drawn quickly rather than at once (UpdateSnapTurn): how far the drawn
    // body is still behind the facing, in degrees, how fast it catches up, and the facing it last saw (NAN: none yet).
    float SnapTurnOffset = 0.f;
    float SnapTurnSpeed = 0.f;
    float SnapTurnFacing = NAN;
    // A fade from one way of moving into the next (UpdatePoseFade): the action faded out, its frame, how much of it is
    // left (1 to 0), and the action and frame the character had last frame (-1: none yet).
    int FadeAction = -1;
    float FadeFrame = 0.f;
    float FadeLeft = 0.f;
    float FadeSeconds = 0.f;
    int FadeLastAction = -1;
    float FadeLastFrame = 0.f;

    // Back to a character that has done none of it yet.
    void Reset()
    {
        *this = CharacterMotion{};
    }
};
