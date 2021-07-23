// HackTest.cpp: implementation of the CHackTest class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#ifdef CSK_HACK_TEST
#include "HackTest.h"
#include "wsclientinline.h"
#include "ZzzLodTerrain.h"
#include "ZzzEffect.h"
#include "./Utilities/Log/muConsoleDebug.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

bool IsLiveMonster(CHARACTER* pCha)
{
	OBJECT* pObj = &pCha->Object;
	
	if(pCha == NULL || pCha->Dead > 0 || pObj->Live == false || pObj->Kind != KIND_MONSTER)
	{
		return false;
	}
	
	return true;
}

CHackTest::CHackTest()
{
	m_iHackType = HACK_NONE;
	m_iCount = 0;
	m_dwTime = 0;

	m_dwAttackTime = 0;
	m_iAttackSpeed = 100;
}

CHackTest::~CHackTest()
{

}

CHackTest* CHackTest::GetInstance()
{
	static CHackTest s_HackTest;
	return &s_HackTest;
}

void CHackTest::MovePos(int x, int y)
{
	SendPosition(x, y);
}

void CHackTest::MoveMonster()
{
	for(int i=0; i<MAX_CHARACTERS_CLIENT; i++)
	{
		CHARACTER *c = &CharactersClient[i];
		OBJECT *o = &c->Object;
		if(o->Live && o->Kind == KIND_MONSTER)
		{
			MovePos(c->PositionX, c->PositionY);
			break;
		}
	}
}

void CHackTest::MoveAuto()
{
	int x, y;

	x = Hero->PositionX + rand()%20 - 10;
	y = Hero->PositionY + rand()%20 - 10;
	
	while(true)
	{
		int iTerrainIndex = TERRAIN_INDEX_REPEAT(x, y);
		if((TerrainWall[iTerrainIndex]&TW_SAFEZONE) != TW_SAFEZONE && (TerrainWall[iTerrainIndex]&TW_NOMOVE) != TW_NOMOVE && (TerrainWall[iTerrainIndex]&TW_NOGROUND) != TW_NOGROUND)
		{
			break;
		}
		
		x = Hero->PositionX + rand()%20 - 10;
		y = Hero->PositionY + rand()%20 - 10;
	}
	
	if(x != Hero->PositionX || y != Hero->PositionY)
	{
		MovePos(x, y);
	}
	else
	{
		MovePos(x+(rand()%3-1), y+(rand()%3-1));
	}
}

void CHackTest::AttackMonster()
{
	CHARACTER* pCha = FindMonster();
	if(pCha == NULL)
		return;

	MovePos(pCha->PositionX, pCha->PositionY);
}

CHARACTER* CHackTest::FindMonster()
{
	for(int i=0; i<MAX_CHARACTERS_CLIENT; i++)
	{
		CHARACTER *pCha = &CharactersClient[i];
		if(IsLiveMonster(pCha) == true)
		{
			return pCha;
		}
	}

	return NULL;
}

void CHackTest::FindAllMonster()
{
	m_MonsterManager.Reset();
	m_MonsterManager.m_MonsterIndexList.clear();

	if(m_MonsterManager.Size() > 0)
	{
		m_MonsterManager.DeleteDeadMonster();
	}

	for(int i=0; i<MAX_CHARACTERS_CLIENT; ++i)
	{
		CHARACTER *pCha = &CharactersClient[i];
		if(IsLiveMonster(pCha) == true)
		{
			m_MonsterManager.Add(pCha->Key, i);
			m_MonsterManager.m_MonsterIndexList.push_back(i);
		}
	}

	int iSize = m_MonsterManager.m_MonsterIndexList.size();
	for(i=0; i<iSize; ++i)
	{
		int iIndex = m_MonsterManager.m_MonsterIndexList[i];
	}
}

void CHackTest::PlusAttackSpeed()
{
	m_iAttackSpeed++;
}

void CHackTest::MinusAttackSpeed()
{
	m_iAttackSpeed--;
	if(m_iAttackSpeed < 0)
	{
		m_iAttackSpeed = 0;
	}
}

void CHackTest::StartAuto(int iType)
{
	m_iHackType = iType;

	switch(iType)
	{
	case HACK_AUTOATTACK:
		break;
	case HACK_PILLAR_OF_FIRE:
		break;
	case HACK_EVILSPIRIT:
		break;
	}
		
	m_iCount = 0;
	m_dwTime = 0;

	
}

