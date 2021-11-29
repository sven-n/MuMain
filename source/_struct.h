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

#ifdef CSK_FIX_MONSTERSKILL
#define MAX_MONSTERSKILL_NUM	10
#endif // CSK_FIX_MONSTERSKILL

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
#ifdef CSK_ADD_MAP_ICECITY	
	WORD Level;
#else // CSK_ADD_MAP_ICECITY	
	BYTE Level;
#endif // CSK_ADD_MAP_ICECITY
#ifdef ASG_ADD_GATE_TEXT_MAX_LEVEL
	WORD m_wMaxLevel;	// 최대 제한 레벨.
#endif	// ASG_ADD_GATE_TEXT_MAX_LEVEL
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
	bool TwoHand;
	WORD Level;
	BYTE m_byItemSlot;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	WORD m_wSkillIndex;
#else //PBG_ADD_NEWCHAR_MONK_SKILL
	BYTE m_bySkillIndex;
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
	BYTE Value;
	int  iZen;
	BYTE  AttType;
	BYTE RequireClass[MAX_CLASS];
	BYTE Resistance[MAX_RESISTANCE];
} ITEM_ATTRIBUTE;

typedef struct _MASTER_LEVEL_DATA
{
	BYTE Width;
	int Ability[8][4];
}MASTER_LEVEL_DATA;

#ifdef PJH_ADD_MINIMAP
typedef struct _MINI_MAP
{
	BYTE Kind;
	int	 Location[2];
	int  Rotation;
#ifdef KJH_FIX_MINIMAP_NAME
	char Name[100];
#else KJH_FIX_MINIMAP_NAME
	char Name[30];
#endif // KJH_FIX_MINIMAP_NAME
}MINI_MAP;
#endif //PJH_ADD_MINIMAP

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
	
#ifdef LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE
	BYTE bySelectedSlotIndex;
#endif // LDS_FIX_OUTPUT_WRONG_COUNT_EQUIPPEDSETITEMOPTIONVALUE

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
#ifdef SOCKET_SYSTEM
	BYTE bySocketOption[MAX_SOCKETS];
	BYTE SocketCount;
	BYTE SocketSeedID[MAX_SOCKETS];
	BYTE SocketSphereLv[MAX_SOCKETS];
	BYTE SocketSeedSetOption;
#endif	// SOCKET_SYSTEM

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
#ifdef ADD_SOCKET_ITEM
	int iNumBone;
#endif // ADD_SOCKET_ITEM
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
#ifdef CSK_FIX_MONSTERSKILL
	int Skill_Num[MAX_MONSTERSKILL_NUM];
#else // CSK_FIX_MONSTERSKILL
	int Skill_Num[5];
#endif // CSK_FIX_MONSTERSKILL
	int Slot;
}Script_Skill;


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
	BYTE        m_byReverseUV;          //  UV좌표 반전 ( 0: 정상, 1:V 좌표 반전, 2:U 좌표 반전. )
	bool        m_bCreateTails;         //  기본적으로 꼬리를 생성할지 결정. ( true:생성한다. false:사용자가 직접 생성한다. )
	int			TargetIndex[5];
	BYTE		m_bySkillSerialNum;
	int			m_iChaIndex;
	short int	m_sTargetIndex;			// 체인라이트닝스킬 다음 타겟을 위한 변수
} JOINT;
//character end

