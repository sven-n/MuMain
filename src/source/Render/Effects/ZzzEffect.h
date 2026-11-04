#ifndef __ZZZ_EFFECT_H__
#define __ZZZ_EFFECT_H__
#include "Core/Utilities/Random.h"

#include <vector>

namespace Render::EffectDrawing
{
struct SpriteDraw;
class EffectDrawer;
} // namespace Render::EffectDrawing

extern OBJECT Effects[];
extern OBJECT Sprites[];
extern JOINT Joints[];

extern PARTICLE Particles[];
extern PARTICLE Leaves[];
extern PARTICLE Points[];
extern PARTICLE Pointers[];

void CreateBlur(CHARACTER* Owner, vec3_t p1, vec3_t p2, vec3_t Light, int Type, bool Short = false, int SubType = 0);
void MoveBlurs();
void RenderBlurs();
void ClearAllObjectBlurs();

void CreateObjectBlur(OBJECT* Owner, vec3_t p1, vec3_t p2, vec3_t Light, int Type, bool Short = false, int SubType = 0,
                      int iLimitLifeTime = -1);

void MoveObjectBlurs();
void RenderObjectBlurs();
void RemoveObjectBlurs(OBJECT* Owner, int SubType = 0);
void CreateSpark(int Type, CHARACTER* tc, vec3_t Position, vec3_t Angle);
void CreateBlood(OBJECT* o);

int CreateSprite(int Type, vec3_t Position, float Scale, vec3_t Light, OBJECT* Owner, float Rotation = 0.f,
                 int SubType = 0);
// Whether the sprite was created inside a bloom exclusion.
bool IsSpriteBloomExcluded(int sprite);
// Sprites are drawn for one frame: the update clears last frame's before anything spawns new ones.
void ClearSprites();
// A new frame's draws: every live sprite is to draw again, whether or not an update ran since the last frame.
void BeginSpriteDraws();
void RenderSprites(BYTE byRenderOneMore = 0);
// Which sprites this frame's passes already drew; a parity check gives it back to draw the world pass again.
[[nodiscard]] std::vector<bool> SpritesDrawnThisFrame();
void SetSpritesDrawnThisFrame(const std::vector<bool>& drawn);
// The draw a live sprite asks for: its quad, its blend and whether bloom leaves it alone.
Render::EffectDrawing::SpriteDraw DescribeSprite(const OBJECT& sprite, bool bloomExcluded);

void AnimationFlag();

int CreateParticle(int Type, vec3_t Position, vec3_t Angle, vec3_t Light, int SubType = 0, float Scale = 1.f,
                   OBJECT* Owner = NULL);
int CreateParticleFpsChecked(int Type, vec3_t Position, vec3_t Angle, vec3_t Light, int SubType = 0, float Scale = 1.f,
                             OBJECT* Owner = NULL);

void RenderParticles(BYTE byRenderOneMore = 0);
// Hands the sprites and state changes a live particle draws with to `out`; `slot` is its index in Particles.
void DescribeParticle(const PARTICLE& particle, int slot, float width, float height,
                      Render::EffectDrawing::EffectDrawer& out);
void MoveParticles();
bool DeleteParticle(int iType);

void RenderLeaves();
bool MoveLeaves();
void CreatePoint(vec3_t Position, int Value, vec3_t Color, float scale = 15.f, bool bMove = true,
                 bool bRepeatedly = false);
void RenderPoints(BYTE byRenderOneMore = 0);
void MovePoints();

void CreatePointer(int Type, vec3_t Position, float Angle, vec3_t Light, float Scale = 1.f);
void RenderPointers();
void MovePointers();

void CreateBonfire(vec3_t Position, vec3_t Angle);
void CreateFire(int Type, OBJECT* o, float x, float y, float z);
void CheckSkull(OBJECT* o);

void CreateHealing(OBJECT* o);

BOOL FindSameEffectOfSameOwner(int iType, OBJECT* pOwner);

void CheckClientArrow(OBJECT* o);

