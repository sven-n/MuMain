#ifndef ___STRUCT_H__
#define ___STRUCT_H__

#pragma once

#include "w_Buff.h"

#include "w_ObjectInfo.h"
class OBJECT;
#include "w_CharacterInfo.h"
class CHARACTER;

namespace SEASON3B
{
	class CNewUIItemMng;
}

#define MAX_MONSTERSKILL_NUM	10

namespace info
{
	struct Script_Item
	{
		char Name[30];
		bool TwoHand;
		WORD Level;
		BYTE m_byItemSlot;
		BYTE m_bySkillIndex;
		BYTE Width;
		BYTE Height;
		BYTE DamageMin;
		BYTE DamageMax;
		BYTE SuccessfulBlocking;
		BYTE Defense;
		BYTE MagicDefense;
		BYTE WeaponSpeed;
		BYTE WalkSpeed;
		BYTE Durability;
		BYTE MagicDur;
		BYTE MagicPower;
		WORD RequireStrength;
		WORD RequireDexterity;
		WORD RequireEnergy;
		WORD  RequireVitality;
		WORD RequireCharisma;
		WORD RequireLevel;
		BYTE Value;
		int  iZen;
		BYTE  AttType;
		BYTE RequireClass[MAX_CLASS];
		BYTE Resistance[MAX_RESISTANCE];
	};
	
	struct Script_Silde 
	{
		typedef std::vector<char*>     SildeVECTOR;
		SildeVECTOR					   Sildelist;
	};

	struct Script_Dialog
	{
		char m_lpszText[MAX_LENGTH_DIALOG];
		int m_iNumAnswer;
		int m_iLinkForAnswer[MAX_ANSWER_FOR_DIALOG];
		int m_iReturnForAnswer[MAX_ANSWER_FOR_DIALOG];
		char m_lpszAnswer[MAX_ANSWER_FOR_DIALOG][MAX_LENGTH_ANSWER];
	};
	
	struct Script_Credit
	{
		BYTE	byClass;
		char	szName[32];
		//char	szName[CRW_NAME_MAX];
	};

	struct Script_Movereq
	{
		int index;
		char szMainMapName[32];			//. Main map name
		char szSubMapName[32];			//. Substitute map name
		int	iReqLevel;					//. required level
		int iReqZen;					//. required zen
		int iGateNum;					//. Gate number
	};

	struct Script_Quest_Class_Act
	{
		BYTE    chLive;
		BYTE    byQuestType;
		WORD    wItemType;
		BYTE    byItemSubType;
		BYTE	byItemLevel;
		BYTE    byItemNum;
		BYTE    byRequestType;
		BYTE    byRequestClass[MAX_CLASS];
		short   shQuestStartText[4];
	};

	struct Script_Quest_Class_Request
	{
		BYTE    byLive;
		BYTE    byType;
		WORD    wCompleteQuestIndex;
		WORD    wLevelMin;
		WORD    wLevelMax;
		WORD	wRequestStrength;
		DWORD   dwZen;
		short   shErrorText;
	};

	struct Script_Quest
	{
		short   shQuestConditionNum;
		short   shQuestRequestNum;
		WORD    wNpcType;
		char    strQuestName[32];

		Script_Quest_Class_Act     QuestAct[MAX_QUEST_CONDITION];
		Script_Quest_Class_Request QuestRequest[MAX_QUEST_REQUEST];
	};
};

//infomation start
typedef struct
{
	BYTE Flag;
	BYTE Map;
	BYTE x1;
	BYTE y1;
	BYTE x2;
	BYTE y2;
	WORD Target;
	BYTE Angle;
	WORD Level;
	WORD m_wMaxLevel;
} GATE_ATTRIBUTE;

typedef struct
{
	char m_lpszText[MAX_LENGTH_DIALOG];
	int m_iNumAnswer;
	int m_iLinkForAnswer[MAX_ANSWER_FOR_DIALOG];
	int m_iReturnForAnswer[MAX_ANSWER_FOR_DIALOG];
	char m_lpszAnswer[MAX_ANSWER_FOR_DIALOG][MAX_LENGTH_ANSWER];
} DIALOG_SCRIPT;//Script_Dialog


