//*****************************************************************************
// file    : GM_PK_Field.h
// producer: BGPARK
//*****************************************************************************

#ifndef _GM_PK_FIELD_H_
#define _GM_PK_FIELD_H_

#ifdef PBG_ADD_PKFIELD

class BMD;

#include "w_BaseMap.h"
#include "./Time/Timer.h"

BoostSmartPointer(CGM_PK_Field);
class CGM_PK_Field: public BaseMap
{
public:
	static CGM_PK_FieldPtr Make();
	virtual ~CGM_PK_Field();

public:	// Object
	// 오브젝트 생성
	virtual bool CreateObject(OBJECT* o);
	// 오브젝트 프로세서
	virtual bool MoveObject(OBJECT* o);
	// 오브젝트 이펙트
	virtual bool RenderObjectVisual(OBJECT* o, BMD* b);
	// 오브젝트 랜더
	virtual bool RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
	// 맵 관련 오브젝트 이펙트
	virtual void RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon = 0);
	
public:
	// 맵에 불씨 날리게 하는 효과
	bool CreateFireSpark(PARTICLE* o);

public:	// Character
	// 몬스터 생성
	virtual CHARACTER* CreateMonster(int iType, int PosX, int PosY, int Key);
	// 몬스터(NPC) 프로세서
	virtual bool MoveMonsterVisual(OBJECT* o, BMD* b);
	// 몬스터 스킬 블러 이펙트
	virtual void MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b);
	// 몬스터 이펙트
	virtual bool RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b);

public: // Sound
	// 몬스터 사운드
	virtual bool PlayMonsterSound(OBJECT* o);
	// 오브젝트 사운드
	virtual void PlayObjectSound(OBJECT* o);
	// 배경음악
	void PlayBGM();
		
public:
	void Init();
	void Destroy();
	
private:
	CGM_PK_Field();
	
	// 몬스터 렌더링
	bool RenderMonster(OBJECT* o, BMD* b, bool ExtraMon = 0);
};

// pk필드 맵인가
extern bool IsPKField();

#endif //PBG_ADD_PKFIELD

#endif //_GM_PK_FIELD_H_