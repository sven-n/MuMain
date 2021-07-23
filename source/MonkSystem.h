// MonkSystem.h: interface for the CMonkSystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MONKSYSTEM_H__BB2D80C0_92D7_4E8D_BF3E_32C643EDD764__INCLUDED_)
#define AFX_MONKSYSTEM_H__BB2D80C0_92D7_4E8D_BF3E_32C643EDD764__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#ifdef PBG_ADD_NEWCHAR_MONK
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"

class CItemEqualType
{
private:
	int m_nModelType;
	int m_nSubLeftType;
	int m_nSubRightType;

public:
	CItemEqualType();
	~CItemEqualType();

	void SetModelType(int _ModelIndex, int _Left, int _Right);
	const int& GetModelType() const { return m_nModelType; }
	const int& GetSubLeftType() const { return m_nSubLeftType; }
	const int& GetSubRightType() const { return m_nSubRightType; }
};

class CDummyUnit
{
private:
	enum {
		MAX_DUMMYDISTANCE = 500,
	};
	vec3_t m_vPosition;
	vec3_t m_vStartPosition;
	vec3_t m_vTargetPosition;
	vec3_t m_vDirection;
	float m_fDisFrame;
	float m_fAniFrame;
	float m_fAlpha;
#ifdef PBG_MOD_RAGEFIGHTERSOUND
	float m_fAniFrameSpeed;
#endif //PBG_MOD_RAGEFIGHTERSOUND
public:
	CDummyUnit();
	~CDummyUnit();

	void Init(vec3_t Pos= NULL, vec3_t Target= NULL);
	void Destroy();
	bool IsDistance();
	vec_t* GetPosition();
	vec_t* GetStartPosition();
	float GetAniFrame();
	float GetAlpha();
#ifdef PBG_MOD_RAGEFIGHTERSOUND
	void CalDummyPosition(vec3_t vOutPos, float& fAni, bool bChange=false);
#else //PBG_MOD_RAGEFIGHTERSOUND
	void CalDummyPosition(vec3_t vOutPos, float& fAni);
#endif //PBG_MOD_RAGEFIGHTERSOUND
};
typedef std::map<int, CDummyUnit>	tm_DummyUnit;

typedef std::map<int, CItemEqualType>	tm_ItemEqualType;
typedef std::list<int>	list_ItemType;
typedef std::vector<WORD>	vec_DarkIndex;
typedef std::vector<int>	vec_DarkTargetIndex;

class CMonkSystem  
{
private:
	enum {
		DS_TARGET_NONE = NUMOFMON,
		MAX_REPEATEDLY = 4,
	};
	struct DamageInfo{
		int m_Damage;
		int m_DamageType;
		bool m_Double;
		//int m_ShieldDamage;
	};

	DamageInfo m_arrRepeatedly[MAX_REPEATEDLY];	// 연타공격데미지
	int m_nRepeatedlyCnt;

	int m_nTotalCnt;
	tm_ItemEqualType m_mapItemEqualType;
	CItemEqualType m_cItemEqualType;
	list_ItemType m_listGloveformSword;

	int m_nDarksideCnt;
	vec_DarkIndex m_DarksideIndex;
	vec_DarkTargetIndex m_DarkTargetIndex;

	enum ATTACK_FRAME
	{
		NONE_ATT = 0,
		FRAME_FIRSTATT = 1,
		FRAME_SECONDATT = 2,
	};

	float m_fFirstFrame;
	float m_fSecondFrame;
	BYTE m_btAttState;

	float m_fDistanceFrame;
	//타겟몬스터토탈갯수
	int m_nDarksideEffectTotal;
	int m_nDarksideEffectAttCnt;
	// 분신 캐릭의 애니 프레임(max=2frame)
	float m_fOtherAniFrame;
	float m_fDistanceNextFrame;
	bool m_bDarkSideEffOnce;
	bool m_bDarkSideEffOnce2;
	// 더미 캐릭랜더
	CDummyUnit m_CDummyUnit;
	tm_DummyUnit m_tmDummyUnit;
	float m_fDummyAniFrame;
	// 더미 총갯수 
	int m_nDummyIndex;

	bool m_bUseEffectOnce;
	int m_nLowerEffCnt;
	
public:
	CMonkSystem();
	virtual ~CMonkSystem();

