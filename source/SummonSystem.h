// SummonSystem.h: interface for the CSummonSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SUMMONSYSTEM_H__B6F7054C_8D6B_4E3C_94E1_C9A5B1207076__INCLUDED_)
#define AFX_SUMMONSYSTEM_H__B6F7054C_8D6B_4E3C_94E1_C9A5B1207076__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "zzzinfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include <map>

class CSummonSystem  
{
public:
	CSummonSystem();
	virtual ~CSummonSystem();

	void MoveEquipEffect(CHARACTER * pCharacter, int iItemType, int iItemLevel,int iItemOption1);		// 소환수 아이템 장비 이펙트 처리
	void RemoveEquipEffects(CHARACTER * pCharacter);	// 해당 캐릭터의 소환수 아이템 장비 이펙트를 모두 제거한다
	void RemoveEquipEffect_Summon(CHARACTER * pCharacter);	// 소환수 아이템 장비시 소환 유령 회전 이펙트 제거
	
	BOOL SendRequestSummonSkill(int iSkill, CHARACTER * pCharacter, OBJECT * pObject);	// 소환수 스킬 사용 서버 전송
	void CastSummonSkill(int iSkill, CHARACTER * pCharacter, OBJECT * pObject, int iTargetPos_X, int iTargetPos_Y);	// 소환수 스킬 이펙트 생성

	void CreateDamageOfTimeEffect(int iSkill, OBJECT * pObject);	// DOT 이펙트 생성
	void RemoveDamageOfTimeEffect(int iSkill, OBJECT * pObject);	// DOT 이펙트 제거
	void RemoveAllDamageOfTimeEffect(OBJECT * pObject);	// DOT 이펙트 모두 제거
	
protected:
	void CreateEquipEffect_WristRing(CHARACTER * pCharacter, int iItemType, int iItemLevel,int iItemOption1);	// 소환수 아이템 장비시 팔목 이펙트 생성
	void RemoveEquipEffect_WristRing(CHARACTER * pCharacter);	// 소환수 아이템 해제시 팔목 이펙트 제거
	void CreateEquipEffect_Summon(CHARACTER * pCharacter, int iItemType, int iItemLevel,int iItemOption1);	// 소환수 아이템 장비시 소환 유령 회전 이펙트 생성

	void CreateCastingEffect(vec3_t vPosition, vec3_t vAngle, int iSubType);	// 소환수 스킬 시전시 공통 이펙트 생성

	void CreateSummonObject(int iSkill, CHARACTER * pCharacter, OBJECT * pObject, float fTargetPos_X, float fTargetPos_Y);	// 소환수 생성
	void SetPlayerSummon(CHARACTER * pCharacter, OBJECT * pObject);
	
protected:
	std::map<SHORT, BYTE> m_EquipEffectRandom;
};

extern CSummonSystem g_SummonSystem;

#endif // !defined(AFX_SUMMONSYSTEM_H__B6F7054C_8D6B_4E3C_94E1_C9A5B1207076__INCLUDED_)