typedef struct
{
	char Name[30];
	WORD TwoHand;
	WORD Level;
	WORD m_byItemSlot;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	WORD m_wSkillIndex;
#else //PBG_ADD_NEWCHAR_MONK_SKILL
	WORD m_bySkillIndex;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	BYTE Width;
	BYTE Height;
	BYTE DamageMin;
	BYTE DamageMax;
	BYTE SuccessfulBlocking;
	BYTE Defense;
	BYTE MagicDefense;
	BYTE WeaponSpeed;
	BYTE WalkSpeed;
	BYTE Durability;
	BYTE MagicDur;
	BYTE MagicPower;
	WORD RequireStrength;
	WORD RequireDexterity;
	WORD RequireEnergy;
	WORD  RequireVitality;
	WORD RequireCharisma;
	WORD RequireLevel;
	WORD Value;
	int  iZen;
	BYTE  AttType;
	BYTE RequireClass[MAX_CLASS+1];
	BYTE Resistance[MAX_RESISTANCE+1];
} ITEM_ATTRIBUTE;

typedef struct _MASTER_LEVEL_DATA
{
	BYTE Width;
	int Ability[8][4];
}MASTER_LEVEL_DATA;

typedef struct _MINI_MAP
{
	BYTE Kind;
	int	 Location[2];
	int  Rotation;
	char Name[100];
}MINI_MAP;

#pragma pack(push, 1)
typedef struct tagITEM
{
	short Type;
	int   Level;
	BYTE  Part;
	BYTE  Class;
	bool  TwoHand;
	WORD  DamageMin;
	WORD  DamageMax;
	BYTE  SuccessfulBlocking;
	WORD  Defense;
	WORD  MagicDefense;
	BYTE  MagicPower;
	BYTE  WeaponSpeed;
	WORD  WalkSpeed;
	BYTE  Durability;
	BYTE  Option1;
	BYTE  ExtOption;
	WORD  RequireStrength;
	WORD  RequireDexterity;
	WORD  RequireEnergy;
	WORD  RequireVitality;
	WORD  RequireCharisma;
	WORD  RequireLevel;
	BYTE  SpecialNum;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	WORD  Special     [MAX_ITEM_SPECIAL];
#else //PBG_ADD_NEWCHAR_MONK_SKILL
	BYTE  Special     [MAX_ITEM_SPECIAL];
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	BYTE  SpecialValue[MAX_ITEM_SPECIAL];
	DWORD Key;	//client olny
	
	BYTE bySelectedSlotIndex;

	union 
	{
		BYTE  x;    //client olny
		BYTE  lineal_pos;
	};
	union
	{
		BYTE  y;    //client olny
		BYTE  ex_src_type;
	};
	WORD  Jewel_Of_Harmony_Option;
	WORD  Jewel_Of_Harmony_OptionLevel;
	bool option_380;
	BYTE bySocketOption[MAX_SOCKETS];
	BYTE SocketCount;
	BYTE SocketSeedID[MAX_SOCKETS];
	BYTE SocketSphereLv[MAX_SOCKETS];
	BYTE SocketSeedSetOption;

	int   Number;
	BYTE  Color;
	
	BYTE byColorState;
	
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
	bool	bPeriodItem;
	bool	bExpiredPeriod;
	//DWORD	dwLeftTime;
	long	lExpireTime;
	//DWORD	dwPastSystemTime;
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM

private:
	friend class SEASON3B::CNewUIItemMng;
	int   RefCount;
} ITEM;
#pragma pack(pop)

typedef struct
{
	WORD Life;
	WORD MoveSpeed;
	WORD AttackSpeed;
	WORD AttackDamageMin;
	WORD AttackDamageMax;
	WORD Defense;
	WORD MagicDefense;
	WORD AttackRating;
	WORD SuccessfulBlocking;
} MONSTER_ATTRIBUTE;

typedef struct
{
	WORD				Type;
	char				Name[32];
	WORD				Level;
	MONSTER_ATTRIBUTE	Attribute;
} MONSTER_SCRIPT;

typedef struct
{
	short             Type;
	BYTE              Level;
	int               Experince;
	MONSTER_ATTRIBUTE Attribute;
} MONSTER;

typedef struct
{
	char Name[32];
	BYTE Level;
	WORD Damage;
	WORD Mana;
	WORD AbilityGuage;
	BYTE Distance;
	int  Delay;

	int Energy;
	
	WORD Charisma;
	BYTE MasteryType;
	BYTE SkillUseType;
	BYTE SkillBrand;
	BYTE KillCount;
	BYTE RequireDutyClass[MAX_DUTY_CLASS];
	BYTE RequireClass[MAX_CLASS];
	WORD Magic_Icon;
	
	BYTE TypeSkill;

	int Strength;
	int Dexterity;

} SKILL_ATTRIBUTE;


