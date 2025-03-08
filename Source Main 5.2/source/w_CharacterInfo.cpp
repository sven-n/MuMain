// w_CharacterInfo.cpp: implementation of the CHARACTER class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_CharacterInfo.h"
#include "SkillManager.h"

#include <NewUIGensRanking.h>

CHARACTER::CHARACTER()
{
    Initialize();
}

CHARACTER::~CHARACTER()
{
    Destroy();
}

void CHARACTER::Initialize()
{
    Object.Initialize();

    Blood = false;
    Ride = false;
    SkillSuccess = false;
    NotRotateOnMagicHit = false;
    Foot[0] = false;
    Foot[1] = false;
    SafeZone = false;
    Change = false;
    HideShadow = false;
    m_bIsSelected = false;
    Decoy = false;

    Class = CLASS_WIZARD;
    Skin = 0;
    CtlCode = 0;
    ExtendState = 0;
    EtcPart = 0;
    GuildStatus = 0;
    GuildType = 0;
    GuildRelationShip = 0;
    GuildSkill = 0;
    GuildMasterKillCount = 0;
    BackupCurrentSkill = 0;
    GuildTeam = 0;
    m_byGensInfluence = 0;
    PK = 0;
    AttackFlag = 0;
    AttackTime = 0;
    LastAttackEffectTime = -1;
    TargetAngle = 0;
    Dead = 0;
    Run = 0;
    Skill = 0;
    SwordCount = 0;
    byExtensionSkill = 0;
    m_byDieType = 0;
    StormTime = 0;
    JumpTime = 0;
    TargetX = 0;
    TargetY = 0;
    SkillX = 0;
    SkillY = 0;
    Appear = 0;
    CurrentSkill = 0;
    CastRenderTime = 0;
    m_byFriend = 0;
    MonsterSkill = 0;

    for (int i = 0; i < 32; ++i) ID[i] = 0;

    Movement = 0;
    MovementType = 0;
    CollisionTime = 0;
    GuildMarkIndex = 0;
    Key = 0;
    TargetCharacter = 0;

    Level = 0;
    MonsterIndex = MONSTER_BULL_FIGHTER;
    Damage = 0;
    Hit = 0;
    MoveSpeed = 0;

    Action = 0;
    ExtendStateTime = 0;
    LongRangeAttack = 0;
    SelectItem = 0;
    Item = 0;
    FreezeType = 0;
    PriorPositionX = 0;
    PriorPositionY = 0;
    PositionX = 0;
    PositionY = 0;
    m_iDeleteTime = 0;
    m_iFenrirSkillTarget = -1;
    LastCritDamageEffect = 0;

    ProtectGuildMarkWorldTime = 0.0f;
    AttackRange = 0.0f;
    Freeze = 0.0f;
    Duplication = 0.0f;
    Rot = 0.0f;

    IdentityVector3D(TargetPosition);
    IdentityVector3D(Light);

    m_pParts = NULL;

    m_pPet = NULL;
    InitPetInfo(PET_TYPE_DARK_HORSE);
    InitPetInfo(PET_TYPE_DARK_SPIRIT);

    for (int i = 0; i < 32; ++i) OwnerID[i] = 0;

    m_pPostMoveProcess = NULL;

    m_pTempParts = NULL;
    m_iTempKey = 0;
    m_CursedTempleCurSkill = AT_SKILL_CURSED_TEMPLE_PRODECTION;
    m_CursedTempleCurSkillPacket = false;
    GensRanking = 0;
    GensContributionPoints = 0;
}

void CHARACTER::Destroy()
{
}

void CHARACTER::InitPetInfo(int iPetType)
{
    m_PetInfo[iPetType].m_dwPetType = PET_TYPE_NONE;
}

PET_INFO* CHARACTER::GetEquipedPetInfo(int iPetType)
{
    return &(m_PetInfo[iPetType]);
}

void CHARACTER::PostMoveProcess_Active(unsigned int uiLimitCount)
{
    if (m_pPostMoveProcess != NULL)
    {
        delete m_pPostMoveProcess; m_pPostMoveProcess = NULL;
    }

    m_pPostMoveProcess = new ST_POSTMOVE_PROCESS();
    m_pPostMoveProcess->bProcessingPostMoveEvent = false;
    m_pPostMoveProcess->uiProcessingCount_PostMoveEvent = uiLimitCount;
}

unsigned int CHARACTER::PostMoveProcess_GetCurProcessCount()
{
    return m_pPostMoveProcess->uiProcessingCount_PostMoveEvent;
}

bool CHARACTER::PostMoveProcess_IsProcessing()
{
    if (m_pPostMoveProcess == NULL)
    {
        return false;
    }

    return (0 <= m_pPostMoveProcess->uiProcessingCount_PostMoveEvent);
}

bool CHARACTER::PostMoveProcess_Process()
{
    if (m_pPostMoveProcess == NULL)
    {
        return false;
    }

    unsigned int uiCurretProcessingCount_ = m_pPostMoveProcess->uiProcessingCount_PostMoveEvent--;

    if (0 >= uiCurretProcessingCount_)
    {
        delete m_pPostMoveProcess;
        m_pPostMoveProcess = NULL;
        return false;
    }

    return true;
}