void CHackTest::EndAuto()
{
	m_iHackType = HACK_NONE;
}

void CHackTest::Update()
{
// 	if(m_iHackType > 0)
// 	{
// 		DWORD dwTime = timeGetTime();
// 		int iSize = m_MonsterManager.Size();
// 		
// 		if(iSize == 0 || (dwTime - m_dwTime > 1000))
// 		{
// 			m_dwTime = dwTime;
// 			FindAllMonster();
// 			iSize = m_MonsterManager.Size();
// 			
// 			if(iSize == 0)
// 			{
// 				m_iCount++;
// 				if(m_iCount > 10)
// 				{
// 					m_iCount = 0;
// 					
// 					//MoveAuto();
// 				}
// 				
// 				return;
// 			}
// 		}
// 	}

	if(m_iHackType > 0)
	{
		DWORD dwTime = timeGetTime();
		if(dwTime - m_dwTime > 1000)
		{
			m_dwTime = dwTime;
			FindAllMonster();

			int iSize = m_MonsterManager.m_MonsterIndexList.size();
			if(iSize == 0)
				return;
		}
	}
	
	switch(m_iHackType)
	{
	case HACK_AUTOATTACK:
		UpdateAutoAttack();
		break;
	case HACK_PILLAR_OF_FIRE:
		UpdatePillarOfFire();
		break;
	case HACK_EVILSPIRIT:
		UpdateEvilSpirit();
		break;
	case HACK_DABAL:
		UpdateDaval();
		break;
	case HACK_SKILL_LOG:
		UpdateSkillLog();
		break;
	}
}

void CHackTest::UpdateAutoAttack()
{
	DWORD dwTime = timeGetTime();

	int iSize = m_MonsterManager.m_MonsterIndexList.size();

	if(dwTime - m_dwAttackTime > (m_iAttackSpeed * 10))
	{
		m_dwAttackTime = dwTime;
		for(int i=0; i<iSize; ++i)
		{
			int iIndex = m_MonsterManager.m_MonsterIndexList[i];
			CHARACTER* pCha = &CharactersClient[iIndex];
			OBJECT* pObj = &pCha->Object;
			
			if(IsLiveMonster(pCha) == true)
			{
				int iDir = ((BYTE)((pObj->Angle[2]+22.5f)/360.f*8.f+1.f)%8);
	 			short sKey = pCha->Key;
 				SendRequestAttack(sKey, iDir);
			}
			else
			{

			}
		}
	}

	//m_iHackType = HACK_NONE;
	
// 	CMonster* pMonster = m_MonsterManager.GetMonster();
// 	if(pMonster == NULL)
// 		return;
// 	
// 	CHARACTER* pCha = &CharactersClient[pMonster->m_iIndex];
// 	OBJECT* pObj = &pCha->Object;
// 
// 	if(IsLiveMonster(pCha) == true)
// 	{
// 		int iDir = ((BYTE)((pObj->Angle[2]+22.5f)/360.f*8.f+1.f)%8);
// 		short sKey = pCha->Key;
// 		
// 		//if(dwTime - pMonster->m_dwTime > 40)
// 		if(dwTime - m_dwAttackTime > (m_iAttackSpeed * 10))
// 		{
// 			m_dwAttackTime = dwTime;
// 			//pMonster->m_dwTime = dwTime;
// 			SendRequestAttack(sKey, iDir);
// 		}
// 	}
// 	else
// 	{
// 		m_MonsterManager.Delete(pCha->Key);
// 	}
}

