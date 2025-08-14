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
        wchar_t Name[30];
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
        typedef std::vector<wchar_t*>     SildeVECTOR;
        SildeVECTOR					   Sildelist;
    };

    struct Script_Dialog
    {
        wchar_t m_lpszText[MAX_LENGTH_DIALOG];
        int m_iNumAnswer;
        int m_iLinkForAnswer[MAX_ANSWER_FOR_DIALOG];
        int m_iReturnForAnswer[MAX_ANSWER_FOR_DIALOG];
        wchar_t m_lpszAnswer[MAX_ANSWER_FOR_DIALOG][MAX_LENGTH_ANSWER];
    };

    struct Script_Credit
    {
        BYTE	byClass;
        wchar_t	szName[32];
    };

    struct Script_Movereq
    {
        int index;
        wchar_t szMainMapName[32];
        wchar_t szSubMapName[32];
        int	iReqLevel;
        int iReqZen;
        int iGateNum;
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
    char Name[MAX_ITEM_NAME];
    bool TwoHand;
    WORD Level;
    BYTE m_byItemSlot;
    WORD m_wSkillIndex;
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
    WORD RequireVitality;
    WORD RequireCharisma;
    WORD RequireLevel;
    BYTE Value;
    int  iZen;
    BYTE  AttType;
    BYTE RequireClass[MAX_CLASS];
    BYTE Resistance[MAX_RESISTANCE + 1];
} ITEM_ATTRIBUTE_FILE;

typedef struct
{
    wchar_t Name[MAX_ITEM_NAME];
    bool TwoHand;
    WORD Level;
    BYTE m_byItemSlot;
    WORD m_wSkillIndex;
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
    WORD RequireVitality;
    WORD RequireCharisma;
    WORD RequireLevel;
    BYTE Value;
    int  iZen;
    BYTE  AttType;
    BYTE RequireClass[MAX_CLASS];
    BYTE Resistance[MAX_RESISTANCE + 1];
} ITEM_ATTRIBUTE;

typedef struct _MASTER_LEVEL_DATA
{
    BYTE Width;
    int Ability[8][4];
}MASTER_LEVEL_DATA;

typedef struct
{
    BYTE Kind;
    int	 Location[2];
    int  Rotation;
    char Name[MAX_MINIMAP_NAME];
}MINI_MAP_FILE;

typedef struct
{
    BYTE Kind;
    int	 Location[2];
    int  Rotation;
    wchar_t Name[MAX_MINIMAP_NAME];
} MINI_MAP;

#pragma pack(push, 1)
typedef struct tagITEM
{
    short Type;
    int   Level;
    BYTE  Durability;
    BYTE OptionLevel;
    BYTE OptionType;
    BYTE ExcellentFlags;
    BYTE AncientDiscriminator; // 0 = no ancient;
    BYTE AncientBonusOption;
    WORD  Jewel_Of_Harmony_Option;
    WORD  Jewel_Of_Harmony_OptionLevel;
    bool HasSkill;
    bool HasLuck;
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

    WORD  RequireStrength;
    WORD  RequireDexterity;
    WORD  RequireEnergy;
    WORD  RequireVitality;
    WORD  RequireCharisma;
    WORD  RequireLevel;
    BYTE  SpecialNum;
    WORD  Special[MAX_ITEM_SPECIAL];
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
    bool option_380;

    BYTE SocketCount;
    BYTE bySocketOption[MAX_SOCKETS];
    BYTE SocketSeedID[MAX_SOCKETS];
    BYTE SocketSphereLv[MAX_SOCKETS];
    BYTE SocketSeedSetOption;

    int   Number;
    BYTE  Color;

    BYTE byColorState;

    bool	bPeriodItem;
    bool	bExpiredPeriod;
    long	lExpireTime;

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
    wchar_t				Name[MAX_MONSTER_NAME];
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
    /*+00*/	char Name[MAX_SKILL_NAME];
    /*+32*/	WORD Level;
    /*+34*/	WORD Damage;
    /*+36*/	WORD Mana;
    /*+38*/	WORD AbilityGuage;
    /*+40*/	DWORD Distance;
    /*+44*/	int Delay;
    /*+48*/	int Energy;
    /*+52*/	WORD Charisma;
    /*+54*/	BYTE MasteryType;
    /*+55*/	BYTE SkillUseType;
    /*+56*/	DWORD SkillBrand;
    /*+60*/	BYTE KillCount;
    /*+61*/	BYTE RequireDutyClass[MAX_DUTY_CLASS];
    /*+64*/	BYTE RequireClass[MAX_CLASS];
    /*+71*/	BYTE SkillRank;
    /*+72*/	WORD Magic_Icon;
    /*+74*/	BYTE TypeSkill;
    /*+76*/	int Strength;
    /*+80*/	int Dexterity;
    /*+84*/	BYTE ItemSkill;
    /*+85*/	BYTE IsDamage;
    /*+86*/	WORD Effect;
} SKILL_ATTRIBUTE_FILE;

typedef struct
{
    wchar_t Name[MAX_SKILL_NAME];
    WORD Level;
    WORD Damage;
    WORD Mana;
    WORD AbilityGuage;
    DWORD Distance;
    int Delay;
    int Energy;
    WORD Charisma;
    BYTE MasteryType;
    BYTE SkillUseType;
    DWORD SkillBrand;
    BYTE KillCount;
    BYTE RequireDutyClass[MAX_DUTY_CLASS];
    BYTE RequireClass[MAX_CLASS];
    BYTE SkillRank;
    WORD Magic_Icon;
    BYTE TypeSkill;
    int Strength;
    int Dexterity;
    BYTE ItemSkill;
    BYTE IsDamage;
    WORD Effect;
} SKILL_ATTRIBUTE;
/*
typedef struct
{
    wchar_t Name[32];
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
*/

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

