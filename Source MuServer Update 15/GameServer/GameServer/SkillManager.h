// SkillManager.h: interface for the CSkillManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DefaultClassInfo.h"
#include "Protocol.h"
#include "User.h"

#if(GAMESERVER_UPDATE>=602)
#define MAX_SKILL 622
#else
#define MAX_SKILL 555
#endif
#if(GAMESERVER_UPDATE>=701)
#define MAX_SKILL_LIST 60
#define MAX_MASTER_SKILL_LIST 120
#else
#define MAX_SKILL_LIST 60
#define MAX_MASTER_SKILL_LIST 60
#endif

#define CHECK_SKILL_ATTACK_COUNT(x) (((++x)>=10)?0:1)
#define CHECK_SKILL_ATTACK_EXTENDED_COUNT(x) (((++x)>=20)?0:1)

enum eSkillNumber
{
	SKILL_NONE = 0,
	SKILL_POISON = 1,
	SKILL_METEORITE = 2,
	SKILL_LIGHTNING = 3,
	SKILL_FIRE_BALL = 4,
	SKILL_FLAME = 5,
	SKILL_TELEPORT = 6,
	SKILL_ICE = 7,
	SKILL_TWISTER = 8,
	SKILL_EVIL_SPIRIT = 9,
	SKILL_HELL_FIRE = 10,
	SKILL_POWER_WAVE = 11,
	SKILL_AQUA_BEAM = 12,
	SKILL_BLAST = 13,
	SKILL_INFERNO = 14,
	SKILL_TELEPORT_ALLY = 15,
	SKILL_MANA_SHIELD = 16,
	SKILL_ENERGY_BALL = 17,
	SKILL_DEFENSE = 18,
	SKILL_FALLING_SLASH = 19,
	SKILL_LUNGE = 20,
	SKILL_UPPERCUT = 21,
	SKILL_CYCLONE = 22,
	SKILL_SLASH = 23,
	SKILL_TRIPLE_SHOT = 24,
	SKILL_HEAL = 26,
	SKILL_GREATER_DEFENSE = 27,
	SKILL_GREATER_DAMAGE = 28,
	SKILL_SUMMON1 = 30,
	SKILL_SUMMON2 = 31,
	SKILL_SUMMON3 = 32,
	SKILL_SUMMON4 = 33,
	SKILL_SUMMON5 = 34,
	SKILL_SUMMON6 = 35,
	SKILL_SUMMON7 = 36,
	SKILL_DECAY = 38,
	SKILL_ICE_STORM = 39,
	SKILL_NOVA = 40,
	SKILL_TWISTING_SLASH = 41,
	SKILL_RAGEFUL_BLOW = 42,
	SKILL_DEATH_STAB = 43,
	SKILL_CRESCENT_MOON_SLASH = 44,
	SKILL_MANA_GLAIVE = 45,
	SKILL_STAR_FALL = 46,
	SKILL_IMPALE = 47,
	SKILL_GREATER_LIFE = 48,
	SKILL_FIRE_BREATH = 49,
	SKILL_MONSTER_AREA_ATTACK = 50,
	SKILL_ICE_ARROW = 51,
	SKILL_PENETRATION = 52,
	SKILL_FIRE_SLASH = 55,
	SKILL_POWER_SLASH = 56,
	SKILL_SPIRAL_SLASH = 57,
	SKILL_NOVA_START = 58,
	SKILL_COMBO = 59,
	SKILL_FORCE = 60,
	SKILL_FIRE_BURST = 61,
	SKILL_EARTHQUAKE = 62,
	SKILL_SUMMON_PARTY = 63,
	SKILL_GREATER_CRITICAL_DAMAGE = 64,
	SKILL_ELECTRIC_SPARK = 65,
	SKILL_FORCE_WAVE = 66,
	SKILL_STERN = 67,
	SKILL_REMOVE_STERN = 68,
	SKILL_GREATER_MANA = 69,
	SKILL_INVISIBILITY = 70,
	SKILL_REMOVE_INVISIBILITY = 71,
	SKILL_REMOVE_ALL_EFFECT = 72,
	SKILL_MANA_RAYS = 73,
	SKILL_FIRE_BLAST = 74,
	SKILL_BRAND = 75,
	SKILL_PLASMA_STORM = 76,
	SKILL_INFINITY_ARROW = 77,
	SKILL_FIRE_SCREAM = 78,
	SKILL_EXPLOSION = 79,
	SKILL_MONSTER_SUMMON = 200,
	SKILL_MAGIC_DAMAGE_IMMUNITY = 201,
	SKILL_PHYSI_DAMAGE_IMMUNITY = 202,
	SKILL_ORDER_OF_PROTECTION = 210,
	SKILL_ORDER_OF_RESTRAINT = 211,
	SKILL_ORDER_OF_TRACKING = 212,
	SKILL_ORDER_OF_WEAKEN = 213,
	SKILL_DRAIN_LIFE = 214,
	SKILL_CHAIN_LIGHTNING = 215,
	SKILL_ELECTRIC_SURGE = 216,
	SKILL_DAMAGE_REFLECT = 217,
	SKILL_SWORD_POWER = 218,
	SKILL_SLEEP = 219,
	SKILL_LESSER_DAMAGE = 221,
	SKILL_LESSER_DEFENSE = 222,
	SKILL_SAHAMUTT = 223,
	SKILL_NEIL = 224,
	SKILL_GHOST_PHANTOM = 225,
	SKILL_RED_STORM = 230,
	SKILL_FROZEN_STAB = 232,
	SKILL_MAGIC_CIRCLE = 233,
	SKILL_SHIELD_RECOVER = 234,
	SKILL_FIVE_SHOT = 235,
	SKILL_SWORD_SLASH = 236,
	SKILL_LIGHTNING_STORM = 237,
	SKILL_BIRDS = 238,
	SKILL_SELF_EXPLOSION = 239,
	SKILL_RAKLION_SELUPAN1 = 250,
	SKILL_RAKLION_SELUPAN2 = 251,
	SKILL_RAKLION_SELUPAN3 = 252,
	SKILL_RAKLION_SELUPAN4 = 253,
	SKILL_LARGE_RING_BLOWER = 260,
	SKILL_UPPER_BEAST = 261,
	SKILL_CHAIN_DRIVER = 262,
	SKILL_DARK_SIDE = 263,
	SKILL_DRAGON_LORE = 264,
	SKILL_DRAGON_SLAYER = 265,
	SKILL_GREATER_IGNORE_DEFENSE_RATE = 266,
	SKILL_FITNESS = 267,
	SKILL_GREATER_DEFENSE_SUCCESS_RATE = 268,
	SKILL_CHARGE = 269,
	SKILL_PHOENIX_SHOT = 270,
	SKILL_BLOOD_STORM = 344,
	SKILL_CURE = 425,
	SKILL_PARTY_HEAL = 426,
	SKILL_POISON_ARROW = 427,
	SKILL_BLESS = 430,
	SKILL_SUMMON_SATYROS = 432,
	SKILL_BLIND = 461,
	SKILL_EARTH_PRISON = 495,
	SKILL_IRON_DEFENSE = 521,
	SKILL_BLOOD_HOWLING = 565,
	SKILL_SPROTECTION = 618,
	SKILL_SRESTRICTION = 619,
	SKILL_SPURSUIT = 620,
	SKILL_SSHIELD_BURN = 621,
};

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_MULTI_SKILL_ATTACK_RECV
{
	PBMSG_HEAD header; // C3:[PROTOCOL_CODE4]
	#if(GAMESERVER_UPDATE>=701)
	BYTE skillH;
	BYTE count;
	BYTE skillL;
	BYTE x;
	BYTE serial;
	BYTE y;
	#else
	BYTE skill[2];
	BYTE x;
	BYTE y;
	BYTE serial;
	BYTE count;
	#endif
};