void CHackTest::UpdatePillarOfFire()
{
	DWORD dwTime = timeGetTime();
	static DWORD s_dwTime = 0;
	
	CMonster* pMonster = m_MonsterManager.GetMonster();
	if(pMonster == NULL)
		return;
	
	CHARACTER* pCha = &CharactersClient[pMonster->m_iIndex];
	OBJECT* pObj = &pCha->Object;
	
	if(IsLiveMonster(pCha) == true)
	{
		OBJECT* pHeroObj = &Hero->Object;
		if(dwTime - pMonster->m_dwTime > 1500)
		{
			pMonster->m_dwTime = dwTime;
			SendRequestMagicContinue(AT_SKILL_FLAME, (BYTE)(Hero->PositionX), (BYTE)(Hero->PositionY), (BYTE)(pHeroObj->Angle[2]/360.f*256.f), 0, 0, 0xffff, 0);
		}
		
		if(dwTime - s_dwTime > 100)
		{
			s_dwTime = dwTime;
			
			vec3_t vPos;
			vPos[0] = (float)(pCha->PositionX+0.5f)*TERRAIN_SCALE;
			vPos[1] = (float)(pCha->PositionY+0.5f)*TERRAIN_SCALE;
			vPos[2] = RequestTerrainHeight(vPos[0], vPos[1]);
			CreateEffect(BITMAP_FLAME, vPos, pHeroObj->Angle, pHeroObj->Light, 0, pHeroObj, pHeroObj->PKKey, FindHotKey(AT_SKILL_FLAME));
			++CharacterMachine->PacketSerial;
		}
	}
	else
	{
		m_MonsterManager.Delete(pCha->Key);
	}

// 	DWORD dwTime = timeGetTime();
// 	static DWORD s_dwTime = 0;
// 	static DWORD s_dwTime2 = 0;
// 
// 	if(m_MonsterManager.Size() <= 0)
// 		return;
// 
// 	OBJECT* pHeroObj = &Hero->Object;
// 
// 	if(dwTime - s_dwTime > 1500)
// 	{
// 		s_dwTime = dwTime;
// 		SendRequestMagicContinue(AT_SKILL_FLAME, (BYTE)(Hero->PositionX), (BYTE)(Hero->PositionY), (BYTE)(pHeroObj->Angle[2]/360.f*256.f), 0, 0, 0xffff, 0);
// 	}
// 		
// 	if(dwTime - s_dwTime2 > 100)
// 	{
// 		s_dwTime2 = dwTime;
// 		CMonster* pMonster = m_MonsterManager.GetMonster();
// 		if(pMonster == NULL)
// 			return;
// 		CHARACTER* pCha = &CharactersClient[pMonster->m_iIndex];
// 	 	OBJECT* pObj = &pCha->Object;
// 		int iKeyList[5] = {0, 0, 0, 0, 0};
// 		iKeyList[0] = pCha->Key;
// 		SendRequestMagicAttack(AT_SKILL_FLAME, pCha->PositionX, pCha->PositionY, pObj->Weapon, 1, iKeyList, 0);
// 		++CharacterMachine->PacketSerial;
// 	}
}

void CHackTest::UpdateEvilSpirit()
{
// 	DWORD dwTime = timeGetTime();
// 	static DWORD s_dwTime = 0;
// 	
// 	CMonster* pMonster = m_MonsterManager.GetMonster();
// 	if(pMonster == NULL)
// 		return;
// 	
// 	CHARACTER* pCha = &CharactersClient[pMonster->m_iIndex];
// 	OBJECT* pObj = &pCha->Object;
// 	
// 	if(IsLiveMonster(pCha) == true)
// 	{
// 		OBJECT* pHeroObj = &Hero->Object;
// 		if(dwTime - pMonster->m_dwTime > 1500)
// 		{
// 			pMonster->m_dwTime = dwTime;
// 			SendRequestMagicContinue(AT_SKILL_EVIL, (BYTE)(Hero->PositionX), (BYTE)(Hero->PositionY), (BYTE)(pHeroObj->Angle[2]/360.f*256.f), 0, 0, 0xffff, &pHeroObj->m_bySkillSerialNum);
// 		}
// 		
// 		if(dwTime - s_dwTime > 100)
// 		{
// 			s_dwTime = dwTime;
// 			
// 			vec3_t vPos;
// 			VectorCopy(pHeroObj->Position, vPos);
// 			vPos[2] += 100.f;
// 			int iSkillIndex = FindHotKey(AT_SKILL_EVIL);
// 			//for(int i=0; i<4; ++i)
// 			{
// 				CreateJoint(BITMAP_JOINT_SPIRIT,vPos,pHeroObj->Position,pHeroObj->Angle,0,pHeroObj,80.f,pHeroObj->PKKey,iSkillIndex,pHeroObj->m_bySkillSerialNum);
// 			}
// 			++CharacterMachine->PacketSerial;
// 		}
// 	}
// 	else
// 	{
// 		m_MonsterManager.Delete(pCha->Key);
// 	}

	DWORD dwTime = timeGetTime();
	static DWORD s_dwTime = 0;
	static DWORD s_dwTime2 = 0;

	if(m_MonsterManager.Size() <= 0)
		return;

	OBJECT* pHeroObj = &Hero->Object;

	if(dwTime - s_dwTime > 1500)
	{
		s_dwTime = dwTime;
		SendRequestMagicContinue(AT_SKILL_EVIL, (BYTE)(Hero->PositionX), (BYTE)(Hero->PositionY), (BYTE)(pHeroObj->Angle[2]/360.f*256.f), 0, 0, 0xffff, &pHeroObj->m_bySkillSerialNum);
	}
		
	if(dwTime - s_dwTime2 > 100)
	{
		s_dwTime2 = dwTime;
		CMonster* pMonster = m_MonsterManager.GetMonster();
		if(pMonster == NULL)
			return;
		CHARACTER* pCha = &CharactersClient[pMonster->m_iIndex];
	 	OBJECT* pObj = &pCha->Object;
		int iKeyList[5] = {0, 0, 0, 0, 0};
		iKeyList[0] = pCha->Key;
		SendRequestMagicAttack(AT_SKILL_EVIL, pCha->PositionX, pCha->PositionY, pHeroObj->Weapon, 1, iKeyList, pHeroObj->m_bySkillSerialNum);
		++CharacterMachine->PacketSerial;
	}
}

