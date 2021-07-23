// PKSystem.h: interface for the CPKSystem class.
//
// 혼합유료화 서버에서의 PK 시스템
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PKSYSTEM_H__646E4968_61BD_45E8_8648_CC6C6B5CAA31__INCLUDED_)
#define AFX_PKSYSTEM_H__646E4968_61BD_45E8_8648_CC6C6B5CAA31__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef PBG_ADD_PKSYSTEM_INGAMESHOP

class CPKSystem  
{
public:
	CPKSystem();
	virtual ~CPKSystem();
	
	void Init();
	void Destroy();	
	static CPKSystem* GetInstance();

	// 요구젠값 계산하기 (몇배를 적용할것인가)
	int GetReqZen(const int& _iZen, int _iValue = 10);
	// 현재 상태를 확인한다(기본 무법자1단계보다 이상인가)
	bool IsPKState(int _iPKState = PVP_MURDERER1, int _iCharacterPK = 0);
	const bool& GetPKState() const;
	// 이동 불가 버프상태
	void SetMoveBuffState(const bool& _bIsBuff);
	const bool& GetMoveBuffState() const;
	// 이동창 사용가능 상태인가?
	bool GetCanDoMoveCommand();
	// Hero의 PK상태
	void SetHeroPKState(const BYTE& _byHeroPK);
	const BYTE& GetHeroPKState() const;
#ifdef LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER 
	// 서버에서 받은 player가 속해 있는 파티원의 pk상태 중 가장 높은 값 설정
	void SetHeroPartyPKState(const BYTE& _byHeroPartyPK);
#endif //LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER

private:
	// eDeBuff_MoveCommandWin버프상태
	bool m_bIsMoveBuff;
	// 혼합유료화서버에서의 PK요구상태값
	bool m_bIsPKState;
	//현 Hero->pk상태
	BYTE m_HeroPKState;
#ifdef LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER 
	// 현재 Hero->PKPartyLevel(player가 속해 있는 파티원의 pk상태 중 가장 높은 값)
	BYTE m_HeroPartyPKState;
#endif //LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER
};

inline const bool& CPKSystem::GetPKState() const
{
	return m_bIsPKState; 
}

inline const bool& CPKSystem::GetMoveBuffState() const
{
	return m_bIsMoveBuff; 
}

inline const BYTE& CPKSystem::GetHeroPKState() const
{
	return m_HeroPKState; 
}

#define g_PKSystem	CPKSystem::GetInstance()

#endif //PBG_ADD_PKSYSTEM_INGAMESHOP
#endif // !defined(AFX_PKSYSTEM_H__646E4968_61BD_45E8_8648_CC6C6B5CAA31__INCLUDED_)
