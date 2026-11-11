#pragma once

// The named fields of the game's characters, objects and effect pool
// elements, for the recorders that compare what code and data write into them:
// the effect recorder (effect catalogue, D40) and the monster recorder's
// creation part (monsters design, section 10). Each list names every data
// member, so a recorder sees a value that no list names as a size change: the
// build stops until the list has the new member too.

#include "Engine/Object/ZzzCharacter.h"
#include "Render/Effects/ZzzEffect.h"

// Keep in step with OBJECT (w_ObjectInfo.h), PARTICLE and JOINT (_struct.h),
// CHARACTER and its parts (w_CharacterInfo.h).
// clang-format off
#define RECORDER_OBJECT_FIELDS(F)                                                                                    \
    F(Live) F(bBillBoard) F(m_bCollisionCheck) F(m_bRenderShadow) F(EnableShadow) F(LightEnable) F(m_bActionStart)   \
    F(m_bRenderAfterCharacter) F(Visible) F(AlphaEnable) F(EnableBoneMatrix) F(ContrastEnable) F(ChromeEnable)       \
    F(OwnsBoneTransform)                                                                                             \
    F(AI) F(CurrentAction) F(PriorAction) F(ExtState) F(Teleport) F(Kind) F(Skill) F(m_byNumCloth)                   \
    F(m_byHurtByDeathstab) F(WeaponLevel) F(DamageTime) F(m_byBuildTime) F(m_bySkillCount) F(m_bySkillSerialNum)     \
    F(Block) F(m_pCloth) F(ScreenX) F(ScreenY) F(Weapon) F(Type) F(SubType) F(m_iAnimation) F(HiddenMesh)            \
    F(LifeTime) F(BlendMesh) F(AttackPoint) F(RenderType) F(InitialSceneTime) F(LinkBone) F(m_dwTime) F(Scale)       \
    F(BlendMeshLight) F(BlendMeshTexCoordU) F(BlendMeshTexCoordV) F(Timer) F(m_fEdgeScale) F(Velocity)               \
    F(CollisionRange) F(ShadowScale) F(Gravity) F(Distance) F(AnimationFrame) F(PriorAnimationFrame)                 \
    F(AlphaTarget) F(Alpha) F(LastHorseWaveEffect) F(PKKey) F(Light) F(Direction) F(m_vPosSword) F(StartPosition)    \
    F(BoundingBoxMin) F(BoundingBoxMax) F(m_vDownAngle) F(m_vDeadPosition) F(Position) F(Angle) F(HeadAngle)         \
    F(HeadTargetAngle) F(EyeLeft) F(EyeRight) F(EyeLeft2) F(EyeRight2) F(EyeLeft3) F(EyeRight3) F(SetTrailLeft)      \
    F(SetTrailRight) F(Matrix)                                                                                       \
    F(BoneTransform) F(Pose.Type) F(Pose.CurrentAction) F(Pose.PriorAction) F(Pose.AnimationFrame)                   \
    F(Pose.PriorAnimationFrame) F(Pose.Angle) F(Pose.HeadAngle) F(Pose.World) F(Pose.Origin) F(Pose.Scale)           \
    F(Pose.Height) F(DrawnFrame) F(OBB.StartPos) F(OBB.XAxis) F(OBB.YAxis) F(OBB.ZAxis) F(Owner) F(Prior)            \
    F(Next) F(m_BuffMap)                                                                                             \
    F(m_sTargetIndex) F(m_bpcroom) F(m_v3PrePos1) F(m_v3PrePos2) F(m_Interpolates)

#define RECORDER_PARTICLE_FIELDS(F)                                                                                  \
    F(Live) F(Type) F(TexType) F(SubType) F(Scale) F(Position) F(Angle) F(Light) F(Alpha) F(LifeTime) F(Target)      \
    F(Rotation) F(Frame) F(bEnableMove) F(Gravity) F(Velocity) F(TurningForce) F(StartPosition) F(iNumBone)          \
    F(bRepeatedly) F(fRepeatedlyHeight)