void CHackTest::UpdateDaval()
{
	DWORD dwTime = timeGetTime();
	
	int iSize = m_MonsterManager.m_MonsterIndexList.size();
	int iCount = 0;
	int iDamageKeyList[5];
	OBJECT* pHeroObj = &Hero->Object;

	for(int i=0; i<iSize; ++i)
	{
		int iIndex = m_MonsterManager.m_MonsterIndexList[i];
		CHARACTER* pCha = &CharactersClient[iIndex];
		OBJECT* pObj = &pCha->Object;
		
		if(IsLiveMonster(pCha) == true)
		{
			iDamageKeyList[iCount] = pCha->Key;
			iCount++;
		}
		else
		{
			
		}
	}
	
	if(dwTime - m_dwAttackTime > (m_iAttackSpeed * 10))
	{
		m_dwAttackTime = dwTime;

		SendRequestMagicAttack(AT_SKILL_CROSSBOW, (BYTE)(Hero->PositionX), (BYTE)(Hero->PositionY), pHeroObj->Weapon, iCount, iDamageKeyList, 0);
		pHeroObj->Weapon++;
		++CharacterMachine->PacketSerial;
	}
}

void CHackTest::UpdateSkillLog()
{
	if(m_SkillLog.m_bLogStart == false)
		return;

	

	if(m_SkillLog.m_iKey != -1)
	{
		int iIndex = FindCharacterIndex(m_SkillLog.m_iKey);
		if(iIndex == 400)
		{
			m_SkillLog.m_iKey = -1;
			m_SkillLog.m_pMonster = NULL;
		}
		else
		{
			CHARACTER* pCha = &CharactersClient[iIndex];
			if(IsLiveMonster(pCha) == false)
			{
				m_SkillLog.m_iKey = -1;
				m_SkillLog.m_pMonster = NULL;
			}
		}
	}



	DWORD dwTime = timeGetTime();
	if(dwTime - m_SkillLog.m_dwTime > 100)
	{
		m_SkillLog.m_dwTime = dwTime;

		if(m_SkillLog.m_iKey == -1)
		{
			int iSize = m_MonsterManager.m_MonsterIndexList.size();
			if(iSize > 0)
			{
				int iIndex = m_MonsterManager.m_MonsterIndexList[0];
				CHARACTER* pCha = &CharactersClient[iIndex];
				m_SkillLog.m_iKey = pCha->Key;
				m_SkillLog.m_pMonster = pCha;
				m_SkillLog.m_iSelectedMonster = iIndex;
			}
		}
	}

	// 1시간 이후에 자동 종료
	if(dwTime - m_SkillLog.m_dwStartTime > 3600000)
	{
		EndSkill();
		return;
	}

	if(m_SkillLog.m_iKey == -1 || m_SkillLog.m_pMonster == NULL)
	{
		return;
	}

	SelectedCharacter = m_SkillLog.m_iSelectedMonster;
	Attacking = 2;
}