class CSkillTreeInfo
{
public:
    CSkillTreeInfo()
    {
        this->skillLevel = 0;
        this->skillValue = 0.0f;
        this->skillNextValue = 0.0f;
    }

    CSkillTreeInfo(BYTE point, float value, float nextValue)
    {
        this->skillLevel = point;
        this->skillValue = value;
        this->skillNextValue = nextValue;
    }

    ~CSkillTreeInfo() = default;

    BYTE GetSkillLevel() const { return this->skillLevel; }
    float GetSkillValue() const { return this->skillValue; }
    float GetSkillNextValue() const { return this->skillNextValue; }
private:
    BYTE skillLevel;
    float skillValue;
    float skillNextValue;
};

typedef struct
{
    wchar_t Name[MAX_ID_SIZE + 1];
    CLASS_TYPE Class;
    BYTE Skin;
    BYTE InventoryExtensions;
    BYTE IsVaultExtended;
    WORD Level;

    WORD Strength;
    WORD Dexterity;
    WORD Vitality;
    WORD Energy;
    WORD Charisma;
    DWORD Life;
    DWORD Mana;
    DWORD LifeMax;
    DWORD ManaMax;
    DWORD Shield;
    DWORD ShieldMax;
    DWORD SkillMana;
    DWORD SkillManaMax;
    WORD AttackRatingPK;
    WORD SuccessfulBlockingPK;
    WORD AddStrength;
    WORD AddDexterity;
    WORD AddVitality;
    WORD AddEnergy;
    WORD AddLifeMax;
    WORD AddManaMax;
    WORD AddCharisma;
    BYTE Ability;
    float AbilityTime[3]; // Number of reference frames

    short AddPoint;
    short MaxAddPoint;
    WORD wMinusPoint;
    WORD wMaxMinusPoint;
    WORD AttackSpeed;
    WORD MaxAttackSpeed; // Maximum attack speed which can be reached on the server.
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

    uint64_t Experience;
    uint64_t NextExperience;

    ActionSkillType Skill[MAX_SKILLS];
    int  SkillDelay[MAX_SKILLS];
    BYTE SkillLevel[MAX_SKILLS]; // Do we even need this array when we have the map of CSkillTreeInfo?

    CSkillTreeInfo MasterSkillInfo[AT_SKILL_MASTER_END + 1]; // Index = ActionSkillType
    
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
    DWORD		wMaxLife;
    DWORD		wMaxMana;
    DWORD		wMaxShield;
    DWORD		wMaxBP;
} MASTER_LEVEL_VALUE;

//infomation end

typedef struct _OBJECT_BLOCK
{
    unsigned char Index;
    OBJECT* Head;
    OBJECT* Tail;
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
    float  LifeTime;
    OBJECT* Target;
    float  Rotation;
    int    Frame;

    bool   bEnableMove;
    float  Gravity;
    vec3_t Velocity;
    vec3_t TurningForce;
    vec3_t StartPosition;
    int iNumBone;
    bool bRepeatedly;
    float fRepeatedlyHeight;
} PARTICLE;

typedef struct
{
    bool	 Live;
    int	     Type;
    OBJECT* Owner;
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
    OBJECT* Target;
    vec3_t      TargetPosition;
    BYTE        byOnlyOneRender;
    float  		LifeTime;
    bool        Collision;
    float		Velocity;
    vec3_t		Direction;
    short       PKKey;
    WORD		Skill;
    float		Weapon;
    float		MultiUse;
    bool        bTileMapping;
    BYTE        m_byReverseUV;
    int			TargetIndex[5];
    BYTE		m_bySkillSerialNum;
    int			m_iChaIndex;
    short int	m_sTargetIndex;

    // Fields about the "Tails" of effects:
    bool        m_bCreateTails; // Flag, if tails are created.
    int         NumTails; // The number of currently used tail entries. Usually this gets increased by one in every frame until the maximum is reached.
    int         MaxTails; // The maximum number of tail entries to use.
    vec3_t      Tails[MAX_TAILS][4]; // The tail entries, which are getting moved back by one in every frame.
} JOINT;
//character end

//inventory start
typedef struct
{
    wchar_t Name[MAX_ID_SIZE + 1];
    BYTE Number;
    BYTE Server;
    BYTE GuildStatus;
} GUILD_LIST_t;

typedef struct
{
    int  Key;
    wchar_t UnionName[8 + 1];
    wchar_t GuildName[8 + 1];
    BYTE Mark[64];
} MARK_t;

typedef struct
{
    wchar_t    Name[MAX_ID_SIZE + 1];
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
    wchar_t      ID[MAX_ID_SIZE + 1];
    wchar_t      Text[256];
    int       Type;
    int       LifeTime;
    int       Width;
} WHISPER;

typedef struct
{
    bool m_bMagic;
    int m_iSkill; // When m_bMagic is true, it's the skill index, otherwise it's already the ActionSkillType. TODO: check all usages, refactor. This could be buggy as hell.
    int m_iTarget;
} MovementSkill;
//interface end

//matchevent start
typedef struct
{
    wchar_t	m_lpID[MAX_ID_SIZE];
    int		m_iScore;
    DWORD	m_dwExp;
    int		m_iZen;
} DevilSquareRank;
//matchevent end

//gmhellas start
typedef struct
{
    wchar_t    m_strName[64];
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

    wchar_t strQuestName[32];

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

//cseventmatch start
typedef struct
{
    BYTE	m_lpID[MAX_ID_SIZE];
    DWORD	m_iScore;
    DWORD	m_dwExp;
    DWORD	m_iZen;
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
