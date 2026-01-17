#ifndef __ZZCHARACTER_H__
#define __ZZCHARACTER_H__

#include "ZzzBMD.h"

extern Script_Skill MonsterSkill[];
extern CHARACTER* CharactersClient;
extern CHARACTER CharacterView;
extern CHARACTER* Hero;

DWORD GetGuildRelationShipTextColor(BYTE GuildRelationShip);
DWORD GetGuildRelationShipBGColor(BYTE GuildRelationShip);

CHARACTER* FindCharacterByID(wchar_t* szName);
CHARACTER* FindCharacterByKey(int Key);

void RenderLinkObject(float x, float y, float z, CHARACTER* c, PART_t* f, int Type, int Level, int Option1, bool Link, bool Translate, int RenderType = 0, bool bRightHandItem = true);
void RenderCharacter(CHARACTER* c, OBJECT* o, int Select = 0);
void RenderCharactersClient();
void MoveCharacterClient(CHARACTER* cc);
void MoveCharactersClient();

void MoveEye(OBJECT* o, BMD* b, int Right, int Left, int Right2 = -1, int Left2 = -1, int Right3 = -1, int Left3 = -1);
void DeleteCloth(CHARACTER* c, OBJECT* o = NULL, PART_t* p2 = NULL);

bool CheckFullSet(CHARACTER* c);

void MoveCharacterPosition(CHARACTER* c);
void ChangeCharacterExt(int Key, BYTE* Equipment, CHARACTER* pCharacter = NULL, OBJECT* pHelper = NULL);
void ReadEquipmentExtended(int Key, BYTE flags, BYTE* Equipment, CHARACTER* pCharacter = nullptr, OBJECT* pHelper = nullptr);
void ClearCharacters(int Key = -1);
void DeleteCharacter(int Key);
void DeleteCharacter(CHARACTER* c, OBJECT* o);
int FindCharacterIndex(int Key);
int FindCharacterIndexByMonsterIndex(int Type);

void DeadCharacterBuff(OBJECT* o);

int  HangerBloodCastleQuestItem(int Key);
void SetAllAction(int Action);

void ReleaseCharacters(void);
void CreateCharacterPointer(CHARACTER* c, int Type, unsigned char PositionX, unsigned char PositionY, float Rotation = 0.f);
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

bool CheckMonsterSkill(CHARACTER* c, OBJECT* o);
bool CharacterAnimation(CHARACTER* c, OBJECT* o);
bool AttackStage(CHARACTER* c, OBJECT* o);

void RenderGuild(OBJECT* o, int Type = -1, vec3_t vPos = NULL);
void RenderLight(OBJECT* o, int Texture, float Scale, int Bone, float x, float y, float z);
void RenderProtectGuildMark(CHARACTER* c);

void MakeElfHelper(CHARACTER* c);
int GetFenrirType(CHARACTER* c);

extern int       EquipmentLevelSet;
extern bool      g_bAddDefense;

void CreateJoint(int Type, vec3_t Position, vec3_t TargetPosition, vec3_t Angle, int SubType = 0, OBJECT* Target = NULL, float Scale = 10.f, short PK = -1, WORD SkillIndex = 0, WORD SkillSerialNum = 0, int iChaIndex = -1, const float* vColor = NULL, short int sTargetIndex = -1);
void CreateJointFpsChecked(int Type, vec3_t Position, vec3_t TargetPosition, vec3_t Angle, int SubType = 0, OBJECT* Target = NULL, float Scale = 10.f, short PK = -1, WORD SkillIndex = 0, WORD SkillSerialNum = 0, int iChaIndex = -1, const float* vColor = NULL, short int sTargetIndex = -1);
bool RenderCharacterBackItem(CHARACTER* c, OBJECT* o, bool bTranslate);
bool IsBackItem(CHARACTER* c, int iType);

bool IsPlayer(CHARACTER* c);
bool IsMonster(CHARACTER* c);

#endif