//inventory start
typedef struct
{
	char Name[MAX_ID_SIZE+1];
	BYTE Number;
	BYTE Server;
	BYTE GuildStatus;	// 길드직위
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
typedef struct  // 악마의 광장 순위
{
	char	m_lpID[MAX_ID_SIZE];	// ID
	int		m_iScore;				// 누적점수
	DWORD	m_dwExp;				// 보상 경험치
	int		m_iZen;					// 포상금
} DevilSquareRank;
//matchevent end

//gmhellas start
//  선택된 오브젝트의 설명.
typedef struct 
{
	char    m_strName[64];
	vec3_t  m_vPos;
}ObjectDescript;
//gmhellas end

//csquest start
typedef struct 
{
	BYTE    chLive;                 //  해당클래스. 0:해당사항 없음. 1:해당됨.
	BYTE    byQuestType;            //  퀘스트 종류.
	WORD    wItemType;				//  아이템 타입 번호 또는 몹 인덱스.
	BYTE    byItemSubType;          //  아이템.
	BYTE	byItemLevel;			//	아이템 레벨.
	BYTE    byItemNum;              //  아이템 수.
	BYTE    byRequestType;          //  요구 조건 인덱스.
	BYTE    byRequestClass[MAX_CLASS];//  해당 클래스.
	short   shQuestStartText[4];    //  퀘스트 내용 시작. ( 등록전, 등록후, 조건완비, 수행완료 ) 
} QUEST_CLASS_ACT;

typedef struct 
{
	BYTE    byLive;                 //  
	BYTE    byType;                 //  255는 공통 요구 조건임.
	WORD    wCompleteQuestIndex;    //  수행 왼료된 퀘스트 인덱스.
	WORD    wLevelMin;              //  최소 레벨.
	WORD    wLevelMax;              //  최대 레벨.
	WORD	wRequestStrength;		//	요구 힘.
	DWORD   dwZen;                  //  요구 젠.
	short   shErrorText;            //  
} QUEST_CLASS_REQUEST;

typedef struct 
{
	short   shQuestConditionNum;    //  퀘스트 수행 조건 수.
	short   shQuestRequestNum;      //  퀘스트 요구 조건 수.
	WORD	wNpcType;				//  NPC타입.

	unicode::t_char strQuestName[32];       //  퀘스트 이름.

	QUEST_CLASS_ACT     QuestAct[MAX_QUEST_CONDITION];      //  수행 조건.
	QUEST_CLASS_REQUEST QuestRequest[MAX_QUEST_REQUEST];    //  요구 조건.
} QUEST_ATTRIBUTE;


//  퀘스트 플래그 버퍼.
typedef struct 
{
	BYTE    byFlag;                 //  0: 미사용. 1~128:퀘스트 아이템, 129~255:몬스터 아이템.
	BYTE    byCount;                //  갯수.
} QUEST_FLAG_BUFFER;


//  크래스트.
typedef struct 
{
	int iCrastGold;                 //  금.
	int iCrastSilver;               //  은.
	int iCrastBronze;               //  동.
} QUEST_CRAST;
//csquest end

#ifndef KJH_FIX_DARKLOAD_PET_SYSTEM							//## 소스정리 대상임.
//cspetsystem start
typedef struct 
{
	DWORD		m_dwPetType;	//	펫타입
	DWORD       m_dwExp1;       //  경험치1.
	DWORD       m_dwExp2;       //  경험치2.
	WORD        m_wLevel;       //  레벨.
	WORD        m_wLife;        //  생명력.
	WORD        m_wDamageMin;   //  최소 데미지.
	WORD        m_wDamageMax;   //  최대 데미지.
	WORD        m_wAttackSpeed; //  공격 속도.
	WORD        m_wAttackSuccess;// 공격 성공율.
}PET_INFO;
//cspetsystem end
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM						//## 소스정리 대상임.

//csmapinterface start 
//  위치 정보.
typedef struct
{
	BYTE    bIndex;
	BYTE    x;
	BYTE    y;
}VisibleUnitLocation;

//  길드 명령.
typedef struct 
{
	BYTE    byTeam;     //  팀 번호.
	BYTE    byX;        //  위치.
	BYTE    byY;        //  위치.
	BYTE    byCmd;      //  명령. ( 0:공격(A), 1:방어(D), 2:대기(H), 3:자유(F) ).
	BYTE    byLifeTime; //  특정 상태를 보여주는 시간.
}GuildCommander;
//csmapinterface end

//csitemsetoption start
typedef	struct 
{
	//	값이 255는 존재하지 않는 아이템임.
	//	[0]: A세트, [1]:B세트.
	BYTE	byOption[2];		//	옵션 인덱스.
	BYTE	byMixItemLevel[2];	//	조합할 아아템의 레벨.
}ITEM_SET_TYPE;


typedef struct 
{
	char	strSetName[64];			    //	세트 추가 이름.
	BYTE	byStandardOption[6][2];	    //	기본 옵션 ( 옵션 1 ~ 6 ).
	BYTE	byStandardOptionValue[6][2];//	기본 옵션 수치 ( 옵션 1 ~ 6 ).
	BYTE	byExtOption[2];			    //	추가 옵션 ( 옵션 7 ~ 8 ).
	BYTE	byExtOptionValue[2];        //	추가 옵션 수치 ( 옵션 7 ~ 8 ).
	BYTE	byOptionCount;
	BYTE	byFullOption[5];		    //	풀   옵션 ( 풀옵 1 ~ 5 ).
	BYTE	byFullOptionValue[5];		//	풀   옵션 수치 ( 풀옵 1 ~ 5 ).
	BYTE	byRequireClass[MAX_CLASS];	//	해당 클래스.
}ITEM_SET_OPTION;
//csitemsetoption end

//cseventmatch start
typedef struct      //  이벤트 경기 결과 정보.
{
	char	m_lpID[MAX_ID_SIZE];	// ID
	int		m_iScore;				// 누적점수         ( 카오스 캐슬:몬스터수 ).
	DWORD	m_dwExp;				// 보상 경험치
	int		m_iZen;					// 포상금           ( 카오스 캐슬:사람수 ).
}MatchResult;
//cseventmatch end

//cdirection start
struct DirectionMonster
{
	int		m_Index;			// 생성될 몬스터의 번호
	int		m_iActionCheck;		// 크라이울프 연출중 동작 체크
	bool	m_bAngleCheck;		// 생성될 몬스터 방향 트는것 체크
};
//cdirection end

#endif ___STRUCT_H__