typedef struct
{
	WORD Strength;
	WORD Dexterity;
	WORD Vitality;
	WORD Energy;
	WORD Life;
	WORD Mana;
	WORD Shield;
	BYTE LevelLife;
	BYTE LevelMana;
	BYTE VitalityToLife;
	BYTE EnergyToMana;
	BYTE ClassSkill[MAX_SKILLS];
} CLASS_ATTRIBUTE;

typedef struct
{
	char Name[MAX_ID_SIZE+1];
	BYTE Class;
	BYTE Skin;
	WORD Level;
	DWORD Experience;
	DWORD NextExperince;

	WORD Strength;
	WORD Dexterity;
	WORD Vitality;
	WORD Energy;
	WORD Charisma;
	WORD Life;
	WORD Mana;
	WORD LifeMax;
	WORD ManaMax;
	WORD Shield;
	WORD ShieldMax;
	WORD AttackRatingPK;
	WORD SuccessfulBlockingPK;
	WORD AddStrength;
	WORD AddDexterity;
	WORD AddVitality;
	WORD AddEnergy;
	WORD AddLifeMax;
	WORD AddManaMax;
	WORD AddCharisma;
	WORD SkillMana;
	WORD SkillManaMax;
	BYTE Ability;
	WORD AbilityTime[3];

	short AddPoint;
	short MaxAddPoint;
	WORD wMinusPoint;
	WORD wMaxMinusPoint;
	WORD AttackSpeed;
	WORD AttackRating;
	WORD AttackDamageMinRight;
	WORD AttackDamageMaxRight;
	WORD AttackDamageMinLeft;
	WORD AttackDamageMaxLeft;
	WORD MagicSpeed;
	WORD MagicDamageMin;
	WORD MagicDamageMax;
	WORD CurseDamageMin;
	WORD CurseDamageMax;
	WORD CriticalDamage;
	WORD SuccessfulBlocking;
	WORD Defense;
	WORD MagicDefense;
	WORD WalkSpeed;
	WORD LevelUpPoint;
	BYTE SkillNumber;
	BYTE SkillMasterNumber;
	WORD Skill[MAX_SKILLS];
	BYTE SkillLevel[MAX_SKILLS];
	int  SkillDelay[MAX_SKILLS];
} CHARACTER_ATTRIBUTE;

typedef struct _MASTER_LEVEL_VALUE
{
	short		 nMLevel;
	__int64		 lMasterLevel_Experince;
	__int64		 lNext_MasterLevel_Experince;

	short		nAddMPoint;
	short      	nMLevelUpMPoint;
	short		nTotalMPoint;
	short		nMaxPoint;

	WORD		wMaxLife;
	WORD		wMaxMana;
	WORD		wMaxShield;
	WORD		wMaxBP;

} MASTER_LEVEL_VALUE;

//infomation end

typedef struct _OBJECT_BLOCK
{
	unsigned char Index;
	OBJECT        *Head;
	OBJECT        *Tail;
	bool          Visible;

	_OBJECT_BLOCK()
	{
		Index = 0;
		Head = NULL;
		Tail = NULL;
		Visible = false;
	}

} OBJECT_BLOCK;

typedef struct
{
	bool   Live;
	int    Type;
	int    TexType;
	int    SubType;
	float  Scale;
	vec3_t Position;
	vec3_t Angle;
	vec3_t Light;
	float  Alpha;
	int    LifeTime;
	OBJECT *Target;
	float  Rotation;
	int    Frame;

	bool   bEnableMove;
	float  Gravity;
	vec3_t Velocity;
	vec3_t TurningForce;
	vec3_t StartPosition; 
	int iNumBone;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	bool bRepeatedly;
	float fRepeatedlyHeight;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
} PARTICLE;

typedef struct
{
	bool	 Live;
	int	     Type;
	OBJECT   *Owner;
} OPERATE;

typedef struct
{
	short  Key;
	ITEM   Item;
	OBJECT Object;
} ITEM_t;

//object end

typedef struct Script_Skill
{
	int Skill_Num[MAX_MONSTERSKILL_NUM];
	int Slot;
} Script_Skill;


typedef struct
{
	bool		Live;
	int			Type;
	int         TexType;
	int			SubType;
	BYTE        RenderType;
	BYTE        RenderFace;
	float		Scale;
	vec3_t		Position;
	vec3_t      StartPosition;
	vec3_t		Angle;
	vec3_t		HeadAngle;
	vec3_t		Light;
	OBJECT      *Target;
	vec3_t      TargetPosition;
	BYTE        byOnlyOneRender;
	int			NumTails;
	int			MaxTails;
	vec3_t		Tails[MAX_TAILS][4];
	int  		LifeTime;
	bool        Collision;
	float		Velocity;
	vec3_t		Direction;
	short       PKKey;
	WORD		Skill;
	BYTE		Weapon;
	int			MultiUse;
	bool        bTileMapping;
	BYTE        m_byReverseUV;
	bool        m_bCreateTails;
	int			TargetIndex[5];
	BYTE		m_bySkillSerialNum;
	int			m_iChaIndex;
	short int	m_sTargetIndex;
} JOINT;
//character end

