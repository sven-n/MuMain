// HackTest.h: interface for the CHackTest class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_HACKTEST_H__04C83ED2_C91F_491C_BE3F_1B8DDD1EE6B9__INCLUDED_)
#define AFX_HACKTEST_H__04C83ED2_C91F_491C_BE3F_1B8DDD1EE6B9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef CSK_HACK_TEST


// 몬스터가 살아있는가, 죽었는가 판별하는 함수
bool IsLiveMonster(CHARACTER* pCha);


class CManagedItem 
{
public:
	CManagedItem():m_iRefCount(0) {}
	virtual ~CManagedItem() {}

	void AddRef()
	{
		++m_iRefCount;
	}

	bool DelRef()
	{
		return --m_iRefCount==0;
	}

private:
	int m_iRefCount;

};

template <class IDTYPE>
class CManager
{
public:
	CManager(void) {}

	void clear()
	{
		m_Names.clear();
		m_Items.clear();
	}
	
	virtual IDTYPE add(std::string name) { IDTYPE id; return id; }
	virtual void add(IDTYPE id, std::string name) {}

	virtual void Delete(IDTYPE id)
	{
		if(has(id))
		{
			if (m_Items[id]->DelRef()) 
			{
				CManagedItem *i = m_Items[id];
				DoDelete(id);
				m_Items.erase(m_Items.find(id));
				delete i;
			}
		}
	}

	void DeleteAll()
	{
		for (std::map<IDTYPE, CManagedItem*>::iterator it = m_Items.begin(); it != m_Items.end(); ++it) 
		{
			SAFE_DELETE( it->second );
		}

		clear();
	}

	virtual void DoDelete(IDTYPE id) {}

	bool has(IDTYPE id)
	{
		return (m_Items.find(id) != m_Items.end());
	}

	CManagedItem* get(IDTYPE id)
	{
		CManagedItem* pItem = NULL;

		if(has(id))
			pItem = m_Items[id];

		return pItem;
	}

protected:
	void DoAdd(IDTYPE id, CManagedItem* item)
	{
		item->AddRef();
		m_Items[id] = item;
	}

public:
	std::map<IDTYPE, CManagedItem*> m_Items;
};

class CMonster : public CManagedItem
{
public:
	CMonster();
	virtual ~CMonster();

public:
	short m_iIndex;		// 몬스터 인덱스
	DWORD m_dwTime;		// 공속 측정 타이머
};


class CMonsterManager	: public CManager<int>
{	
public:
	CMonsterManager();
	virtual ~CMonsterManager();

	// key값으로 몬스터 추가
	void Add(int iKey, int iIndex);

	// 리스트에서 죽은 몬스터는 삭제한다.
	void DeleteDeadMonster();

	// 리스트 사이즈
	int Size();

	CMonster* GetMonster();

	void Reset();

	void Add(int iIndex);

private:
	int m_iIndex;
	int m_iKey;

public:
	vector<int> m_MonsterIndexList;
};

class HackLog
{
public:
	typedef std::map<int, int> SkillAttackMap;
	
	void Reset();

	// 로그시작
	bool m_bLogStart;

	// 시작시간
	DWORD m_dwStartTime;
	// 끝시간
	DWORD m_dwEndTime;
	// 총시간
	DWORD m_dwTotalTime;

	// 총데미지
	// ReceiveAttackDamage()
	int m_iTotalDamage;
	// 총 방어데미지 
	int m_iTotalShieldDamage;
	// 콤보데미지 카운트
	int m_iComboDamageCount;
	// 더블데미지 카운트
	int m_iDoubleDamageCount;
	// 데미지입힌 카운트
	int m_iDamageCount;
	// 데미지 평균치
	int m_iAverageDamage;
	// miss 카운트
	int m_iMissCount;

	// 죽은 몬스터 수
	int m_iDieMonsterCount;

	// 공격패킷 날린 수
	int m_iSendClientAttackPacket;

	// 스킬별로 카운트 수
	SkillAttackMap m_SkillAttackCountList;

	// 시작할 당시 선택된 스킬
	WORD m_wCurrentSkill;

	DWORD m_dwTime;
	int m_iKey;
	CHARACTER* m_pMonster;
	int m_iSelectedMonster;
};

class CHackTest  
{
public:
	enum HackType
	{
		HACK_NONE = 0,
		HACK_AUTOATTACK,		// 일반 공격
		HACK_PILLAR_OF_FIRE,	// 불기둥
		HACK_EVILSPIRIT,		// 악령
		HACK_DABAL,				// 다발
		HACK_SKILL_LOG,			// 스킬분석
	};

	CHackTest();
	virtual ~CHackTest();
	static CHackTest* GetInstance();
	
public:
	// 핵 기능중 아무위치로 이동하는 기능
	// x, y 좌표만 넣으면 어디로든 이동 가능함
	void MovePos(int x, int y);
	
	// 맵에 나와있는 제일 빨리 찾는 몬스터한테 이동하는 함수
	void MoveMonster();

	void MoveAuto();
	
	// 맵에 나와있는 임의의 몬스터를 찾아서 공격하는 함수
	void AttackMonster();
	
	// 맵에 나와있는 임의의 몬스터를 찾는 함수
	CHARACTER* FindMonster();
	
	// 뷰포트 안에 들어와 있는 모든 몬스터를 찾는다.
	void FindAllMonster();

	void PlusAttackSpeed();
	void MinusAttackSpeed();
	
	void StartAuto(int iType);
	void EndAuto();
	
	void Update();
	void UpdateAutoAttack();
	void UpdatePillarOfFire();
	void UpdateEvilSpirit();
	void UpdateDaval();
	void UpdateSkillLog();

	// 스킬분석함수
	void StartSkill();
	void EndSkill();
	void ReceiveDamage(CHARACTER* pCha, OBJECT* pObj, int iSuccess, WORD wDamage, int iDamageType, bool bDoubleEnable, bool bComboEnable, WORD wShieldDamage, int iKey);
	void ReceiveDie(CHARACTER* pCha, OBJECT* pObj, WORD wSkillType, int iKey);
	void AddAttackCount(int iSkill);

public:
	int m_iAttackSpeed;
	
private:
	int m_iHackType;

	DWORD m_dwTime;
	DWORD m_dwAttackTime;
	
	int m_iCount;

	CMonsterManager m_MonsterManager;

	HackLog m_SkillLog;
};

#define g_pHackTest CHackTest::GetInstance()
#endif // CSK_HACK_TEST

#endif // !defined(AFX_HACKTEST_H__04C83ED2_C91F_491C_BE3F_1B8DDD1EE6B9__INCLUDED_)
