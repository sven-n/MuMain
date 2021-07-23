#ifndef ___STRUCT_H__
#define ___STRUCT_H__

#pragma once

#include "w_Buff.h"

#include "w_ObjectInfo.h"
class OBJECT;
#include "w_CharacterInfo.h"
class CHARACTER;

/////////////////////// Predecaration ////////////////////////////////////
namespace SEASON3B
{
	class CNewUIItemMng;
}

#ifdef MR0
namespace ModelManager
{
	void	SetTargetObject(OBJECT*);	//애니메이션 Binding할 Object 지정
	void	SetTargetObject_(int iFlag = 0);
}
#endif //MR0
//////////////////////////////////////////////////////////////////////////

//배열로 정의된 부분은 다시 수정을 해야한다.
//먼저 디파인들을 정리 해서 여기 위에다 링크 걸어두자...

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
	
#ifndef CSK_FIX_MONSTERSKILL		// 정리할 때 지워야 하는 소스	
	struct Script_Skill
	{
		int Skill_Num[5];
		int Slot;
	};
#endif //! CSK_FIX_MONSTERSKILL		// 정리할 때 지워야 하는 소스

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
		BYTE	byClass;				// 줄 위치.(1~3째줄, 0이면 크레딧 끝임.)
		char	szName[32];	            // 항목(이름, 부서 등) 문자열.
		//char	szName[CRW_NAME_MAX];	// 항목(이름, 부서 등) 문자열.
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
		BYTE    chLive;                 //  해당클래스. 0:해당사항 없음. 1:해당됨.
		BYTE    byQuestType;            //  퀘스트 종류.
		WORD    wItemType;				//  아이템 타입 번호 또는 몹 인덱스.
		BYTE    byItemSubType;          //  아이템.
		BYTE	byItemLevel;			//	아이템 레벨.
		BYTE    byItemNum;              //  아이템 수.
		BYTE    byRequestType;          //  요구 조건 인덱스.
		BYTE    byRequestClass[MAX_CLASS];		//  해당 클래스.
		short   shQuestStartText[4];    //  퀘스트 내용 시작. ( 등록전, 등록후, 조건완비, 수행완료 ) 
	};

	struct Script_Quest_Class_Request
	{
		BYTE    byLive;                 //  
		BYTE    byType;                 //  255는 공통 요구 조건임.
		WORD    wCompleteQuestIndex;    //  수행 왼료된 퀘스트 인덱스.
		WORD    wLevelMin;              //  최소 레벨.
		WORD    wLevelMax;              //  최대 레벨.
		WORD	wRequestStrength;		//	요구 힘.
		DWORD   dwZen;                  //  요구 젠.
		short   shErrorText;            //  
	};

	struct Script_Quest
	{
		short   shQuestConditionNum;    //  퀘스트 수행 조건 수.
		short   shQuestRequestNum;      //  퀘스트 요구 조건 수.
		WORD    wNpcType;              //  NPC타입.
		char    strQuestName[32];       //  퀘스트 이름.

		Script_Quest_Class_Act     QuestAct[MAX_QUEST_CONDITION];        //  수행 조건.
		Script_Quest_Class_Request QuestRequest[MAX_QUEST_REQUEST];    //  요구 조건.
	};
};

#ifndef KJH_ADD_SERVER_LIST_SYSTEM			// #ifndef
//scene start
typedef struct
{
	char  IP[20];						// 안씀.
	WORD  Port;							// 안씀.
	WORD  Index;
	BYTE  Percent;
#ifdef _PVP_DYNAMIC_SERVER_TYPE
	BYTE  IsPvPServer;
#endif	// _PVP_DYNAMIC_SERVER_TYPE
} SERVER_t;

typedef struct
{
	char     Name[20];
	BYTE     Number;
	bool     extServer;					// 안씀.
	SERVER_t Server[MAX_SERVER_LOW];
} SERVER_LIST_t;
//scene end
#endif // KJH_ADD_SERVER_LIST_SYSTEM

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
		BYTE  lineal_pos;	// 선형 위치(==y*column+x)
	};
	union
	{
		BYTE  y;    //client olny
		BYTE  ex_src_type;	// 아이템 메니져를 통하지 않는 static한 아이템의 경우 이 변수로 구분한다.
		// 0: 없음, 1: 인벤토리 장비창
	};
	WORD  Jewel_Of_Harmony_Option;//옵션 종류
	WORD  Jewel_Of_Harmony_OptionLevel;//옵션 레벨( 값이 아님 )
	bool option_380;
#ifdef SOCKET_SYSTEM
	BYTE bySocketOption[MAX_SOCKETS];	// 소켓 정보 (서버에서 받은것 그대로 저장-서버에 재전송용)
	BYTE SocketCount;					// 소켓 개수 (0~MAX_SOCKETS), 0: 소켓 아이템 아님
	BYTE SocketSeedID[MAX_SOCKETS];		// 시드 고유번호 (0~49), SOCKET_EMPTY: 빈 소켓
	BYTE SocketSphereLv[MAX_SOCKETS];	// 스피어 레벨 (1~5)
	BYTE SocketSeedSetOption;			// 시드 세트 옵션 (소켓 장착시 결정되는)