#define RECORDER_JOINT_FIELDS(F)                                                                                     \
    F(Live) F(Type) F(TexType) F(SubType) F(RenderType) F(RenderFace) F(Scale) F(Position) F(StartPosition)          \
    F(Angle) F(HeadAngle) F(Light) F(Target) F(TargetPosition) F(byOnlyOneRender) F(LifeTime) F(Collision)           \
    F(Velocity) F(Direction) F(PKKey) F(Skill) F(Weapon) F(MultiUse) F(bTileMapping) F(m_byReverseUV)                \
    F(TargetIndex) F(m_bySkillSerialNum) F(m_iChaIndex) F(m_sTargetIndex) F(m_bCreateTails) F(NumTails)              \
    F(MaxTails) F(Tails) F(TailFrames)

// CHARACTER without its Object (RECORDER_OBJECT_FIELDS), its parts (PART_t,
// PATH_t and PET_INFO below) and the private post-move process, which the
// recorders read through PostMoveProcess_IsProcessing.
#define RECORDER_CHARACTER_FIELDS(F)                                                                                 \
    F(Blood) F(Ride) F(SkillSuccess) F(NotRotateOnMagicHit) F(Foot) F(SafeZone) F(Change) F(HideShadow)              \
    F(m_bIsSelected) F(Decoy) F(SoundOncePlayed) F(SoundsDrawnThisFrame) F(ModelTimerLastTicks) F(ModelTimerTicks)   \
    F(ModelTimersAsked) F(Class) F(SkinIndex) F(Skin) F(CtlCode) F(ExtendState) F(EtcPart) F(GuildStatus)            \
    F(GuildType) F(GuildRelationShip) F(GuildSkill) F(GuildMasterKillCount) F(BackupCurrentSkill) F(GuildTeam)       \
    F(m_byGensInfluence) F(PK) F(AttackFlag) F(TargetAngle) F(Dead) F(Skill) F(SwordCount) F(AttackRotation)         \
    F(byExtensionSkill) F(m_byDieType) F(TargetX) F(TargetY) F(SkillX) F(SkillY) F(Appear) F(CurrentSkill)           \
    F(CastRenderTime) F(m_byFriend) F(MonsterSkill) F(ID) F(Movement) F(MovementType) F(CollisionTime)               \
    F(GuildMarkIndex) F(Key) F(TargetCharacter) F(Level) F(MonsterIndex) F(Damage) F(Hit) F(MoveSpeed)               \
    F(AttackSpeed) F(MagicSpeed) F(Action) F(LongRangeAttack) F(SelectItem) F(Item) F(FreezeType) F(PriorPositionX)  \
    F(PriorPositionY) F(PositionX) F(PositionY) F(m_iFenrirSkillTarget) F(LastAttackEffectTime) F(m_iDeleteTime)     \
    F(LastCritDamageEffect) F(ExtendStateTime) F(JumpTime) F(StormTime) F(AttackTime) F(ProtectGuildMarkWorldTime)   \
    F(AttackRange) F(Freeze) F(Duplication) F(Rot) F(Run) F(HealthStatus) F(ShieldStatus) F(TargetPosition)          \
    F(Light) F(BodyPart) F(Weapon) F(Wing) F(Helper) F(Flag) F(Path) F(m_pParts) F(m_pPet) F(m_PetInfo) F(OwnerID)   \
    F(m_pTempParts) F(m_iTempKey) F(m_CursedTempleCurSkill) F(m_CursedTempleCurSkillPacket) F(GensRanking)           \
    F(GensContributionPoints)

#define RECORDER_PART_FIELDS(F)                                                                                      \
    F(Type) F(Level) F(ExcellentFlags) F(AncientDiscriminator) F(LinkBone) F(CurrentAction) F(PriorAction)           \
    F(AnimationFrame) F(PriorAnimationFrame) F(PlaySpeed) F(m_byNumCloth) F(m_pCloth)

// Without its SpinLock, which no game code leaves held.
#define RECORDER_PATH_FIELDS(F)                                                                                      \
    F(CurrentPath) F(CurrentPathFloat) F(PathNum) F(PathX) F(PathY) F(Success) F(Error) F(x) F(y) F(Direction)       \
    F(Run) F(Count)