struct PMSG_MULTI_SKILL_ATTACK
{
	#if(GAMESERVER_UPDATE>=701)
	BYTE indexH;
	BYTE MagicKey;
	BYTE indexL;
	#else
	BYTE index[2];
	BYTE MagicKey;
	#endif
};

struct PMSG_SKILL_ATTACK_RECV
{
	PBMSG_HEAD header; // C3:19
	#if(GAMESERVER_UPDATE>=701)
	BYTE indexH;
	BYTE skillH;
	BYTE indexL;
	BYTE skillL;
	BYTE dis;
	#else
	BYTE skill[2];
	BYTE index[2];
	BYTE dis;
	#endif
};

struct PMSG_SKILL_CANCEL_RECV
{
	PBMSG_HEAD header; // C3:1B
	BYTE skill[2];
	BYTE index[2];
};

struct PMSG_DURATION_SKILL_ATTACK_RECV
{
	PBMSG_HEAD header; // C3:1E
	#if(GAMESERVER_UPDATE>=701)
	BYTE x;
	BYTE skillH;
	BYTE y;
	BYTE skillL;
	BYTE dir;
	BYTE indexH;
	BYTE dis;
	BYTE indexL;
	BYTE angle;
	BYTE MagicKey;
	#else
	BYTE skill[2];
	BYTE x;
	BYTE y;
	BYTE dir;
	BYTE dis;
	BYTE angle;
	BYTE index[2];
	BYTE MagicKey;
	#endif
};

