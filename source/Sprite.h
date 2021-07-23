//*****************************************************************************
// File: Sprite.h
//
// Desc: interface for the CSprite class.
//		 스프라이트 클래스.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_SPRITE_H__1696B800_F6E1_4AB2_AA02_F67BBA8EFD2E__INCLUDED_)
#define AFX_SPRITE_H__1696B800_F6E1_4AB2_AA02_F67BBA8EFD2E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ZzzTexture.h"
#include "UIBaseDef.h"

// 크기 조절시 기준.
#define	SPR_SIZING_DATUMS_LT	0x00	// Left, Top	(2진수 : 00)
#define	SPR_SIZING_DATUMS_LB	0x01	// Left, Bottom	(2진수 : 01)
#define	SPR_SIZING_DATUMS_RT	0x02	// Right, Top	(2진수 : 10)
#define	SPR_SIZING_DATUMS_RB	0x03	// Right, Bottom(2진수 : 11)
#define	IS_SIZING_DATUMS_R(v)	(v & 0x02)	// 오른 기준인가?
#define	IS_SIZING_DATUMS_B(v)	(v & 0x01)	// 아래 기준인가?

class CSprite  
{
protected:
	float		m_fScrHeight;		// 스크린 픽셀 높이.
	// OpenGL은 Y축이 아래로 내려갈 수록 감소하므로 필요.

	BITMAP_t*	m_pTexture;
	int			m_nTexID;			// 텍스처 ID.
	float		m_fOrgWidth;		// 본래의 너비.
	float		m_fOrgHeight;		// 본래의 높이.

	SScrCoord	m_aScrCoord[POS_MAX];	// 버택스의 스크린 좌표.
	STexCoord	m_aTexCoord[POS_MAX];	// 버택스의 택스처 좌표.
	float		m_fDatumX;			// X 기준점.(마우스 커서의 Hot spot 개념)
	float		m_fDatumY;			// Y 기준점.

// 스프라이트 색.
	BYTE		m_byAlpha;
	BYTE		m_byRed;
	BYTE		m_byGreen;
	BYTE		m_byBlue;

// Animation 관련.
	int			m_nMaxFrame;		// 총 프레임 개수.
	int			m_nNowFrame;		// 현재 프레임(장면) 번호.
	int			m_nStartFrame;		// 시작 프레임(장면) 번호.
	int			m_nEndFrame;		// 끝 프레임(장면) 번호.
	STexCoord*	m_aFrameTexCoord;	// 각 프레임 텍스처 좌표 배열.
	bool		m_bRepeat;			// Animation 반복 여부.
	double		m_dDelayTime;		// 프레임 간 지연 시간.(밀리초)
	double		m_dDeltaTickSum;	// Update() 호출때 마다 DeltaTick 합계.

	float		m_fScaleX;			// X 확대, 축소 비율.(내부 좌표는 변화 없음)
	float		m_fScaleY;			// Y 확대, 축소 비율.(내부 좌표는 변화 없음)
	bool		m_bTile;			// 타일링 여부.
	int			m_nSizingDatums;	// 크기 조절시 기준.(SPR_SIZING_DATUMS_LT 등)
	bool		m_bShow;			// 보여주는가?

//	char	m_szToolTip[SPR_TT_MAX_LEN + 1];	// 툴팁에 보여줄 내용.

public:
	CSprite();
	virtual ~CSprite();

	void Release();
	void Create(int nOrgWidth, int nOrgHeight, int nTexID = -1,
		int nMaxFrame = 0, SFrameCoord* aFrameCoord = NULL, int nDatumX = 0,
		int nDatumY = 0, bool bTile = false,
		int nSizingDatums = SPR_SIZING_DATUMS_LT, float fScaleX = 1.0f,
		float fScaleY = 1.0f);
	void Create(SImgInfo* pImgInfo, int nDatumX = 0, int nDatumY = 0,
		bool bTile = false, int nSizingDatums = SPR_SIZING_DATUMS_LT,
		float fScaleX = 1.0f, float fScaleY = 1.0f);
	void SetPosition(int nXCoord, int nYCoord, CHANGE_PRAM eChangedPram = XY);
	int GetXPos(){ return (int)m_aScrCoord[LT].fX; }
	int GetYPos(){ return int(m_fScrHeight - m_aScrCoord[LT].fY); }
	void SetSize(int nWidth, int nHeight, CHANGE_PRAM eChangedPram = XY);
	int GetWidth(){ return int(m_aScrCoord[RT].fX - m_aScrCoord[LT].fX); }
	int GetHeight(){ return int(m_aScrCoord[LT].fY - m_aScrCoord[LB].fY); }
	int GetTexID(){ return m_nTexID; };
	int GetTexWidth() 
	{ 
		return -1 < m_nTexID ? (int)m_pTexture->Width : 0; 
	}
	int GetTexHeight() 
	{ 
		return -1 < m_nTexID ? (int)m_pTexture->Height : 0; 
	}
	
	float GetScaleX() { return m_fScaleX; }
	float GetScaleY() { return m_fScaleY; }
	void Show(bool bShow = true) { m_bShow = bShow; }
	bool IsShow() { return m_bShow; }
	int GetSizingDatums() { return m_nSizingDatums; }
	BOOL PtInSprite(long lXPos, long lYPos);
	BOOL CursorInObject();
	void SetAlpha(BYTE byAlpha) { m_byAlpha = byAlpha; }
	BYTE GetAlpha() { return m_byAlpha; }
	void SetColor(BYTE byRed, BYTE byGreen, BYTE byBlue)
	{ m_byRed = byRed;	m_byGreen = byGreen;	m_byBlue = byBlue; }
	void SetAction(int nStartFrame, int nEndFrame, double dDelayTime = 0.0,
		bool bRepeat = true);
	void SetNowFrame(int nFrame);
	void Update(double dDeltaTick = 0.0);
	void Render();
};

#endif // !defined(AFX_SPRITE_H__1696B800_F6E1_4AB2_AA02_F67BBA8EFD2E__INCLUDED_)
