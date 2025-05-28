#pragma once

/**
 * \brief It's the FPS for which the game was initially developed.
 * All speeds of animations, positional movements, etc. are based on it.
 * Increasing this value means increasing the speed on which the game is running.
 * It's not recommended to change this value.
 */
constexpr double REFERENCE_FPS = 25.0;

int CalcAngle(float PositionX, float PositionY, float TargetX, float TargetY);
float CreateAngle(float x1, float y1, float x2, float y2);
float CreateAngle2D(const vec3_t from, const vec2_t to);
int TurnAngle(int iTheta, int iHeading, int maxTURN);
float TurnAngle2(float angle, float a, float d);
float FarAngle(float angle1, float angle2, bool abs = true);
float MoveHumming(vec3_t Position, vec3_t Angle, vec3_t TargetPosition, float Turn);
void  MovePosition(vec3_t Position, vec3_t Angle, vec3_t Speed);
BYTE  CalcTargetPos(float x, float y, int Tx, int Ty);
void  Alpha(OBJECT* o);
void MoveBoid(OBJECT* o, int i, OBJECT* Boids, int MAX);
void PushObject(vec3_t PushPosition, vec3_t Position, float Power, vec3_t Angle);

void SetAction(OBJECT* o, int Action, bool bBlending = true);

bool TestDistance(CHARACTER* c, vec3_t TargetPosition, float Range);

void LookAtTarget(OBJECT* o, const CHARACTER* targetCharacter);
void MoveHead(CHARACTER* c);
void Damage(vec3_t soPosition, CHARACTER* tc, float AttackRange, int AttackPoint, bool Hit);

bool MovePath(CHARACTER* c, bool Turn = true);
void InitPath();
bool PathFinding2(int sx, int sy, int tx, int ty, PATH_t* a, float fDistance = 0.0f, int iDefaultWall = TW_CHARACTER);

void SetAction_Fenrir_Skill(CHARACTER* c, OBJECT* o);
void SetAction_Fenrir_Damage(CHARACTER* c, OBJECT* o);
void SetAction_Fenrir_Run(CHARACTER* c, OBJECT* o);
void SetAction_Fenrir_Walk(CHARACTER* c, OBJECT* o);
bool IsAliceRideAction_UniDino(unsigned short byAction);
bool IsAliceRideAction_Fenrir(unsigned short byAction);