	//글러브형 아이템초기화
	void Init();
	void Destroy();
	//글러브형 아이템 등록
	void RegistItem();
	//아이템의 착용아이템 인덱스
	int GetSubItemType(int _Type, int _Left = 0);
	//착용아이템 인덱스로 모델인덱스 구함
	int GetModelItemType(int _Type);
	//공통된 방어구의 원형의 타입값계산
	int OrginalTypeCommonItemMonk(int _ModifyType);
	//공통된 방어구의 변형된 타입값계산
	int ModifyTypeCommonItemMonk(int _OrginalType);
	//레이지파이터의 공통된 무기류인가
	bool IsRagefighterCommonWeapon(BYTE _Class, int _Type);
	//레이지파이터의글로브형무기템(modeltype)
	bool IsSwordformGloves(int _Type);
	//레이지파이터의장갑류무기의 랜더를함
	void RenderSwordformGloves(CHARACTER *_pCha, int _ModelType, int _Hand, float _Alpha, bool _Translate=false, int _Select=0);
	//기본 인벤의 아이템이고 착용시엔 피직을 설정한 아이템으로 변경한다
	int ModifyTypeSwordformGloves(int _ModelType, int _LeftHand);
	//모델의 형태만 다른 아이템 같은타입으로 간주
	int EqualItemModelType(int _Type);
	//글러브형 무기아이템 로딩
	void LoadModelItem();
	//글러브형 무기아이템 텍스쳐로딩
	void LoadModelItemTexture();
	//장갑형무기의검기
	void MoveBlurEffect(CHARACTER* pCha, OBJECT* pObj, BMD* pModel);
	const int GetItemCnt() const { return m_nTotalCnt; }
	//글러브형무기의itemtype인덱스
	void SetSwordformGlovesItemType();
	//글러브형 무기타입인가(itemtype)
	bool IsSwordformGlovesItemType(int _Type);
	//레이지파이터무기착용조건
	bool RageEquipmentWeapon(int _Index, short _ItemType);
	//연타공격데미지메소드
	bool SetRepeatedly(int _Damage, int _DamageType, bool _Double, bool _bEndRepeatedly);
	int GetRepeatedlyDamage(int _index);
	int GetRepeatedlyDamageType(int _index);
	bool GetRepeatedlyDouble(int _index);
	int GetRepeatedlyCnt();
	//레이지 애니메이션
	bool SetRageSkillAni(int _nSkill, OBJECT* _pObj);
	// 애니메이션 도중에 공격패킷 사용하는 스킬
	bool IsRageHalfwaySkillAni(int _nSkill);
	// 공격 패킷사용
	bool SendAttackPacket(CHARACTER* _pCha, int _nMoveTarget, int _nSkill);
	// 이펙트
	bool RageFighterEffect(OBJECT* _pObj, int _Type);
	// 다크사이드 공격 관련
	void SetDarksideTargetIndex(WORD* _pTargetIndex);
	WORD GetDarksideTargetIndex(int _nIndex);
	int GetDarksideCnt();
	void SetDarksideCnt();
	// 다크사이드공격패킷
	bool SendDarksideAtt(OBJECT* _pObj);
	void InitDarksideTarget();
	// 다크사이드이펙트랜더
	void DarksideRendering(CHARACTER* pCha, PART_t *pPart, bool Translate=false, int Select=0);
	// 타겟의 몬스터 방향으로 벡터 계산
	bool CalculateDarksideTrans(OBJECT* _pObj, vec3_t _vPos, float _fAni, float _fNextAni=0);
	int GetDarksideEffectTotal();
	void InitDummyCal();
	void DestroyDummy();
	void SetDummy(vec3_t pos, vec3_t startpos, int dummyindex);
#ifdef PBG_MOD_RAGEFIGHTERSOUND
	bool IsDummyRender(vec3_t vOutPos, vec3_t Target, float& fAni, int DummyIndex, bool Change=false);
#else //PBG_MOD_RAGEFIGHTERSOUND
	bool IsDummyRender(vec3_t vOutPos, vec3_t Target, float& fAni, int DummyIndex);
#endif //PBG_MOD_RAGEFIGHTERSOUND
	// 연타공격에 따른 메소드
	void InitConsecutiveState(float _fFirstFrame=0, float _fSecondFrame=0, BYTE _btAttState = NONE_ATT);
	bool IsConsecutiveAtt(float _fAttFrame);
	// 데미지포인트
	void RenderRepeatedly(int _Key, OBJECT *pObj);
	// 탈것탔을경우 사용 불가능한 스킬
	bool IsRideNotUseSkill(int _nSkill, short _Type);
	// 글러브형무기경우만 사용 가능한 스킬
	bool IsSwordformGlovesUseSkill(int _nSkill);
	// 비인간형 변신반지 착용시 불가능한 스킬
	bool IsChangeringNotUseSkill(short _LType, short _RType, int _LLevel, int _RLevel);
	// 이펙트 생성
	void InitEffectOnce();
	bool GetSkillUseState();
	bool RageCreateEffect(OBJECT *_pObj, int _nSkill);
	// 로우이펙트 관련
	void InitLower();
	int GetLowerEffCnt();
	// 로우타겟이펙트는 최대 3개
	bool SetLowerEffEct();
};

extern CMonkSystem g_CMonkSystem;
#endif //PBG_ADD_NEWCHAR_MONK
#endif // !defined(AFX_MONKSYSTEM_H__BB2D80C0_92D7_4E8D_BF3E_32C643EDD764__INCLUDED_)