#ifndef __ZZCHARACTER_H__
#define __ZZCHARACTER_H__

#include "Engine/Object/CharacterAttachments.h"
#include "Render/Models/ZzzBMD.h"
#include <span>

extern CHARACTER* CharactersClient;
extern CHARACTER CharacterView;
extern CHARACTER* Hero;

DWORD GetGuildRelationShipTextColor(BYTE GuildRelationShip);
DWORD GetGuildRelationShipBGColor(BYTE GuildRelationShip);

CHARACTER* FindCharacterByID(wchar_t* szName);
CHARACTER* FindCharacterByKey(int Key);

// A linked part's own scale and light, where they are not its owner's.
struct LinkedPartLook
{
    float ownerScale;
    const float* light;
};
// Draws a linked part and returns the bones it posed it into.
vec34_t* RenderLinkObject(float x, float y, float z, const CHARACTER* c, const PART_t* f, int Type, int Level,
                          int Option1, bool Link, bool Translate, int RenderType = 0, bool bRightHandItem = true,
                          int linkBone = -1, const LinkedPartLook* look = nullptr);
bool RendersWingCarriedModels(const CHARACTER* c);
// A model a Red Dragon or a Saint statue carries on its wing part: where it hangs, and the character's scale for it.
struct WingCarriedModel
{
    int type;
    float x;
    float y;
    float z;
    bool link;
    int bone;
    float scale;
};
int WingCarriedModels(const CHARACTER* c, WingCarriedModel (&models)[2]);
void RenderWingCarriedModels(const CHARACTER* c, bool Translate);
void RenderChristmasCompanions(const CHARACTER* c, vec3_t light, bool Translate, int Select);
void RenderCharacter(const CHARACTER* c, const OBJECT* o, int Select = 0);
void RenderCharactersClient();
// Whether RenderCharactersClient draws the character this frame.
bool IsCharacterDrawn(CHARACTER* c);
// The effects, sounds and state changes of the characters drawn this frame, after every Move* of the frame: their
// bones are final, and what they spawn is drawn this frame.
void UpdateCharacterEffects();
// A preview's update (a character outside CharactersClient, tracked by Engine::Object::Attachments::Track): its parts,
// cloth and transformation, which its draw only draws. Its owner calls it in its update, before drawing it.
void UpdatePreviewCharacter(CHARACTER* c);
// What a character's draw used to set while drawing it, set before the draw: its light, a Doppelganger's look and
// rise, a Blood Castle corpse held to the ground, the blend mesh lights it draws with.
void PrepareCharacterForDraw(CHARACTER* c);
// PrepareCharacterForDraw for the client characters drawn this frame; also clears a target the siege cloak hides and
// moves a hidden GM's pick box away. Part of CullWorld.
void PrepareCharactersForDraw();
// Hands the camera a shake UpdateCharacterEffects requested last frame; call before the frame's damping.
void ApplyCharacterEffectsEarthQuake();
// A shake the update requests for the camera after its next damping, as when the draw set it; a character effect's
// shake wins, as the characters drew later.
void RequestEarthQuake(float value);

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
void AnimateCharacterBones(OBJECT* o);

extern size_t g_LastActiveCharacterCount;
extern bool g_LastAnimationWasParallel;

#ifdef MU_ENABLE_CONTROL_SOCKET
// The poses drawn client characters reuse, against a fresh animation of their state, while enabled.
struct PrepassBoneCheck
{
    bool enabled = false;
    int checked = 0;
    int players = 0;
    int mismatched = 0;
    // Draws that animated the character again: its state changed after the update built the pose.
    int rebuilt = 0;
    // Draws of a client character's own part: reusing the update's animation step, or advancing it themselves
    // (an attachment's first frame; steadily rising when update and draw disagree).
    int partsReused = 0;
    int partsDrawnUnadvanced = 0;
    float maxDelta = 0.f;
};
extern PrepassBoneCheck g_PrepassBoneCheck;
void RecordPrepassBones(const float (*prepass)[3][4], const float (*rendered)[3][4], int numBones, bool player);
// Called by a draw that reused o's pose, or animated o again.
void CheckCharacterPose(const OBJECT* o, BMD* model, bool reused);
#endif

void MoveEye(OBJECT* o, BMD* b, int Right, int Left, int Right2 = -1, int Left2 = -1, int Right3 = -1, int Left3 = -1);
void DeleteCloth(CHARACTER* c, OBJECT* o = NULL, PART_t* p2 = NULL);

// Whether c wears a full set; also sets EquipmentLevelSet and g_bAddDefense, and in Chaos Castle its unit.
bool CheckFullSet(CHARACTER* c);
// The same check for a draw: it sets nothing.
[[nodiscard]] bool IsFullSet(const CHARACTER* c);

void MoveCharacterPosition(CHARACTER* c);
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

bool CharacterAnimation(CHARACTER* c, OBJECT* o);
bool AttackStage(CHARACTER* c, OBJECT* o);

void RenderGuild(const OBJECT* o, int Type = -1, vec3_t vPos = NULL);
void SpawnBoneLight(OBJECT* o, int Texture, float Scale, int Bone, float x, float y, float z);
void SpawnProtectGuildMark(CHARACTER* c);

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
// Whether the character carries its weapons on its back instead of in hand.
bool AreWeaponsOnBack(const CHARACTER* c, const OBJECT* o);
bool RenderCharacterBackItem(const CHARACTER* c, const OBJECT* o, bool bTranslate);
bool DrawsWeaponOnBack(const CHARACTER* c, const OBJECT* o, int index);
void PrepareCarriedParts(CHARACTER* c);
bool IsFlying(const OBJECT* o);
// The capes that hang from the shoulders (bone 19) and link to the body.
bool IsLinkedCape(int type);
// The Archangel quest item a Blood Castle player carries on the back, or 0.
int BloodCastleQuestItem(const CHARACTER* c);
// The quest item plays on the wing part, at its own speed.
float QuestItemPlaySpeed(const OBJECT* o);
// A Lorencia skeleton guard, which carries a golden crossbow on the back.
bool IsSkeletonGuard(const OBJECT* o);
// The model RenderCharacterBackItem shows for weapon `index` on the back.
int BackWeaponType(const CHARACTER* c, const OBJECT* o, int index);
// The part a weapon on the back is drawn with: the wing's, at rest.
PART_t BackWeaponPart(const CHARACTER* c);
// Where the Imp hangs: higher on a Rage Fighter's armor.
void ImpOffset(const CHARACTER* c, vec3_t offset);
// Whether RenderCharacter gets to the character's attachments.
bool DrawsAttachments(const CHARACTER* c);
bool IsBackItem(const CHARACTER* c, int iType);

bool IsPlayer(CHARACTER* c);
bool IsMonster(CHARACTER* c);

#endif