#define RECORDER_PET_INFO_FIELDS(F)                                                                                  \
    F(m_dwPetType) F(m_dwExp1) F(m_dwExp2) F(m_wLevel) F(m_wLife) F(m_wDamageMin) F(m_wDamageMax)                    \
    F(m_wAttackSpeed) F(m_wAttackSuccess)
// clang-format on

// A member added to one of the structs changes its size, so the build stops
// until the lists above have it too. The sizes are those of the 64-bit
// Windows build without iterator debugging (OBJECT holds a std::map and
// std::vectors, whose size depends on it).
#if defined(_MSC_VER) && defined(_WIN64) && defined(_ITERATOR_DEBUG_LEVEL) && _ITERATOR_DEBUG_LEVEL == 0
static_assert(sizeof(OBJECT) == 776, "OBJECT changed: update RECORDER_OBJECT_FIELDS");
static_assert(sizeof(PARTICLE) == 136, "PARTICLE changed: update RECORDER_PARTICLE_FIELDS");
static_assert(sizeof(JOINT) == 9800, "JOINT changed: update RECORDER_JOINT_FIELDS");
static_assert(sizeof(CHARACTER) == 1896, "CHARACTER changed: update RECORDER_CHARACTER_FIELDS");
static_assert(sizeof(PART_t) == 48, "PART_t changed: update RECORDER_PART_FIELDS");
static_assert(sizeof(PATH_t) == 48, "PATH_t changed: update RECORDER_PATH_FIELDS");
static_assert(sizeof(PET_INFO) == 24, "PET_INFO changed: update RECORDER_PET_INFO_FIELDS");
#endif

namespace RecorderFields
{
// Calls visitor(name, field of `target`, same field of `reference`) for every field.
template <typename Visitor> void VisitFields(OBJECT& target, const OBJECT& reference, Visitor&& visitor)
{
#define RECORDER_VISIT(name) visitor(#name, target.name, reference.name);
    RECORDER_OBJECT_FIELDS(RECORDER_VISIT)
#undef RECORDER_VISIT
}

template <typename Visitor> void VisitFields(PARTICLE& target, const PARTICLE& reference, Visitor&& visitor)
{
#define RECORDER_VISIT(name) visitor(#name, target.name, reference.name);
    RECORDER_PARTICLE_FIELDS(RECORDER_VISIT)
#undef RECORDER_VISIT
}

template <typename Visitor> void VisitFields(JOINT& target, const JOINT& reference, Visitor&& visitor)
{
#define RECORDER_VISIT(name) visitor(#name, target.name, reference.name);
    RECORDER_JOINT_FIELDS(RECORDER_VISIT)
#undef RECORDER_VISIT
}

template <typename Visitor> void VisitFields(CHARACTER& target, const CHARACTER& reference, Visitor&& visitor)
{
#define RECORDER_VISIT(name) visitor(#name, target.name, reference.name);
    RECORDER_CHARACTER_FIELDS(RECORDER_VISIT)
#undef RECORDER_VISIT
}

template <typename Visitor> void VisitFields(PART_t& target, const PART_t& reference, Visitor&& visitor)
{
#define RECORDER_VISIT(name) visitor(#name, target.name, reference.name);
    RECORDER_PART_FIELDS(RECORDER_VISIT)
#undef RECORDER_VISIT
}

template <typename Visitor> void VisitFields(PATH_t& target, const PATH_t& reference, Visitor&& visitor)
{
#define RECORDER_VISIT(name) visitor(#name, target.name, reference.name);
    RECORDER_PATH_FIELDS(RECORDER_VISIT)
#undef RECORDER_VISIT
}

template <typename Visitor> void VisitFields(PET_INFO& target, const PET_INFO& reference, Visitor&& visitor)
{
#define RECORDER_VISIT(name) visitor(#name, target.name, reference.name);
    RECORDER_PET_INFO_FIELDS(RECORDER_VISIT)
#undef RECORDER_VISIT
}
} // namespace RecorderFields