CMonster::CMonster()
{
	m_iIndex = -1;
	m_dwTime = 0;
}

CMonster::~CMonster()
{

}

CMonsterManager::CMonsterManager()
{
	m_iKey = 0;
}

CMonsterManager::~CMonsterManager()
{

}

void CMonsterManager::Add(int iKey, int iIndex)
{
	if(m_Items.find(iKey) != m_Items.end()) 
	{
		m_Items[iKey]->AddRef();
		return;
	}
	
	CMonster* pMonster = new CMonster;
	pMonster->m_iIndex = iIndex;
	
	DoAdd(iKey, pMonster);	
}

void CMonsterManager::DeleteDeadMonster()
{
	for(std::map<int, CManagedItem*>::iterator it = m_Items.begin(); it != m_Items.end();) 
	{
		std::map<int, CManagedItem*>::iterator itOld = it;
		it++;
		
		CHARACTER* pCha = (CHARACTER*)(itOld->second);

		if(IsLiveMonster(pCha) == false)
		{
			SAFE_DELETE(pCha);
			m_Items.erase(itOld);
		}
	}
}

int CMonsterManager::Size()
{
	return m_Items.size();
}

CMonster* CMonsterManager::GetMonster()
{
	int iCount = 0;
	int iSize = Size();
	if(m_iIndex >= iSize)
	{
		m_iIndex = 0;
	}
	for(std::map<int, CManagedItem*>::iterator it = m_Items.begin(); it != m_Items.end(); ++it, ++iCount)
	{
 		int iKey = it->first;
// 		if(iCount == 0 && (iKey != m_iKey || iSize == 1))
// 		{
// 			m_iKey = iKey;
// 			return (CMonster*)(it->second);
// 		}

		if(iCount == m_iIndex)
		{
			m_iIndex++;
			if(m_iIndex >= iSize)
			{
				m_iIndex = 0;
			}
			return (CMonster*)(it->second);
		}
	}

	m_iKey = 0;

	return NULL;
}	

void CMonsterManager::Reset()
{
	//m_iIndex = 0;
}

void CMonsterManager::Add(int iIndex)
{
	m_MonsterIndexList.push_back(iIndex);
}

