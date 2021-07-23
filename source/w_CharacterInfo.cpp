// w_CharacterInfo.cpp: implementation of the CHARACTER class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_CharacterInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

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
	m_bFixForm = false;         //  데미지를 맞아도 고정된 형태에서 변경되지 않는 캐릭터.	
	Foot[0] = false;
	Foot[1] = false;
	SafeZone = false;
	Change = false;
	HideShadow = false;
	m_bIsSelected = false;
	Decoy = false;

	Class = 0;
	Skin = 0;
	CtlCode = 0;
	ExtendState = 0;			//  0:無, 1:풀세트아이템
	EtcPart = 0;				//  기타 부탁 아이템. ( 퀘스트용 )
	GuildStatus = 0;			//  길드내에서의 직책 ( -1 : NONE, 0 : PERSON, 32 : BATTLE_MASTER, 64 : SUB_MASTER, 128 : MASTER )
	GuildType = 0;				//  길드종류 일반(0), 수호길드(1)
	GuildRelationShip = 0;	    //  길드관계 (0x00 : 관계없음, 0x01 : Union, 0x04 Union Master, 0x02 : Rival, 0x08: Rival Union )
	GuildSkill = 0;             //  배틀 마스터의 길드 스킬.
	GuildMasterKillCount = 0;   //  배틀 마스터 킬카운트.
	BackupCurrentSkill = 0;     //  스킬 인덱스 백업.
	GuildTeam = 0;
#ifdef ASG_ADD_GENS_SYSTEM
	m_byGensInfluence = 0;		//	겐스 세력.(0:없음, 1:듀프리언, 2:바네르트)
#endif	// ASG_ADD_GENS_SYSTEM
	PK = 0;
	AttackFlag = 0;
	AttackTime = 0;
	TargetAngle = 0;
	Dead = 0;
	Run = 0;
	Skill = 0;
	SwordCount = 0;
	byExtensionSkill = 0;		//	확장 스킬. ( 0: 없음, 1:기본콤보, 2: 등등등 ).
	m_byDieType = 0;			//	죽는 형식.
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

#if SELECTED_LANGUAGE != LANGUAGE_KOREAN
	for( int i = 0; i < 32; ++i ) ID[i] = 0;
#else //SELECTED_LANGUAGE != LANGUAGE_KOREAN
	for( int i = 0; i < 24; ++i ) ID[i] = 0;
#endif //SELECTED_LANGUAGE != LANGUAGE_KOREAN

	Movement = 0;
	MovementType = 0;
	CollisionTime = 0;
	GuildMarkIndex = 0;		// 내가 속한 길드의 길드마크버퍼에서의 인덱스
	Key = 0;
	TargetCharacter = 0;

	Level = 0;
	MonsterIndex = 0;
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

	ProtectGuildMarkWorldTime = 0.0f;
	AttackRange = 0.0f;
	Freeze = 0.0f;
	Duplication = 0.0f;
	Rot = 0.0f;

	IdentityVector3D( TargetPosition );
	IdentityVector3D( Light );

	m_pParts = NULL;

	// 펫 정보
#ifdef PET_SYSTEM
	m_pPet = NULL;
#endif// PET_SYSTEM
#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
	InitPetInfo(PET_TYPE_DARK_HORSE);
	InitPetInfo(PET_TYPE_DARK_SPIRIT);
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM

#ifdef YDG_ADD_NEW_DUEL_SYSTEM
#ifdef _VS2008PORTING
#if SELECTED_LANGUAGE != LANGUAGE_KOREAN
	for( int i = 0; i < 32; ++i ) OwnerID[i] = 0;
#else //SELECTED_LANGUAGE != LANGUAGE_KOREAN
	for( int i = 0; i < 24; ++i ) OwnerID[i] = 0;
#endif //SELECTED_LANGUAGE != LANGUAGE_KOREAN
#else // _VS2008PORTING
#if SELECTED_LANGUAGE != LANGUAGE_KOREAN
	for( i = 0; i < 32; ++i ) OwnerID[i] = 0;
#else //SELECTED_LANGUAGE != LANGUAGE_KOREAN
	for( i = 0; i < 24; ++i ) OwnerID[i] = 0;
#endif //SELECTED_LANGUAGE != LANGUAGE_KOREAN
#endif // _VS2008PORTING
#endif	// YDG_ADD_NEW_DUEL_SYSTEM

#ifdef LDS_FIX_MODULE_POSTMOVECHARACTER_SKILL_WHEEL
	m_pPostMoveProcess	= NULL;
#endif // LDS_FIX_MODULE_POSTMOVECHARACTER_SKILL_WHEEL

	m_pTempParts = NULL;
	m_iTempKey = 0;
	m_CursedTempleCurSkill = AT_SKILL_CURSED_TEMPLE_PRODECTION;
	m_CursedTempleCurSkillPacket = false;
#ifdef PBG_ADD_GENSRANKING
	m_byRankIndex=0;
#endif //PBG_ADD_GENSRANKING
#ifdef PBG_MOD_STRIFE_GENSMARKRENDER
	m_nContributionPoint=0;
#endif //PBG_MOD_STRIFE_GENSMARKRENDER
}

void CHARACTER::Destroy()
{

}

#ifdef KJH_FIX_DARKLOAD_PET_SYSTEM
void CHARACTER::InitPetInfo(int iPetType)
{
	m_PetInfo[iPetType].m_dwPetType = PET_TYPE_NONE;
}

PET_INFO* CHARACTER::GetEquipedPetInfo(int iPetType)
{
	return &(m_PetInfo[iPetType]);
}
#endif // KJH_FIX_DARKLOAD_PET_SYSTEM



// MoveCharacter함수상에 사용자 지정 내용을 RenderCharacter 함수상에서 사용하고자 함입니다.
#ifdef LDS_FIX_MODULE_POSTMOVECHARACTER_SKILL_WHEEL

// 제한 Counting을 두어 PostMoveProcess를 활성화 합니다.
void CHARACTER::PostMoveProcess_Active( unsigned int uiLimitCount )
{
	if( m_pPostMoveProcess != NULL )
	{
		delete m_pPostMoveProcess; m_pPostMoveProcess = NULL;
	}

	m_pPostMoveProcess = new ST_POSTMOVE_PROCESS();
	m_pPostMoveProcess->bProcessingPostMoveEvent = false;
	m_pPostMoveProcess->uiProcessingCount_PostMoveEvent = uiLimitCount;
}

// 현재 Counting을 반환 합니다.
unsigned int CHARACTER::PostMoveProcess_GetCurProcessCount()
{
	return m_pPostMoveProcess->uiProcessingCount_PostMoveEvent;
}

// 현재 PostMoveProcess가 Processing중인지 반환 합니다.
bool CHARACTER::PostMoveProcess_IsProcessing()
{
	if( m_pPostMoveProcess == NULL)
	{
		return false;
	}
	
	return (0 <= m_pPostMoveProcess->uiProcessingCount_PostMoveEvent);
}

// Tick Process 합니다.
bool CHARACTER::PostMoveProcess_Process()
{
	if( m_pPostMoveProcess == NULL )
	{
		return false;
	}
	
	// 한Tick당 한번씩 감산 시킵니다.
	unsigned int uiCurretProcessingCount_ = m_pPostMoveProcess->uiProcessingCount_PostMoveEvent--;

	if( 0 >= uiCurretProcessingCount_)
	{
		delete m_pPostMoveProcess;
		m_pPostMoveProcess = NULL;
		return false;
	}
	
	return true;
}

#endif // LDS_FIX_MODULE_POSTMOVECHARACTER_SKILL_WHEEL
