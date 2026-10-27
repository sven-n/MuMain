#ifndef __ZZCHARACTER_H__
#define __ZZCHARACTER_H__

#include "Render/Models/ZzzBMD.h"
#include <span>

extern CHARACTER* CharactersClient;
extern CHARACTER CharacterView;
extern CHARACTER* Hero;

DWORD GetGuildRelationShipTextColor(BYTE GuildRelationShip);
DWORD GetGuildRelationShipBGColor(BYTE GuildRelationShip);

CHARACTER* FindCharacterByID(wchar_t* szName);
CHARACTER* FindCharacterByKey(int Key);

void RenderLinkObject(float x, float y, float z, CHARACTER* c, PART_t* f, int Type, int Level, int Option1, bool Link,
                      bool Translate, int RenderType = 0, bool bRightHandItem = true);
void RenderCharacter(CHARACTER* c, OBJECT* o, int Select = 0);
void RenderCharactersClient();

/**
 * @brief Builds a generous axis-aligned pick box for the character selection screen.
 *
 * The model OBB is too tight at the steep camera angle used in CHARACTER_SCENE,
 * so we synthesize a wider box from the character's position and model height.
 * Only used in CHARACTER_SCENE; main scene uses the model's own OBB directly.
 */
void BuildCharacterScenePickOBB(const OBJECT* o, OBB_t& outOBB);
void MoveCharacterClient(CHARACTER* cc);
void MoveCharactersClient();
void UpdateCharactersAnimationParallel(std::span<CHARACTER> characters);
void WaitCharactersAnimation();
// `turnLag`: how far the ponytail trails behind the body's turn (CHARACTER::TurnLag), and `facingOffset` how far the
// drawn body is behind its facing (CHARACTER::SnapTurnOffset), in degrees.
void AnimateCharacterBones(OBJECT* o, float turnLag = 0.f, float facingOffset = 0.f);

extern size_t g_LastActiveCharacterCount;
extern bool g_LastAnimationWasParallel;

#ifdef MU_ENABLE_CONTROL_SOCKET
// Render's recompute against the pre-pass bones of each drawn client character, while enabled.
struct PrepassBoneCheck
{
    bool enabled = false;
    int checked = 0;
    int players = 0;
    int mismatched = 0;
    float maxDelta = 0.f;
};
extern PrepassBoneCheck g_PrepassBoneCheck;
void RecordPrepassBones(const float (*prepass)[3][4], const float (*rendered)[3][4], int numBones, bool player);
// Around render's Animation() call: Begin keeps the pre-pass bones, End compares render's with them.
bool BeginPrepassBoneCheck(const OBJECT* o, int numBones);
void EndPrepassBoneCheck(const OBJECT* o, int numBones);
#endif

void MoveEye(OBJECT* o, BMD* b, int Right, int Left, int Right2 = -1, int Left2 = -1, int Right3 = -1, int Left3 = -1);
void DeleteCloth(CHARACTER* c, OBJECT* o = NULL, PART_t* p2 = NULL);

bool CheckFullSet(CHARACTER* c);

void MoveCharacterPosition(CHARACTER* c);
// Moves the character one frame along `heading` (a walking character's MoveHeading) instead of its facing, at `pace`
// of its speed (StepMomentum's).
void MoveCharacterPosition(CHARACTER* c, float heading, float pace = 1.f);
// A walking character's momentum, one frame: the way it really moves this frame (in `heading`, MoveHeading coming in)
// and the share of its speed it has. Turning back (more than a right angle) it slows to a stop the way it was going
// before it takes the new way; from rest, and after that stop, it speeds up. Gentler turns follow at once.
float StepMomentum(CHARACTER* c, float& heading);
// A flying winged player's bank into a turn and slow up-and-down drift, faded in and out (each frame, everyone).
void UpdateFlightFeel(CHARACTER* c);
// A mount's lean into a turn from its rider's (UpdateFlightFeel leans a rider a share of the mount's).
float MountLeanFromRider(float riderLean);
// The wings and the ponytail trailing a little behind the body's turn and settling after it (each frame, everyone).
void UpdateTurnLag(CHARACTER* c);
// A sudden turn of the facing (an attack snapping to its target) drawn as a quick eased turn instead (each frame,
// everyone): the facing itself, which the attack and its effects aim by, still turns at once.
void UpdateSnapTurn(CHARACTER* c);
// The client character whose object `o` is, or nullptr (a mount, an item, a map object).
CHARACTER* CharacterOfObject(const OBJECT* o);
// Who the character's head looks at, if anyone (each frame): a player looks at what it attacks or the nearest enemy in
// sight in a combat zone, else the nearest NPC, else another player passing by; a monster at its target; an NPC at the
// hero passing by.
const CHARACTER* UpdateLookAt(CHARACTER* c);
// Turns the head toward `target`, at most `maxDegrees` to either side.
void LookTowards(OBJECT* o, const CHARACTER* target, float maxDegrees);
void ChangeCharacterExt(int Key, BYTE* Equipment, CHARACTER* pCharacter = NULL, OBJECT* pHelper = NULL);
void ReadEquipmentExtended(int Key, BYTE flags, BYTE* Equipment, CHARACTER* pCharacter = nullptr,
                           OBJECT* pHelper = nullptr);