void CHackTest::StartSkill()
{
	StartAuto(HACK_SKILL_LOG);

	m_SkillLog.Reset();

	m_SkillLog.m_bLogStart = true;
	m_SkillLog.m_dwStartTime = timeGetTime();

	m_SkillLog.m_wCurrentSkill = Hero->CurrentSkill;
	
	SYSTEMTIME st;
	GetLocalTime( &st);
#ifdef CONSOLE_DEBUG
	g_ConsoleDebugLog->log("시작 %4d/%02d/%02d %02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
#endif // CONSOLE_DEBUG
}

void CHackTest::EndSkill()
{
	EndAuto();

	m_SkillLog.m_dwEndTime = timeGetTime();
	m_SkillLog.m_dwTotalTime = m_SkillLog.m_dwEndTime - m_SkillLog.m_dwStartTime;
	// 총 사용 시간 단위 초
	int iSecond = m_SkillLog.m_dwTotalTime / 1000;
	m_SkillLog.m_iAverageDamage = m_SkillLog.m_iTotalDamage / iSecond;

	SYSTEMTIME st;
	GetLocalTime( &st);
#ifdef CONSOLE_DEBUG
	g_ConsoleDebugLog->log("끝 %4d/%02d/%02d %02d:%02d\n", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
	
	g_ConsoleDebugLog->log("**************** 분석 결과 시작 ******************\n");
	
	g_ConsoleDebugLog->log("시작시간 : %d\n", m_SkillLog.m_dwStartTime);
	g_ConsoleDebugLog->log("끝시간 : %d\n", m_SkillLog.m_dwEndTime);
	g_ConsoleDebugLog->log("총시간 : %d\n", m_SkillLog.m_dwTotalTime);
	
	g_ConsoleDebugLog->log("총데미지 : %d\n", m_SkillLog.m_iTotalDamage);
	g_ConsoleDebugLog->log("총방어데미지 : %d\n", m_SkillLog.m_iTotalShieldDamage);
	g_ConsoleDebugLog->log("데미지카운트 : %d\n", m_SkillLog.m_iDamageCount);
	g_ConsoleDebugLog->log("콤보데미지카운트 : %d\n", m_SkillLog.m_iComboDamageCount);
	g_ConsoleDebugLog->log("더블데미지카운트 : %d\n", m_SkillLog.m_iDoubleDamageCount);
	g_ConsoleDebugLog->log("총MISS카운트 : %d\n", m_SkillLog.m_iMissCount);

	g_ConsoleDebugLog->log("몬스터 죽은 수 : %d\n", m_SkillLog.m_iDieMonsterCount);

	g_ConsoleDebugLog->log("클라이언트에서 판별하는 공격패킷 보낸 횟수 : %d\n", m_SkillLog.m_iSendClientAttackPacket);
	
	HackLog::SkillAttackMap::iterator it = m_SkillLog.m_SkillAttackCountList.begin();
	for(; it != m_SkillLog.m_SkillAttackCountList.end(); ++it)
	{
		g_ConsoleDebugLog->log("스킬명 : %s(%d)\n", SkillAttribute[it->first].Name, it->second);
	}

	g_ConsoleDebugLog->log("1초당 들어간 평균 데미지 : %d\n", m_SkillLog.m_iAverageDamage);

	g_ConsoleDebugLog->log("**************** 분석 결과 끝 ******************\n");

#endif // CONSOLE_DEBUG

	m_SkillLog.Reset();
}

void CHackTest::AddAttackCount(int iSkill)
{
	m_SkillLog.m_iSendClientAttackPacket++;

	HackLog::SkillAttackMap::iterator it = m_SkillLog.m_SkillAttackCountList.find(iSkill);
	if(it == m_SkillLog.m_SkillAttackCountList.end())
	{
		m_SkillLog.m_SkillAttackCountList.insert(HackLog::SkillAttackMap::value_type(iSkill, 1));
	}
	else
	{
		it->second++;
	}
}

void CHackTest::ReceiveDamage(CHARACTER* pCha, OBJECT* pObj, int iSuccess, WORD wDamage, int iDamageType, bool bDoubleEnable, bool bComboEnable, WORD wShieldDamage, int iKey)
{
	if(m_SkillLog.m_bLogStart == false)
		return;

	if(wDamage == 0)
	{
		m_SkillLog.m_iMissCount++;
	}
	else
	{
		if(bComboEnable)
		{
			m_SkillLog.m_iTotalDamage += (wDamage * 3); 	
			m_SkillLog.m_iComboDamageCount++;
		}
		else if(bDoubleEnable)
		{
			m_SkillLog.m_iTotalDamage += (wDamage * 2); 
			m_SkillLog.m_iDoubleDamageCount++;
		}
		else
		{
			m_SkillLog.m_iTotalDamage += (wDamage * 1); 	
		}
		
		m_SkillLog.m_iTotalShieldDamage += wShieldDamage;
		
		m_SkillLog.m_iDamageCount++;
	}
}

void CHackTest::ReceiveDie(CHARACTER* pCha, OBJECT* pObj, WORD wSkillType, int iKey)
{
	if(m_SkillLog.m_bLogStart == false)
		return;

	if(pObj->Kind == KIND_MONSTER)
	{
		m_SkillLog.m_iDieMonsterCount++;
#ifdef CONSOLE_DEBUG
		g_ConsoleDebugLog->log("몬스터가 죽음 : 이름(%s)\n", pCha->ID);
#endif // CONSOLE_DEBUG
	}
}

void HackLog::Reset()
{
	m_bLogStart = false;
	m_iTotalDamage = 0;
	m_iTotalShieldDamage = 0;
	m_iDamageCount = 0;
	m_iAverageDamage = 0;
	m_iComboDamageCount = 0;
	m_iDoubleDamageCount = 0;
	m_dwStartTime = 0;
	m_dwEndTime = 0;
	m_dwTotalTime = 0;
	m_iDieMonsterCount = 0;
	m_iMissCount = 0;
	m_iSendClientAttackPacket = 0;
	m_wCurrentSkill = 0;
	m_dwTime = 0;
	m_iKey = -1;
	m_pMonster = NULL;
	m_iSelectedMonster = -1;
}

#endif // CSK_HACK_TEST