struct PMSG_RAGE_FIGHTER_SKILL_ATTACK_RECV
{
	PBMSG_HEAD header; // C1:4A
	#if(GAMESERVER_UPDATE>=701)
	BYTE indexH;
	BYTE skillH;
	BYTE indexL;
	BYTE skillL;
	BYTE dis;
	#else
	BYTE skill[2];
	BYTE dis;
	BYTE index[2];
	#endif
};

struct PMSG_SKILL_DARK_SIDE_RECV
{
	PBMSG_HEAD header; // C1:4B
	BYTE skill[2];
	BYTE index[2];
};

struct PMSG_SKILL_TELEPORT_ALLY_RECV
{
	#pragma pack(1)
	PBMSG_HEAD header; // C3:B0
	WORD index;
	BYTE x;
	BYTE y;
	#pragma pack()
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_SKILL_ATTACK_SEND
{
	PBMSG_HEAD header; // C3:19
	#if(GAMESERVER_UPDATE>=701)
	BYTE index[2];
	BYTE skill[2];
	BYTE target[2];
	#else
	BYTE skill[2];
	BYTE index[2];
	BYTE target[2];
	#endif
};

struct PMSG_SKILL_CANCEL_SEND
{
	PBMSG_HEAD header; // C1:1B
	BYTE skill[2];
	BYTE index[2];
};

struct PMSG_DURATION_SKILL_ATTACK_SEND
{
	PBMSG_HEAD header; // C3:1E
	#if(GAMESERVER_UPDATE>=701)
	BYTE x;
	BYTE y;
	BYTE dir;
	BYTE skillH;
	BYTE indexH;
	BYTE skillL;
	BYTE indexL;
	#else
	BYTE skill[2];
	BYTE index[2];
	BYTE x;
	BYTE y;
	BYTE dir;
	#endif
};

struct PMSG_RAGE_FIGHTER_SKILL_ATTACK_SEND
{
	PBMSG_HEAD header; // C3:4A
	#if(GAMESERVER_UPDATE>=701)
	BYTE index[2];
	BYTE skill[2];
	BYTE target[2];
	#else
	BYTE skill[2];
	BYTE index[2];
	BYTE target[2];
	#endif
};

struct PMSG_SKILL_DARK_SIDE_SEND
{
	PBMSG_HEAD header; // C3:4B
	WORD skill;
	WORD index[5];
};

struct PMSG_SKILL_NOVA_SEND
{
	PBMSG_HEAD header; // C1:BA
	BYTE index[2];
	BYTE type;
	BYTE count;
};

struct PMSG_SKILL_CHAIN_SKILL_SEND
{
	PSBMSG_HEAD header; // C1:BF:[0A:18]
	BYTE skill[2];
	WORD index;
	BYTE count;
};

struct PMSG_SKILL_CHAIN_SKILL
{
	WORD index;
};

struct PMSG_SKILL_LIST_SEND
{
	PSBMSG_HEAD header; // C1:F3:11
	BYTE count;
	BYTE type;
};

struct PMSG_SKILL_LIST
{
	#pragma pack(1)
	BYTE slot;
	WORD skill;
	BYTE level;
	#pragma pack()
};

//**********************************************//
//**********************************************//
//**********************************************//

struct SKILL_INFO
{
	int Index;
	char Name[32];
	int Damage;
	int Mana;
	int BP;
	int Range;
	int Radio;
	int Delay;
	int Type;
	int Effect;
	int RequireLevel;
	int RequireEnergy;
	int RequireLeadership;
	int RequireKillCount;
	int RequireGuildStatus;
	int RequireClass[MAX_CLASS];
};

class CSkillManager
{
public:
	CSkillManager();
	virtual ~CSkillManager();
	void Load(char* path);
	bool GetInfo(int index,SKILL_INFO* lpInfo);
	int GetSkillDamage(int index);
	int GetSkillMana(int index);
	int GetSkillBP(int index);
	int GetSkillType(int index);
	int GetSkillEffect(int index);
	int GetSkillAngle(int x,int y,int tx,int ty);
	int GetSkillFrustrum(int* SkillFrustrumX,int* SkillFrustrumY,BYTE angle,int x,int y,float sx,float sy,float tx,float ty);
	int GetSkillNumber(int index,int level);
	bool CheckSkillMana(LPOBJ lpObj,int index);
	bool CheckSkillBP(LPOBJ lpObj,int index);
	bool CheckSkillRange(int index,int x,int y,int tx,int ty);
	bool CheckSkillRadio(int index,int x,int y,int tx,int ty);
	bool CheckSkillFrustrum(int* SkillFrustrumX,int* SkillFrustrumY,int x,int y);
	bool CheckSkillDelay(LPOBJ lpObj,int index);
	bool CheckSkillTarget(LPOBJ lpObj,int aIndex,int bIndex,int type);
	bool CheckSkillRequireLevel(LPOBJ lpObj,int index);
	bool CheckSkillRequireEnergy(LPOBJ lpObj,int index);
	bool CheckSkillRequireLeadership(LPOBJ lpObj,int index);
	bool CheckSkillRequireKillPoint(LPOBJ lpObj,int index);
	bool CheckSkillRequireGuildStatus(LPOBJ lpObj,int index);
	bool CheckSkillRequireClass(LPOBJ lpObj,int index);
	bool CheckSkillRequireWeapon(LPOBJ lpObj,int index);
	int AddSkillWeapon(LPOBJ lpObj,int index,int level);
	int DelSkillWeapon(LPOBJ lpObj,int index,int level);
	int AddSkill(LPOBJ lpObj,int index,int level);
	int DelSkill(LPOBJ lpObj,int index);
	CSkill* GetSkill(LPOBJ lpObj,int index);
	int AddMasterSkill(LPOBJ lpObj,int index,int level);
	int DelMasterSkill(LPOBJ lpObj,int index);
	CSkill* GetMasterSkill(LPOBJ lpObj,int index);
	bool ConvertSkillByte(CSkill* lpSkill,BYTE* lpMsg);
	void SkillByteConvert(BYTE* lpMsg,CSkill* lpSkill);
	void UseAttackSkill(int aIndex,int bIndex,CSkill* lpSkill);
	void UseDurationSkillAttack(int aIndex,int bIndex,CSkill* lpSkill,BYTE x,BYTE y,BYTE dir,BYTE angle);
	bool RunningSkill(int aIndex,int bIndex,CSkill* lpSkill,BYTE x,BYTE y,BYTE angle,bool combo);
	bool BasicSkillAttack(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool MultiSkillAttack(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillChangeUse(int aIndex);
	bool SkillEvilSpirit(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillManaShield(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillDefense(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillHeal(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillGreaterDefense(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillGreaterDamage(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillSummon(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillDecay(int aIndex,int bIndex,CSkill* lpSkill,BYTE x,BYTE y,bool combo);
	bool SkillIceStorm(int aIndex,int bIndex,CSkill* lpSkill,BYTE x,BYTE y,bool combo);
	bool SkillNova(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillTwistingSlash(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillRagefulBlow(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillDeathStab(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillGreaterLife(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillMonsterAreaAttack(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillPowerSlash(int aIndex,int bIndex,CSkill* lpSkill,BYTE angle,bool combo);
	bool SkillForce(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillFireBurst(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillEarthquake(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillSummonParty(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillGreaterCriticalDamage(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillElectricSpark(int aIndex,int bIndex,CSkill* lpSkill,BYTE angle,bool combo);
	bool SkillCastleSiege(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillStern(int aIndex,int bIndex,CSkill* lpSkill,BYTE angle);
	bool SkillRemoveStern(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillGreaterMana(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillInvisibility(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillRemoveInvisibility(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillRemoveAllEffect(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillBrand(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillPlasmaStorm(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillInfinityArrow(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillMonsterSummon(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillMagicDamageImmunity(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillPhysiDamageImmunity(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillOrderOfProtection(int aIndex,int bIndex);
	bool SkillOrderOfRestraint(int aIndex,int bIndex);
	bool SkillOrderOfTracking(int aIndex,int bIndex);
	bool SkillOrderOfWeaken(int aIndex,int bIndex);
	bool SkillChainLightning(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillDamageReflect(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillSwordPower(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillSwordPowerGetDefense(int aIndex,int* defense);
	bool SkillSwordPowerGetPhysiDamage(int aIndex,int* DamageMin,int* DamageMax);
	bool SkillSwordPowerGetMagicDamage(int aIndex,int* DamageMin,int* DamageMax);
	bool SkillSwordPowerGetCurseDamage(int aIndex,int* DamageMin,int* DamageMax);
	bool SkillSleep(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillLesserDefense(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillLesserDamage(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillSahamutt(int aIndex,int bIndex,CSkill* lpSkill,BYTE x,BYTE y,bool combo);
	bool SkillNeil(int aIndex,int bIndex,CSkill* lpSkill,BYTE x,BYTE y);
	bool SkillGhostPhantom(int aIndex,int bIndex,CSkill* lpSkill,BYTE x,BYTE y);
	bool SkillGhostPhantomAreaAttack(int aIndex,int bIndex,CSkill* lpSkill,BYTE x,BYTE y);
	bool SkillRedStorm(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillFrozenStab(int aIndex,int bIndex,CSkill* lpSkill,BYTE x,BYTE y,bool combo);
	bool SkillMagicCircle(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillShieldRecover(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillFiveShot(int aIndex,int bIndex,CSkill* lpSkill,BYTE angle,bool combo);
	bool SkillSwordSlash(int aIndex,int bIndex,CSkill* lpSkill,BYTE angle,bool combo);
	bool SkillLightningStorm(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillBirds(int aIndex,int bIndex,CSkill* lpSkill,BYTE angle,bool combo);
	bool SkillSelfExplosion(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillLargeRingBlower(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillUpperBeast(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillChainDriver(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillDarkSide(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillDarkSideGetTargetIndex(int aIndex,int bIndex,CSkill* lpSkill,WORD* target);
	bool SkillDragonLore(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillGreaterIgnoreDefenseRate(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillFitness(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillGreaterDefenseSuccessRate(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillPhoenixShot(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillBloodStorm(int aIndex,int bIndex,CSkill* lpSkill,BYTE x,BYTE y,bool combo);
	bool SkillCure(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillPartyHeal(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillPoisonArrow(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillBless(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillBlind(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillEarthPrison(int aIndex,int bIndex,CSkill* lpSkill,bool combo);
	bool SkillIronDefense(int aIndex,int bIndex,CSkill* lpSkill);
	bool SkillBloodHowling(int aIndex,int bIndex,CSkill* lpSkill);
	void ApplyMeteoriteEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyIceStormEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyTwistingSlashEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyRagefulBlowEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyDeathStabEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyFireSlashEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyFireBurstEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyPlasmaStormEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyFireScreamEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyEarthquakeEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyDrainLifeEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplySahamuttEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyNeilEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyGhostPhantomEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyFrozenStabEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyFiveShotEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplySwordSlashEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyLargeRingBlowerEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyUpperBeastEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyChainDriverEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyDragonLoreEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyDragonSlayerEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyPhoenixShotEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void ApplyEarthPrisonEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	void CGMultiSkillAttackRecv(PMSG_MULTI_SKILL_ATTACK_RECV* lpMsg,int aIndex,bool type);
	void CGSkillAttackRecv(PMSG_SKILL_ATTACK_RECV* lpMsg,int aIndex);
	void CGDurationSkillAttackRecv(PMSG_DURATION_SKILL_ATTACK_RECV* lpMsg,int aIndex);
	void CGSkillCancelRecv(PMSG_SKILL_CANCEL_RECV* lpMsg,int aIndex);
	void CGRageFighterSkillAttackRecv(PMSG_RAGE_FIGHTER_SKILL_ATTACK_RECV* lpMsg,int aIndex);
	void CGSkillDarkSideRecv(PMSG_SKILL_DARK_SIDE_RECV* lpMsg,int aIndex);
	void CGSkillTeleportAllyRecv(PMSG_SKILL_TELEPORT_ALLY_RECV* lpMsg,int aIndex);
	void GCSkillAttackSend(LPOBJ lpObj,int skill,int aIndex,BYTE type);
	void GCSkillCancelSend(LPOBJ lpObj,int skill);
	void GCDurationSkillAttackSend(LPOBJ lpObj,int skill,BYTE x,BYTE y,BYTE dir);
	void GCRageFighterSkillAttackSend(LPOBJ lpObj,int skill,int aIndex,BYTE type);
	void GCSkillAddSend(int aIndex,BYTE slot,int skill,BYTE level,BYTE type);
	void GCSkillDelSend(int aIndex,BYTE slot,int skill,BYTE level,BYTE type);
	void GCSkillListSend(LPOBJ lpObj,BYTE type);
private:
	std::map<int,SKILL_INFO> m_SkillInfo;
};

extern CSkillManager gSkillManager;