#endif	// SOCKET_SYSTEM
	
	// 정리할 때 지워야 하는 소스
	int   Number;	//. UI_RENEW_INVENTORY_BASE: 사용하지 않을 예정
	BYTE  Color;	//. UI_RENEW_INVENTORY_BASE: 사용하지 않을 예정
	
	BYTE byColorState;		// 아이템 밑에 깔리는 칼라 세팅
	
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
	bool	bPeriodItem;	// 기간제 아이템인가?
	bool	bExpiredPeriod;	// 기간이 만료되었나?
	//DWORD	dwLeftTime;		// 기간제 아이템일때 남은시간 (단위 : 초)	- Render용
	long	lExpireTime;	// 기간 만료 시간.
	//DWORD	dwPastSystemTime;	// 지난System시간		- 계산용
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM

private:
	friend class SEASON3B::CNewUIItemMng;
	int   RefCount;		//. 오로지 CNewUIItemMng 에서만 접근 가능
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

#ifdef PBG_FIX_SKILL_DEMENDCONDITION
	int Energy;				// 필요 에너지 ( 계산식이 필요함 = 20+(ReqEng*Level)*0.04 )
#else //PBG_FIX_SKILL_DEMENDCONDITION
	BYTE Energy;            // 필요 에너지 ( 계산식이 필요함 = 20+(ReqEng*Level)*0.04 )
#endif //PBG_FIX_SKILL_DEMENDCONDITION
	
	WORD Charisma;
	BYTE MasteryType;
	BYTE SkillUseType;      //  스킬 유저 종류.
	BYTE SkillBrand;        //  스킬을 사용하기 위해 시전되야 하는 스킬 번호.
	BYTE KillCount;
	BYTE RequireDutyClass[MAX_DUTY_CLASS];
	BYTE RequireClass[MAX_CLASS];
	WORD Magic_Icon;		//마스터 스킬에서 사용
	
#ifdef LDS_FIX_APPLYSKILLTYPE_AND_CURSEDTEMPLEWRONGPARTYMEMBER
	BYTE TypeSkill;
#endif // LDS_FIX_APPLYSKILLTYPE_AND_CURSEDTEMPLEWRONGPARTYMEMBER

#ifdef PBG_FIX_SKILL_DEMENDCONDITION
	int Strength;		// 필요 힘
	int Dexterity;	    // 필요 민첩
#endif //PBG_FIX_SKILL_DEMENDCONDITION
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

	WORD Strength;		//힘
	WORD Dexterity;		//민첩
	WORD Vitality;		//체력
	WORD Energy;		//에너지
	WORD Charisma;		//통솔
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

	short AddPoint;			// 현재 생성 포인트
	short MaxAddPoint;		// 최대 생성 포인트
	WORD wMinusPoint;		// 현재 감소 포인트
	WORD wMaxMinusPoint;	// 최대 감소 포인트
	WORD AttackSpeed;
	WORD AttackRating;
	WORD AttackDamageMinRight;
	WORD AttackDamageMaxRight;
	WORD AttackDamageMinLeft;
	WORD AttackDamageMaxLeft;
	WORD MagicSpeed;
	WORD MagicDamageMin;
	WORD MagicDamageMax;
	WORD CurseDamageMin;	// 저주력 데미지 최소값
	WORD CurseDamageMax;	// 저주력 데미지 최대값
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
	int  SkillDelay[MAX_SKILLS];        //  스킬 딜레이.
} CHARACTER_ATTRIBUTE;

typedef struct _MASTER_LEVEL_VALUE
{
	short		 nMLevel;			// 현재 마스터레벨
	__int64		 lMasterLevel_Experince;
	__int64		 lNext_MasterLevel_Experince;

	short		nAddMPoint;			// 추가된 마스터레벨 포인트
	short      	nMLevelUpMPoint;	// 사용 가능한 마스터레벨 포인트
	short		nTotalMPoint;		// 내가 가진 총 마스터레벨 포인트
	short		nMaxPoint;			// 최대 마스터레벨 포인트

	WORD		wMaxLife;
	WORD		wMaxMana;
	WORD		wMaxShield;
	WORD		wMaxBP;

} MASTER_LEVEL_VALUE;

//infomation end

#ifdef MR0
typedef struct _AUTOOBJ : public OBJECT
{
	_AUTOOBJ::_AUTOOBJ() : OBJECT()
	{
		m_iID = GenID();
	}
}AUTOOBJ;
#endif //MR0

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
	int iNumBone;				// 오브젝트의 해당하는 본번호의 위치를 보정
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
