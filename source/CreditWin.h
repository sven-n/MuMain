//*****************************************************************************
// File: CreditWin.h
//
// Desc: interface for the CCreditWin class.
//		 크레딧 창 클래스.(화면 전체 크기임.)
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_CREDITWIN_H__9D392798_811A_46FE_918B_7753E6BA35D0__INCLUDED_)
#define AFX_CREDITWIN_H__9D392798_811A_46FE_918B_7753E6BA35D0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Win.h"
#include "Button.h"

#define	CRW_SPR_PIC_L			0	// 일러스트 좌.
#define	CRW_SPR_PIC_R			1	// 일러스트 우.
									//(하나의 일러스트는 좌우로 나뉘어 있음.)
#define	CRW_SPR_DECO			2	// 우측 하단 장식.
#define	CRW_SPR_LOGO			3	// 중앙 하단 뮤 로고.
#define CRW_SPR_TXT_HIDE0		4	// 1째줄 글자 가리개.
#define CRW_SPR_TXT_HIDE1		5	// 2째줄 글자 가리개.
#define CRW_SPR_TXT_HIDE2		6	// 3째줄 글자 가리개.
#define	CRW_SPR_MAX				7

#define CRW_ILLUST_MAX			8	// 일러스트 최대 개수.

#define	CRW_NAME_MAX			32	// 이름 또는 부서 문자열 메모리 공간.
#define	CRW_ITEM_MAX			400	// 이름 또는 부서 항목 최대 개수.

#define	CRW_INDEX_DEPARTMENT	0	// 부서.
#define	CRW_INDEX_TEAM			1	// 팀.
#define	CRW_INDEX_NAME			2	// 이름.
#define	CRW_INDEX_NAME0			2	// 이름0.
#define	CRW_INDEX_NAME1			3	// 이름1.
#define	CRW_INDEX_NAME2			4	// 이름2.
#define	CRW_INDEX_NAME3			5	// 이름3.
#define	CRW_INDEX_MAX			6

class CCreditWin : public CWin  
{
	// 안보임, 나타남, 보임, 사라짐.
	enum SHOW_STATE { HIDE, FADEIN, SHOW, FADEOUT };

	struct SCreditItem					// 크레딧 항목.
	{
		BYTE	byClass;				// 줄 위치.(1~3째줄, 0이면 크레딧 끝임.)
		char	szName[CRW_NAME_MAX];	// 항목(이름, 부서 등) 문자열.
	};

protected:
	CSprite		m_aSpr[CRW_SPR_MAX];	// 스프라이트.
	CButton		m_btnClose;				// 닫기 버튼.

// 일러스트 관련.
	SHOW_STATE	m_eIllustState;			// 현재 일러스트 상태.
	double		m_dIllustDeltaTickSum;	// 일러스트 SHOW상태일 때 흐른 시간.
	BYTE		m_byIllust;				// 현재 일러스트 번호.
	char*		m_apszIllustPath[CRW_ILLUST_MAX][2];// 일러스트 경로.

// 텍스트 관련.
	HFONT		m_hFont;				// 폰트핸들.
	SCreditItem	m_aCredit[CRW_ITEM_MAX];// 파일로부터 읽어들인 크레딧 정보.
	int			m_nNowIndex;			// m_aCredit[]의 인덱스.
	int			m_nNameCount;			// 현재 3번째줄에 보여주고 있는 항목 개수.
	int			m_anTextIndex[CRW_INDEX_MAX];// 현재 각 위치에서 보여주고 있는 항목 인덱스.
	SHOW_STATE	m_aeTextState[CRW_INDEX_NAME + 1];//각 줄의 상태.
	double		m_dTextDeltaTickSum;	// 3번째줄 SHOW상태일 때 흐른 시간.

public:
	CCreditWin();
	virtual ~CCreditWin();

	void Create();
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	void SetPosition();
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	void SetPosition(int nXCoord, int nYCoord);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	void Show(bool bShow);
	bool CursorInWin(int nArea);

protected:
	void PreRelease();
	void UpdateWhileActive(double dDeltaTick);
	void RenderControls();

	void CloseWin();
	void Init();
	void LoadIllust();
	void AnimationIllust(double dDeltaTick);
	void LoadText();
	void SetTextIndex();
	void AnimationText(int nClass, double dDeltaTick);
};

#endif // !defined(AFX_CREDITWIN_H__9D392798_811A_46FE_918B_7753E6BA35D0__INCLUDED_)
