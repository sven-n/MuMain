// Slotlimit.h: interface for the CSlotlimit class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SLOTLIMIT_H__72458006_723F_4246_AA14_5454298A5A5A__INCLUDED_)
#define AFX_SLOTLIMIT_H__72458006_723F_4246_AA14_5454298A5A5A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#ifdef PBG_ADD_CHARACTERSLOT
class CSlotlimit
{
	enum CHAR_ATTRIBUTE
	{
		// 신캐릭터 맵의 맞는 위치값
 		POS_FIRST_X = 8282,
 		POS_FIRST_Y = 19845,
 		POS_FIRST_ANGLE = 35,

		POS_SECOND_X = 8133,
		POS_SECOND_Y = 19645,
		POS_SECOND_ANGLE = 60,

		POS_THIRD_X = 8046,
		POS_THIRD_Y = 19400,
		POS_THIRD_ANGLE = 75,

		POS_FOURTH_X = 7986,
		POS_FOURTH_Y = 19145,
		POS_FOURTH_ANGLE = 90,		

		POS_FIFTH_X = 8008,
		POS_FIFTH_Y = 18885,
		POS_FIFTH_ANGLE = 115,

		SLOTCNT_MAX = 5, // 4,
		CHARCNT_MAX = 5,
	};

	bool	m_bSlotEnable[SLOTCNT_MAX];
	vec3_t	m_vPos[SLOTCNT_MAX];		// [0],[1]번째는 좌표로 사용하고 [2]번째는 rotation사용한다
	int		m_nCharacterCnt;			// 캐릭터를 생성할수 있는갯수====기본 카운트 3
public:
	CSlotlimit();
	~CSlotlimit();

	static CSlotlimit* GetInstance();

	void Init();
	// 생성될수 있는 캐릭수
	void SetCharacterCnt(int _Value);
	int GetCharacterCnt();
	// 자물쇠를 채워야할 갯수
	int GetDisableSlot();
	// disable될 곳 지정
	void SetSlotEnable(int _Value);
	// 인덱스 번째의 활성화 여부
	BOOL GetIsSlotEnable(int _Index);
	// 자물쇠가 채워져할 곳의 정보를 입력한다
	void SetSlotPosition();
	// 자물쇠랜더
	void CreateSlotLock();
};

#define g_SlotLimit	CSlotlimit::GetInstance()

#endif //PBG_ADD_CHARACTERSLOT
#endif // !defined(AFX_SLOTLIMIT_H__72458006_723F_4246_AA14_5454298A5A5A__INCLUDED_)
