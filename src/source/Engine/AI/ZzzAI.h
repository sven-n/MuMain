#pragma once

#include <cstdint>

/**
 * \brief It's the FPS for which the game was initially developed.
 * All speeds of animations, positional movements, etc. are based on it.
 * Increasing this value means increasing the speed on which the game is running.
 * It's not recommended to change this value.
 */
constexpr double REFERENCE_FPS = 25.0;

// Adds a frame's share of a reference frame and takes out the whole reference frames it completes.
int TakeReferenceFrameSteps(float& referenceFrames, float frameFactor);

int CalcAngle(float PositionX, float PositionY, float TargetX, float TargetY);
float CreateAngle(float x1, float y1, float x2, float y2);
float CreateAngle2D(const vec3_t from, const vec2_t to);
int TurnAngle(int iTheta, int iHeading, int maxTURN);
float TurnAngle2(float angle, float a, float d);
float FarAngle(float angle1, float angle2, bool absolute = true);
// One frame of a turn that eases in and out: `speed` (degrees per second, signed, kept between frames) moves toward
// the fastest that still stops on `target`, by at most acceleration * seconds and never past maxSpeed. Returns the new
// angle (0 to 360), which lands on `target` (with speed 0) rather than passing it.
float StepEasedTurn(float angle, float target, float& speed, float seconds, float maxSpeed, float acceleration);
// One frame of a damped spring pulling `value` back to 0: `velocity` (per second) is kept between frames, `frequency`
// is how many times a second it would swing undamped, and `damping` how fast the swing dies out (1: no overshoot,
// below 1: it overshoots a little and settles). The same at any frame rate.
void StepSpring(float& value, float& velocity, float seconds, float frequency, float damping);
// perFrame false: one spatial step of a shape built within a frame (a lightning bolt), so the turn is not time-scaled.
float MoveHumming(vec3_t Position, vec3_t Angle, vec3_t TargetPosition, float Turn, bool perFrame = true);
void MovePosition(vec3_t Position, vec3_t Angle, vec3_t Speed);
std::uint8_t CalcTargetPos(float x, float y, int Tx, int Ty);
void Alpha(OBJECT* o);
void MoveBoid(OBJECT* o, int i, OBJECT* Boids, int MAX, float referenceFrames);
void PushObject(vec3_t PushPosition, vec3_t Position, float Power, vec3_t Angle);

void SetAction(OBJECT* o, int Action, bool bBlending = true);

bool TestDistance(CHARACTER* c, vec3_t TargetPosition, float Range);

void LookAtTarget(OBJECT* o, const CHARACTER* targetCharacter);
void MoveHead(CHARACTER* c);
void SmoothHeadAngles(vec3_t angles, const vec3_t targetAngles, float referenceFrames);
void Damage(vec3_t soPosition, CHARACTER* tc, float AttackRange, int AttackPoint, bool Hit);

// How fast a character's facing turns toward the direction it travels, at most, and how quickly a turn gets up to
// that speed and slows down again before it arrives.
constexpr float WalkTurnDegreesPerSecond = 540.f;
constexpr float WalkTurnAcceleration = 1440.f;
// One frame of a character's facing turning toward its MoveHeading (StepEasedTurn, with the walk turn's speeds).
void TurnTowardsMoveHeading(CHARACTER* c);
// Starts a standing character turning toward `angle` (a direction from the server), stepping around instead of
// snapping.
void StartStandingTurn(CHARACTER* c, float angle);
// One frame of a standing character's turn (CHARACTER::TurnPending: one a walk ended before, or StartStandingTurn's):
// called each frame for every character. The feet step with the walk animation until the facing arrives.
void FinishPendingTurn(CHARACTER* c);

// One frame along the character's path: sets its MoveHeading toward the next point (and, with Turn, turns its
// facing toward it); true when the path's end is reached.
bool MovePath(CHARACTER* c, bool Turn = true);
void InitPath();
bool PathFinding2(int sx, int sy, int tx, int ty, PATH_t* a, float fDistance = 0.0f, int iDefaultWall = TW_CHARACTER);

void SetAction_Fenrir_Skill(CHARACTER* c, OBJECT* o);
void SetAction_Fenrir_Damage(CHARACTER* c, OBJECT* o);
void SetAction_Fenrir_Run(CHARACTER* c, OBJECT* o);
void SetAction_Fenrir_Walk(CHARACTER* c, OBJECT* o);
bool IsAliceRideAction_UniDino(unsigned short byAction);
bool IsAliceRideAction_Fenrir(unsigned short byAction);
