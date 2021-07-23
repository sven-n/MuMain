// PKSystem.cpp: implementation of the CPKSystem class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP
#include "PKSystem.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPKSystem::CPKSystem()
{
	Init();
}
//////////////////////////////////////////////////////////////////////
CPKSystem::~CPKSystem()
{
	Destroy();
}
//////////////////////////////////////////////////////////////////////
CPKSystem* CPKSystem::GetInstance()
{
	static CPKSystem sInstance;
	return &sInstance;
}
//////////////////////////////////////////////////////////////////////
void CPKSystem::Init()
{
	m_bIsPKState = false;
	m_bIsMoveBuff = false;
	m_HeroPKState = PVP_NEUTRAL;
#ifdef LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER
	m_HeroPartyPKState = m_HeroPKState;
#endif //LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER
}
//////////////////////////////////////////////////////////////////////
void CPKSystem::Destroy()
{
	//n/a
}
//////////////////////////////////////////////////////////////////////
int CPKSystem::GetReqZen(const int& _iZen, int _iValue)
{
	// 블루 뮤 서버가 아니면 계산 필요없음
	if(!BLUE_MU::IsBlueMuServer())
		return _iZen;

	return _iZen*_iValue;
}
//////////////////////////////////////////////////////////////////////
bool CPKSystem::IsPKState(int _iPKState, int _iCharacterPK)
{
	// 블루뮤 서버가 아니면 계산 필요없음
	if(!BLUE_MU::IsBlueMuServer())
		return false;
	// 타겟팅의 pk상태
	if(!_iCharacterPK)
#ifdef LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER
		_iCharacterPK = max(m_HeroPKState, m_HeroPartyPKState);
#else  //LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER
		_iCharacterPK = m_HeroPKState;
#endif //LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER
		
	if(_iCharacterPK >= _iPKState)
		m_bIsPKState = true;
	else 
		m_bIsPKState = false;


	return m_bIsPKState;
}
//////////////////////////////////////////////////////////////////////
void CPKSystem::SetMoveBuffState(const bool& _bIsBuff)
{
	// 버프가 걸려있는 상태
	m_bIsMoveBuff = _bIsBuff;
}
//////////////////////////////////////////////////////////////////////
bool CPKSystem::GetCanDoMoveCommand()
{
	// 블루 뮤 서버가 아닐경우 판단대상
	if(!BLUE_MU::IsBlueMuServer())
		return (m_HeroPKState<PVP_MURDERER1);

	IsPKState();
	// 혼합유료화 서버일경우 판단대상
	// pk상태일때 버프를 받았을 경우만 판단(false)이동 불가
	// 그외 버프 상태는 무시한다(true)
	//*5분 동안 이동명령어(창) 사용 불가 패널티 세부 내용
	//- 무법자 경고 단계에서 pk를 해서 무법자1단계가 될 경우 내부적으로 '이동불가 버프'(유져가 볼 수 없음)를 
	//부여 받게 되어 지속시간(5분) 동안 이동명령어(창)을 사용할 수 없습니다.
	//- 무법자 1단계에서 무법자 2단계로 상승할 때 '이동불가 버프'를 다시 부여 받게 됩니다.
	//- 무법자 2단계에서 PK를 할 때마다 '이동불가 버프'를 부여 받게 됩니다.
	//- '이동불가 버프'는 무법자 등급의 변화와 상관없이 지속됩니다.
	return !(m_bIsPKState&m_bIsMoveBuff);
}
//////////////////////////////////////////////////////////////////////
void CPKSystem::SetHeroPKState(const BYTE& _byHeroPK)
{
	m_HeroPKState = _byHeroPK;
}

#ifdef LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER 
void CPKSystem::SetHeroPartyPKState(const BYTE& _byHeroPartyPK)
{
	m_HeroPartyPKState = _byHeroPartyPK;
}
#endif //LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER
#endif //PBG_ADD_PKSYSTEM_INGAMESHOP