// Marks the effects the frame's view holds, for the draw and the next update; the last step of the update.
void CullEffects();
void RenderEffects(bool bRenderBlendMesh = false);
// Spawns the effects of the effect models the last cull marked; call once per update, after the effects moved.
void UpdateEffectModelEffects();
void RenderAfterEffects();
void RenderEffectShadows();
// Hands the ground marks and rings a visible effect lays under itself to `out`.
void DescribeEffectShadow(const OBJECT& effect, Render::EffectDrawing::EffectDrawer& out);
void CreateEffectFpsChecked(int Type, vec3_t Position, vec3_t Angle, vec3_t Light, int SubType = 0,
                            OBJECT* Target = NULL, short PKKey = -1, WORD SkillIndex = 0, WORD Skill = 0,
                            WORD SkillSerialNum = 0, float Scale = 0.0f, short int sTargetIndex = -1);
void CreateEffect(int Type, vec3_t Position, vec3_t Angle, vec3_t Light, int SubType = 0, OBJECT* Target = NULL,
                  short PKKey = -1, WORD SkillIndex = 0, WORD Skill = 0, WORD SkillSerialNum = 0, float Scale = 0.0f,
                  short int sTargetIndex = -1);
void MoveEffects();

void RenderCircle(int Type, vec3_t ObjectPosition, float ScaleBottom, float ScaleTop, float Height,
                  float Rotation = 0.f, float LightTop = 1.f, float TextureV = 0.f);
void RenderCircle2D(int Type, vec3_t ObjectPosition, float ScaleBottom, float ScaleTop, float Height,
                    float Rotation = 0.f, float TextureV = 0.f, float TextureVScale = 0.f);

void CreateTeleportBegin(OBJECT* o);
void CreateTeleportEnd(OBJECT* o);
void CreateMagicShiny(CHARACTER* c, int Hand = 0);

bool AttackCharacterRange(int Index, vec3_t Position, float Range, BYTE Serial, short PKKey = -1,
                          WORD SkillSerialNum = 0);

void CreateArrow(CHARACTER* c, OBJECT* o, OBJECT* to, WORD SkillIndex, WORD Skill, WORD SKKey);
void CreateArrows(CHARACTER* c, OBJECT* o, OBJECT* to, WORD SkillIndex = 0, WORD Skill = 1, WORD SKKey = 0);

void DeleteEffect(int efftype);

void EffectDestructor(OBJECT* o);

void TerminateOwnerEffectObject(int iOwnerObjectType = -1);

bool DeleteEffect(int Type, OBJECT* Owner, int iSubType = -1);
bool SearchEffect(int iType, OBJECT* pOwner, int iSubType = -1);

void CreateBomb(vec3_t p, bool Exp, int SubType = 0);
void CreateBomb2(vec3_t p, bool Exp, int SubType = 0, float Scale = 0.0f);
void CreateBomb3(vec3_t vPos, int iSubType, float fScale = 1.0f);
void CreateInferno(vec3_t Position, int SubType = 0);

void DeleteJoint(int Type, OBJECT* Target, int SubType = -1);
bool SearchJoint(int Type, OBJECT* Target, int SubType = -1);
void CreateTail(JOINT* o, float Matrix[3][4], bool Blur = false);
void CreateTailAxis(JOINT* o, float Matrix[3][4], BYTE axis = 0);

void MoveJoint(JOINT* o, int iIndex);
void MoveJoints();
void RenderJoints(BYTE bRenderOneMore = 0);
// Hands the state changes, trail quads and ground mark a live joint draws with to `out`.
void DescribeJoint(const JOINT& joint, Render::EffectDrawing::EffectDrawer& out);

void GetMagicScrew(int iParam, vec3_t vResult, float fSpeedRate = 1.0f);
void GetMagicScrewAt(double frame, vec3_t vResult, float fSpeedRate = 1.0f);
double ReferenceFrameClock();

inline void GetNearRandomPos(vec3_t Pos, int iRange, vec3_t NewPos)
{
    VectorCopy(Pos, NewPos);
    NewPos[0] += (float)(Random::Rand() % (iRange * 2 + 1) - iRange);
    NewPos[1] += (float)(Random::Rand() % (iRange * 2 + 1) - iRange);
    NewPos[2] += (float)(Random::Rand() % (iRange * 2 + 1) - iRange);
}

void CreateForce(OBJECT* o, vec3_t Pos);

bool MoveJump(OBJECT* o);
void MoveParticle(OBJECT* o, int Turn);
void MoveParticle(OBJECT* o, vec3_t angle);

#ifdef ASG_ADD_INFLUENCE_GROUND_EFFECT
void CreateMyGensInfluenceGroundEffect();
#endif // ASG_ADD_INFLUENCE_GROUND_EFFECT

#endif // __ZZZ_EFFECT_H__