//inventory start
typedef struct
{
	char Name[MAX_ID_SIZE+1];
	BYTE Number;
	BYTE Server;
	BYTE GuildStatus;
} GUILD_LIST_t;

typedef struct
{
	int  Key;
	char UnionName[8+1];
	char GuildName[8+1];
	BYTE Mark[64];
} MARK_t;

typedef struct
{
	char    Name[MAX_ID_SIZE+1];
	BYTE    Number;
	BYTE    Map;
	BYTE    x;
	BYTE    y;
	int     currHP;
	int     maxHP;
	BYTE    stepHP;
	int     index;
} PARTY_t;
//inventory end

//interface start
typedef struct
{
	char      ID[MAX_ID_SIZE+1];
	char      Text[256];
	int       Type;
	int       LifeTime;
	int       Width;
} WHISPER;

typedef struct
{
	bool m_bMagic;
	int m_iSkill;
	int m_iTarget;
} MovementSkill;
//interface end

//matchevent start
typedef struct
{
	char	m_lpID[MAX_ID_SIZE];
	int		m_iScore;
	DWORD	m_dwExp;
	int		m_iZen;
} DevilSquareRank;
//matchevent end

//gmhellas start
typedef struct 
{
	char    m_strName[64];
	vec3_t  m_vPos;
}ObjectDescript;
//gmhellas end

//csquest start
typedef struct 
{
	BYTE    chLive;
	BYTE    byQuestType;
	WORD    wItemType;
	BYTE    byItemSubType;
	BYTE	byItemLevel;
	BYTE    byItemNum;
	BYTE    byRequestType;
	BYTE    byRequestClass[MAX_CLASS];
	short   shQuestStartText[4];
} QUEST_CLASS_ACT;

typedef struct 
{
	BYTE    byLive;
	BYTE    byType;
	WORD    wCompleteQuestIndex;
	WORD    wLevelMin;
	WORD    wLevelMax;
	WORD	wRequestStrength;
	DWORD   dwZen;
	short   shErrorText;
} QUEST_CLASS_REQUEST;

typedef struct 
{
	short   shQuestConditionNum;
	short   shQuestRequestNum;
	WORD	wNpcType;

	unicode::t_char strQuestName[32];

	QUEST_CLASS_ACT     QuestAct[MAX_QUEST_CONDITION];
	QUEST_CLASS_REQUEST QuestRequest[MAX_QUEST_REQUEST];
} QUEST_ATTRIBUTE;

typedef struct 
{
	BYTE    byFlag;
	BYTE    byCount;
} QUEST_FLAG_BUFFER;

typedef struct 
{
	int iCrastGold;
	int iCrastSilver;
	int iCrastBronze;
} QUEST_CRAST;
//csquest end

//csmapinterface start 
typedef struct
{
	BYTE    bIndex;
	BYTE    x;
	BYTE    y;
}VisibleUnitLocation;

typedef struct 
{
	BYTE    byTeam;
	BYTE    byX;
	BYTE    byY;
	BYTE    byCmd;
	BYTE    byLifeTime;
}GuildCommander;
//csmapinterface end

//csitemsetoption start
typedef	struct 
{
	BYTE	byOption[2];
	BYTE	byMixItemLevel[2];
}ITEM_SET_TYPE;


typedef struct 
{
	char	strSetName[64];
	BYTE	byStandardOption[6][2];
	BYTE	byStandardOptionValue[6][2];
	BYTE	byExtOption[2];
	BYTE	byExtOptionValue[2];
	BYTE	byOptionCount;
	BYTE	byFullOption[5];
	BYTE	byFullOptionValue[5];
	BYTE	byRequireClass[MAX_CLASS];
}ITEM_SET_OPTION;
//csitemsetoption end

//cseventmatch start
typedef struct
{
	char	m_lpID[MAX_ID_SIZE];
	int		m_iScore;
	DWORD	m_dwExp;
	int		m_iZen;
}MatchResult;
//cseventmatch end

//cdirection start
struct DirectionMonster
{
	int		m_Index;
	int		m_iActionCheck;
	bool	m_bAngleCheck;
};
//cdirection end

#endif ___STRUCT_H__