void ClearCharacters(int Key = -1);
void DeleteCharacter(int Key);
void DeleteCharacter(CHARACTER* c, OBJECT* o);
int FindCharacterIndex(int Key);
int FindCharacterIndexByMonsterIndex(int Type);

void DeadCharacterBuff(OBJECT* o);

int HangerBloodCastleQuestItem(int Key);
void SetAllAction(int Action);

void ReleaseCharacters(void);
void DeleteCharacterCompanions(CHARACTER* c);
void CreateCharacterPointer(CHARACTER* c, int Type, unsigned char PositionX, unsigned char PositionY,
                            float Rotation = 0.f);
CHARACTER* CreateCharacter(int Key, int Type, unsigned char PositionX, unsigned char PositionY, float Rotation = 0.f);
CHARACTER* CreateHero(int Key, CLASS_TYPE Class, int Skin = 0, float x = 0.f, float y = 0.f, float Rotation = 0.f);
CHARACTER* CreateMonster(EMonsterType Type, int PositionX, int PositionY, int Key = 0);
CHARACTER* CreateHellGate(char* ID, int Key, EMonsterType Index, int x, int y, int CreateFlag);

void SetAttackSpeed();
void SetPlayerShock(CHARACTER* c, int Hit);
void SetPlayerStop(CHARACTER* c);
void SetPlayerWalk(CHARACTER* c);

void SetPlayerAttack(CHARACTER* c);
void SetPlayerDie(CHARACTER* c);
void SetPlayerMagic(CHARACTER* c);
void SetPlayerTeleport(CHARACTER* c);
void SetPlayerHighBowAttack(CHARACTER* c);
void SetCharacterClass(CHARACTER* c);
void SetCharacterScale(CHARACTER* c);
void SetChangeClass(CHARACTER* c);
int LevelConvert(BYTE Level);
float CharacterMoveSpeed(CHARACTER* c);

// Plays the attacks of the monster's entry; Maya plays her scene. False when
// the entry has no attacks.
bool CheckMonsterSkill(CHARACTER* c, OBJECT* o);
bool CharacterAnimation(CHARACTER* c, OBJECT* o);
bool AttackStage(CHARACTER* c, OBJECT* o);

void RenderGuild(OBJECT* o, int Type = -1, vec3_t vPos = NULL);
void RenderLight(OBJECT* o, int Texture, float Scale, int Bone, float x, float y, float z);
void RenderProtectGuildMark(CHARACTER* c);

void MakeElfHelper(CHARACTER* c);
int GetFenrirType(CHARACTER* c);

extern int EquipmentLevelSet;
extern bool g_bAddDefense;

void CreateJoint(int Type, vec3_t Position, vec3_t TargetPosition, vec3_t Angle, int SubType = 0, OBJECT* Target = NULL,
                 float Scale = 10.f, short PK = -1, WORD SkillIndex = 0, WORD SkillSerialNum = 0, int iChaIndex = -1,
                 const float* vColor = NULL, short int sTargetIndex = -1);
void CreateJointFpsChecked(int Type, vec3_t Position, vec3_t TargetPosition, vec3_t Angle, int SubType = 0,
                           OBJECT* Target = NULL, float Scale = 10.f, short PK = -1, WORD SkillIndex = 0,
                           WORD SkillSerialNum = 0, int iChaIndex = -1, const float* vColor = NULL,
                           short int sTargetIndex = -1);
bool RenderCharacterBackItem(CHARACTER* c, OBJECT* o, bool bTranslate);
bool IsBackItem(CHARACTER* c, int iType);

bool IsPlayer(CHARACTER* c);
bool IsMonster(CHARACTER* c);

#endif