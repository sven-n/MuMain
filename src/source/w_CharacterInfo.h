#pragma once

#include "SpinLock.h"

typedef struct _PATH_t
{
    unsigned char CurrentPath;
    unsigned char CurrentPathFloat;
    unsigned char PathNum;
    unsigned char PathX[MAX_PATH_FIND];
    unsigned char PathY[MAX_PATH_FIND];

    bool          Success;
    bool          Error;
    unsigned char x, y;
    unsigned char Direction;
    unsigned char Run;
    int           Count;
    SpinLock      Lock;

    _PATH_t()
    {
        CurrentPath = 0;
        CurrentPathFloat = 0;
        PathNum = 0;

        for (int i = 0; i < MAX_PATH_FIND; ++i)
        {
            PathX[i] = 0;
            PathY[i] = 0;
        }

        Success = 0;
        Error = 0;
        x = 0, y = 0;
        Direction = 0;
        Run = 0;
        Count = 0;
    }
} PATH_t;

typedef struct _PART_t
{
    short Type;
    BYTE  Level;
    BYTE  ExcellentFlags; // Excellent
    BYTE  AncientDiscriminator; // Ancient
    BYTE  LinkBone;
    BYTE  CurrentAction;
    unsigned short  PriorAction;
    float AnimationFrame;
    float PriorAnimationFrame;
    float PlaySpeed;
    BYTE m_byNumCloth;
    void* m_pCloth[2];

    _PART_t()
    {
        Type = 0;
        Level = 0;
        ExcellentFlags = 0;
        AncientDiscriminator = 0;
        LinkBone = 0;
        CurrentAction = 0;
        PriorAction = 0;
        AnimationFrame = 0;
        PriorAnimationFrame = 0;
        PlaySpeed = 0;
        m_byNumCloth = 0;
        m_pCloth[0] = NULL;
        m_pCloth[1] = NULL;
    }
} PART_t;

typedef struct
{
    DWORD		m_dwPetType;
    DWORD       m_dwExp1;
    DWORD       m_dwExp2;
    WORD        m_wLevel;
    WORD        m_wLife;
    WORD        m_wDamageMin;
    WORD        m_wDamageMax;
    WORD        m_wAttackSpeed;
    WORD        m_wAttackSuccess;
} PET_INFO;

#define DEFAULT_MAX_POSTMOVEPCOCESS		15

struct ST_POSTMOVE_PROCESS
{
    bool				bProcessingPostMoveEvent;
    unsigned int		uiProcessingCount_PostMoveEvent;

    ST_POSTMOVE_PROCESS()
    {
        bProcessingPostMoveEvent = false;
        uiProcessingCount_PostMoveEvent = 0;
    }

    ~ST_POSTMOVE_PROCESS()
    {
    }
};

class CHARACTER
{
public:
    CHARACTER();
    virtual ~CHARACTER();

public:
    void			Initialize();
    void			Destroy();
    void			InitPetInfo(int iPetType);
    PET_INFO* GetEquipedPetInfo(int iPetType);
    void			PostMoveProcess_Active(unsigned int uiLimitCount);
    unsigned int	PostMoveProcess_GetCurProcessCount();
    bool			PostMoveProcess_IsProcessing();
    bool			PostMoveProcess_Process();
    bool			Blood;
    bool			Ride;
    bool			SkillSuccess;
    bool			NotRotateOnMagicHit;
    bool			Foot[2];
    bool			SafeZone;
    bool			Change; // True for transformed players
    bool			HideShadow;
    bool			m_bIsSelected;
    bool			Decoy;
    CLASS_TYPE			Class;
    CLASS_SKIN_INDEX            SkinIndex;
    BYTE			Skin; // What is this good for?
    BYTE			CtlCode;
    BYTE			ExtendState;
    BYTE			EtcPart;
    BYTE		GuildStatus;
    BYTE		GuildType;
    BYTE		GuildRelationShip;
    BYTE        GuildSkill;
    BYTE        GuildMasterKillCount;
    BYTE        BackupCurrentSkill;
    BYTE        GuildTeam;
#ifdef ASG_ADD_GENS_SYSTEM
    BYTE		m_byGensInfluence;
#endif	// ASG_ADD_GENS_SYSTEM
    BYTE        PK;
#ifdef LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER
    char		PKPartyLevel;
#endif //LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER
    BYTE        AttackFlag;
    
    BYTE        TargetAngle;
    float        Dead; // Number of reference frames after death
    WORD		Skill;
    BYTE        SwordCount;
    BYTE		byExtensionSkill;
    WORD		m_byDieType;
    BYTE        TargetX;
    BYTE        TargetY;
    BYTE        SkillX;
    BYTE        SkillY;
    float        Appear;
    BYTE	    CurrentSkill; // Skill Index
    BYTE        CastRenderTime; // unused?
    BYTE        m_byFriend;
    WORD        MonsterSkill;

    wchar_t     ID[MAX_MONSTER_NAME + 1]{};
    char 		Movement;
    char		MovementType;
    char		CollisionTime; // unused?

    short       GuildMarkIndex;
    SHORT       Key;
    short   	TargetCharacter;

    WORD        Level;
    EMonsterType        MonsterIndex;
    int        Damage;
    int        Hit;
    WORD        MoveSpeed;
    WORD        AttackSpeed;
    WORD        MagicSpeed;

    int			Action;
    int			LongRangeAttack;
    int			SelectItem;
    int			Item;
    int         FreezeType;
    int         PriorPositionX;
    int         PriorPositionY;
    int         PositionX;
    int         PositionY;
    int			m_iFenrirSkillTarget;
    int         LastAttackEffectTime;

    float       m_iDeleteTime;
    float       LastCritDamageEffect;
    float       ExtendStateTime;
    float		JumpTime;
    float		StormTime;
    float		AttackTime;

    float		ProtectGuildMarkWorldTime;
    float		AttackRange;
    float       Freeze;
    float       Duplication;
    float		Rot;
    float       Run;
    float       HealthStatus;
    float       ShieldStatus;

    vec3_t		TargetPosition;
    vec3_t      Light;

    PART_t    	BodyPart[MAX_BODYPART];
    PART_t		Weapon[2];
    PART_t		Wing;
    PART_t		Helper;
    PART_t		Flag;
    PATH_t      Path;

    void* m_pParts;

    void* m_pPet;
    PET_INFO	m_PetInfo[PET_TYPE_END];

    wchar_t     OwnerID[32];

private:
    ST_POSTMOVE_PROCESS*
        m_pPostMoveProcess;

public:
    void* m_pTempParts;
    int			m_iTempKey;
    WORD		m_CursedTempleCurSkill;
    bool		m_CursedTempleCurSkillPacket;
    OBJECT	    Object;
#ifdef PBG_ADD_GENSRANKING
    BYTE		GensRanking;
#endif //PBG_ADD_GENSRANKING
#ifdef PBG_MOD_STRIFE_GENSMARKRENDER
    BYTE		GensContributionPoints;
#endif //PBG_MOD_STRIFE_GENSMARKRENDER
    bool CheckAttackTime(int timeNumber) const
    {
        return static_cast<int>(AttackTime) == timeNumber && LastAttackEffectTime != timeNumber;
    }
    void SetLastAttackEffectTime()
    {
        LastAttackEffectTime = static_cast<int>(AttackTime);
    }
};
